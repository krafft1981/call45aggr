#include "PullManager.h"

#include <type_traits>

#include <common/concat.hpp>
#include <common/join_through.hpp>
#include <systems/ExceptionHolder.h>
#include <daemon/daemon.hpp>
#include <daemon/settings.hpp>
#include <daemon/cyclic_task.hpp>
#include <daemon/life_span.hpp>

PullManager::PullManager():
    bandwidth_limit_mbits{ static_cast<double>(get_settings().db_bandwidth()) },
    speed_limiter(bandwidth_limit_mbits),
    errors_count(0)
{
    init();

    for (auto p : {PullerPriority ::Low, PullerPriority ::Critical, PullerPriority ::High}) {
        puller_workers[p].start(p);
    }

    for (auto const& puller : pullers){
        puller_workers.at(puller.second->priority()).add(puller.first, puller.second.get());
    }

}

int PullManager::get_errors_count() const {
    return errors_count;
}

int PullManager::get_event_count() const {
    int result = 0;
    for (auto const& p : puller_workers){
        result += p.second.events_count();
    }
    return result;
}

std::vector<const Puller*> PullManager::get_pulls() const {
    std::vector<const Puller*> result;
    result.reserve(pullers.size());
    for (const auto& it: pullers) {
        result.push_back(it.second.get());
    }
    return result;
}

std::string PullManager::excludeEventsSql() {

    vector<string> skipped_events;
    for (auto const& task : puller_workers){
        if (task.second.working()) {
            task.second.copy_names_to(skipped_events);
        }
    }

    // Если инвертировать условие только для отбираемых в текущей итерации событий,
    // то будут пропущены события, которые система не поддерживает.
    return concat(" and event not in ('", join_through(skipped_events,"','"), "')");
}

void PullManager::pull() {

/*
    errors_count += tl_exception_holder().have_exception();

    // Проверка, есть ли для этого потока исключения
    // Устанавливаются в задаче, которая назначается в PullWorker::enqueue
    tl_exception_holder().try_rethrow();

    // Поиск событий на удаление
    clear_event_table();

    string select_events_query = concat(
            "select event, param, version from event.queue where server_id in (", get_settings().instance_id(),")",
            excludeEventsSql(), " order by server_id, version limit ", part_size);

    std::unordered_map<std::string, std::vector<int64_t>> ids, versions;

    auto db_main = app::asterisk_source_conn();

    BDbResult res = db_main->query(select_events_query);

    int nResults = static_cast<int>(res.size());
    if (nResults == 0) {
        errors_count = 0;
        return;
    }

    // Содержит все обрабатываемые события. Сохраняет порядок из таблицы базы данных
    std::vector<std::string> events;
    while (res.next()) {
        std::string event = res.get_s(0);
        if (ids.find(event) == ids.end()){
            events.push_back(event);
        }

        ids[event].push_back(res.get_ll(1));
        versions[event].push_back(res.get_ll(2));
    }

    // Поиск неподдерживаемых событий
    for (auto it = events.begin(); it != events.end();) {
        if (pullers.find(*it) == pullers.end()) {
            std::string event = *it;

            ids.erase(event);
            versions.erase(event);

            db_main->exec2("delete from event.queue where server_id in ",
                         "(", get_settings().instance_id() , ")"
                         " and event = '", event, "'");

            it = events.erase(it);
        } else {
            ++it;
        }
    }

    auto delayer = [this](size_t bytes_count) mutable {
        speed_limiter.on_progress_updated(bytes_count);
    };

    for (std::string const &event : events) {
        auto &worker = puller_workers.at(pullers.at(event)->priority());
        worker.enqueue(event, ids.at(event), versions.at(event), delayer);
    }
*/
}

void PullManager::clear_event_table() {
    for (auto& [name, puller] : pullers) {
        puller->clear();
    }
}

static void start_puller_task() {
    auto setts = get_settings();
    if (setts.is_slave()) {
        return;
    }

    app::register_cyclic_task([] () {
        PullManager::instance()->pull();
        }, "sync", 10000);
}

ON_INIT_DB("start sync", &start_puller_task);
