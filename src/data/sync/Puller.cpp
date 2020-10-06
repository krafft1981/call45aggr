#include "Puller.h"
#include "ArgumentsWrapper.h"

#include <mutex>

#include "../common.h"
#include <db/BDb.h>
#include <systems/Exception.h>
#include <log/log.hpp>
#include <common/concat.hpp>
#include <common/join_through.hpp>
#include <systems/Timer.h>
#include <daemon/daemon.hpp>
#include <daemon/settings.hpp>

namespace {

std::string intersect_conditions(const std::string& first_condition, const std::string& second_condition) {
    if (first_condition.empty()) {
        return second_condition;
    }
    if (second_condition.empty()) {
        return first_condition;
    }
    return concat("(", first_condition, ") and (", second_condition, ")");
}

} // namespace

Puller::Puller(
    const Event& event,
    const SrcTable& src_table,
    const DstTable& dst_table,
    const Key& key,
    const SrcSqlWhere& src_sql_where,
    const Fields& fields,
    PullerPriority priority
):
        _event{ event.data() },
        _src_table{ src_table.data() },
        _dst_table{ dst_table.data() },
        _key{ key.data() },
        _src_sql_where{ src_sql_where.data() },
        _fields{ fields.data() },
        _priority{ priority },
        _only_full_pull_is_supported{
                _key.empty()} // it is also supposed that non-empty _id corresponds primary key in DB table
{}

Puller::~Puller()
{}

const std::string& Puller::get_event() const {
    return _event;
}

Puller::Info Puller::get_info() const {
    const std::lock_guard<std::mutex> lockGuard{ _lock };
    return _info;
}

void Puller::pull(std::function<void(size_t)> delayer, std::vector<int64_t> const& ids, std::vector<int64_t> const& versions) const{


    std::string condition, clear_query;
    std::tie(condition,clear_query) = createUpdateQueries(ids);

    auto main_db = app::source_conn();
    auto slave_db = app::output_conn();

    copied_bytes_ = 0;
    std::function<void(size_t)> delayerProxy = [&,this](size_t bytes_read) mutable {
        copied_bytes_ += bytes_read;
        delayer(bytes_read);

        // Остановка копирования
        if (unlikely(!app::running())) {
            throw std::runtime_error("Application stopped");
        }
    };

    std::size_t copied_bytes = 0;

    try {
        const TimerScopeGuarded<std::mutex> timerScope{ _lock, _info.timer };
        BDbTransaction trans(slave_db.get());
        slave_db->exec(clear_query);
        copied_bytes = BDb::copy(_src_table, _dst_table, getQueryFields(), condition, main_db.get(), slave_db.get(), delayerProxy);
        trans.commit();
    } catch (Exception &e) {
        {
            const std::lock_guard<std::mutex> lockGuard{ _lock };
            ++_info.pull_count_errors;
        }
        e.addTrace("Puller::pull: " + _event);
        throw;
    }

    {
        std::lock_guard<std::mutex> lock(clean_mutex_);
        std::copy(versions.begin(), versions.end(), std::back_inserter(versions_to_clean_));
    }

    {
        const std::lock_guard<std::mutex> lockGuard{ _lock };
        if (is_full_pull_required(ids)) {
            _info.pull_count_full++;
        } else {
            _info.pull_count_partial += ids.size();
        }

        _info.last_copied_bytes = copied_bytes;
        _info.total_copied_bytes += copied_bytes;

    }
}

std::string Puller::getQueryFields() const {
    return "\"" + join_through(_fields, "\",\"") + "\"";
}

bool Puller::is_full_pull_required(std::vector<int64_t> const& ids) const {

    if (_only_full_pull_is_supported){
        return true;
    }

    // Если есть хоть одна запись с 0, то происходит полная синхронизация
    for(auto id : ids){
        if (id == 0){
            return true;
        }
    }
    return false;
}

tuple<string, string> Puller::createUpdateQueries(std::vector<int64_t> const &ids) const {
    bool full_pull_required = is_full_pull_required(ids);

    std::string clear_query = concat("delete from ", _dst_table);
    std::string condition = _src_sql_where;

    if (!full_pull_required) {
        const string ids_sql = join_through(ids,",");
        const std::string key_selection = concat("\"", _key, "\" in (", ids_sql ,")");

        clear_query += concat(" where ", key_selection);
        condition = intersect_conditions(condition, key_selection);
    } else {
        clear_query = concat("truncate ", _dst_table);
    }

    return tuple<string,string>(condition, clear_query);
}

PullerPriority Puller::priority() const {
    return _priority;
}

size_t Puller::copied_bytes_in_last_operation() const {
    return copied_bytes_;
}

void Puller::clear() {

    std::lock_guard<std::mutex> lock(clean_mutex_);
    if (versions_to_clean_.empty()) {
        return;
    }

    versions_to_clean_.clear();

}

std::ostream &operator<<(std::ostream &os, PullerPriority priority) {

    switch(priority ) {
        case PullerPriority::Critical: os << "Critical"; break;
        case PullerPriority::High: os << "High"; break;
        case PullerPriority::Low: os << "Low"; break;
    }

    return os;
}
