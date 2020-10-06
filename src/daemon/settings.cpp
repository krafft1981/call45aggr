#include <json/json.h>
#include <fstream>
#include <assert.h>
#include <type_traits>
#include "settings.hpp"
#include "common.hpp"
#include "Meta.h"
#include "common/split.h"
#include "log/log_message.hpp"

struct settings_impl {

    std::string settings_path_;
    unsigned short web_port_ = 8080;
    unsigned max_call_time_ = 10800;
    unsigned table_reload_time_ = 300;
    unsigned max_queue_size_ = 10000;
    unsigned sql_insert_buffer_size_ = 1024 * 1024 * 10;
    unsigned read_threshold_second_ = 10000;
    unsigned read_waiting_second_ = 900;

    unsigned threads_count_ = 5;
    std::string source_connection_string_;
    std::string output_connection_string_;
    std::vector<std::string> online_connection_string_;
    std::string path_to_log_file_;

    std::string graylog_host_;
    unsigned short graylog_port_ = 12201;
    std::string graylog_source_ = "dev";

    unsigned instance_id_ = 9000;
    std::string instance_name_ = "Unknown";

    size_t sync_thread_period_ = 1000;
    size_t db_bandwidth_ = std::numeric_limits<size_t>::max();

    unsigned loader_thread_count_ = 4;

    std::string type_ = "master";

    std::string log_level_ = "DEBUG";

    bool use_core_dump_ = true;
};

template<typename T>
auto read_json(Json::Value& json){
    // TODO: Workaround some way in static_assert
    assert(false);
}

template<> auto read_json<unsigned short>(Json::Value& json) { return json.asUInt(); }
template<> auto read_json<unsigned>(Json::Value& json) { return json.asUInt(); }
template<> auto read_json<size_t>(Json::Value& json) { return json.asUInt64(); }
template<> auto read_json<std::string>(Json::Value& json) {return json.asString(); }
template<> auto read_json<bool>(Json::Value& json) { return json.asBool(); }
template<> auto read_json<std::vector<std::string>>(Json::Value& json) {
    std::vector<std::string> result;
    if (!json.isArray()) {
        return result;
    }
    
    for(Json::Value::ArrayIndex id = 0; id < json.size(); id ++) {
        if (json[id].isString()) {
            result.push_back(json[id].asString());
        }
    }
    return result;
}

template<> auto read_json<std::vector<int>>(Json::Value& json) {
    std::vector<int> result;
    if (!json.isArray()) {
        return result;
    }
    
    for(Json::Value::ArrayIndex id = 0; id < json.size(); id ++) {
        if (json[id].isInt64()) {
            result.push_back(json[id].asInt64());
        }
    }
    return result;
}

static Json::Value find_json(char const* path, Json::Value const& n) {

    auto subpaths = split_any(path, "/");

    Json::Value current = n;
    for (std::string const& sp : subpaths) {
        current = current[sp];
        if (current.isNull()) {
            return Json::Value::null;
        }
    }

    return current;
}

template <typename ObjectType, typename SequenceType>
static void parse_object(ObjectType &obj, Json::Value &n,
                         SequenceType const &seq) {
    meta::detail::for_tuple(
            [&obj,&n](auto &v) {
                using type = typename std::decay_t<decltype(v)>::member_type;

                char const* path = v.getName();
                auto json = find_json(path,n);
                if (json.isNull()) {
                   return;
                }

                // Чтение значение и каст
                auto tmp = read_json<type>(json);
                if constexpr (std::is_integral_v<type> && !std::is_same_v<bool, type>) {
                    if (json.isString()) {
                        v.getRef(obj) = to<type>(json.asCString());
                    } else {
                        v.getRef(obj) = numeric_cast<type>(tmp);
                    }
                } else {
                    v.getRef(obj) = tmp;
                }
            },
            seq);
}

using meta::members;
using meta::member;

// Все настройки в одном месте
// При замене пути заменить настройку и в функции settings_mask
auto static sequence = members(
        member("threads_count", &settings_impl::threads_count_),
        member("web_port", &settings_impl::web_port_),
        member("max_call_time", &settings_impl::max_call_time_),
        member("table_reload_time", &settings_impl::table_reload_time_),
        member("max_queue_size", &settings_impl::max_queue_size_),
        member("use_core_dump", &settings_impl::use_core_dump_),
        member("db/source_connection_string", &settings_impl::source_connection_string_),
        member("db/output_connection_string", &settings_impl::output_connection_string_),
        member("db/online_connection_string", &settings_impl::online_connection_string_),
        member("db/sql_insert_buffer_size"  , &settings_impl::sql_insert_buffer_size_  ),
        member("db/read_threshold_second"   , &settings_impl::read_threshold_second_   ),
        member("db/read_waiting_second"     , &settings_impl::read_waiting_second_     ),
        member("log/file", &settings_impl::path_to_log_file_),
        member("log/graylog_source", &settings_impl::graylog_source_),
        member("log/graylog_host", &settings_impl::graylog_host_),
        member("log/graylog_port", &settings_impl::graylog_port_),
        member("instanceId", &settings_impl::instance_id_),
        member("instanceName", &settings_impl::instance_name_),
        member("db/bandwidth", &settings_impl::db_bandwidth_),
        member("thread/sync_period", &settings_impl::sync_thread_period_),
        member("thread/max_loader_threads_count", &settings_impl::loader_thread_count_),
        member("log/level", &settings_impl::log_level_),
        member("type", &settings_impl::type_)
);

std::vector<setting_mask> settings::settings_masks() const {
    return {
        {"db/source_connection_string", "password=(.*)|user=(.*)", ""},
        {"db/output_connection_string", "password=(.*)|user=(.*)", ""},
        {"db/online_connection_string", "password=(.*)|user=(.*)", ""}
    };
}

settings settings::instance_;
settings settings::parse(std::string const& path_to_file) {

    settings result;
    result.impl_ = std::make_shared<settings_impl>();
    result.impl_->settings_path_ = path_to_file;

    // Нет пути до настроек - возвращаются значения по умолчанию
    if (path_to_file.empty()) {
        return result;
    }

    auto backup_config = (get_settings() ? get_settings() : result);

    std::ifstream is(path_to_file);
    if (!is.good()){
        return backup_config;
    }

    auto source = std::string(std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>());

    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> newCharReader(builder.newCharReader());

    Json::Value root;
    std::string error;
    if (!newCharReader->parse(&*source.begin(), &*source.end(), &root, &error)) {
        return backup_config;
    }

    auto impl = result.impl_;

    parse_object(*impl, root, sequence);

    return result;
}

settings get_settings() {
    return settings::instance_;
}

std::string settings::settings_path() const {
    return impl_->settings_path_;
}

unsigned short settings::web_port() const {
    return impl_->web_port_;
}

unsigned settings::max_call_time() const {
    return impl_->max_call_time_;
}

unsigned settings::table_reload_time() const {
    return impl_->table_reload_time_;
}

unsigned settings::sql_insert_buffer_size() const {
    return impl_->sql_insert_buffer_size_;
}

unsigned settings::max_queue_size() const {
    return impl_->max_queue_size_;
}

unsigned settings::read_threshold_second() const {
    return impl_->read_threshold_second_;
}

unsigned settings::read_waiting_second() const {
    return impl_->read_waiting_second_;
}

unsigned settings::threads_count() const {
    return impl_->threads_count_;
}

std::string const &settings::source_connection_string() const {
    return impl_->source_connection_string_;
}

std::string const &settings::path_to_logs() const {
    return impl_->path_to_log_file_;
}

unsigned short settings::graylog_port() const {
    return impl_->graylog_port_;
}

std::string const& settings::graylog_host() const {
    return impl_->graylog_host_;
}

std::string const& settings::graylog_source() const {
    return impl_->graylog_source_;
}

template<typename ValueType>
void dump_helper(std::vector<std::string> const& path,
                                            Json::Value& current,
                                            ValueType& v, unsigned idx = 0) {
    if (idx == path.size()) {
        current = v;
        return;
    }
    dump_helper(path, current[path[idx]], v, idx + 1);
}

template<>
void dump_helper<std::vector<std::string>>(std::vector<std::string> const& path,
                                           Json::Value& current,
                                           std::vector<std::string>& v, unsigned idx) {
    if (idx == path.size()) {
        current.resize(v.size());
        for (Json::ArrayIndex idx = 0; idx < v.size(); ++idx) {
            current[idx] = v[idx];
        }
        return;
    }
    dump_helper(path, current[path[idx]], v, idx + 1);
}

template<>
void dump_helper<std::vector<int>>(std::vector<std::string> const& path,
                                           Json::Value& current,
                                           std::vector<int>& v, unsigned idx) {
    if (idx == path.size()) {
        current.resize(v.size());
        for (Json::ArrayIndex idx = 0; idx < v.size(); ++idx) {
            current[idx] = v[idx];
        }
        return;
    }
    dump_helper(path, current[path[idx]], v, idx + 1);
}

template <typename ObjectType, typename SequenceType>
static Json::Value dump_object(ObjectType &obj,
                         SequenceType const &seq) {
    Json::Value result;
    meta::detail::for_tuple(
            [&obj,&result](auto &v) {
                char const* path = v.getName();
                dump_helper(split_any(path,"/"), result, v.getRef(obj));
                },
            seq);
    return result;
}

Json::Value settings::dump() {
    return dump_object(*impl_, sequence);
}

unsigned settings::instance_id() const {
    return impl_->instance_id_;
}

std::string const &settings::instance_name() const {
    return impl_->instance_name_;
}

size_t settings::sync_thread_period() const {
    return impl_->sync_thread_period_;
}

std::vector<std::string> const &settings::online_connection_string() const {
    return impl_->online_connection_string_;
}

std::string const &settings::output_connection_string() const {
    return impl_->output_connection_string_;
}

size_t settings::db_bandwidth() const {
    return impl_->db_bandwidth_;
}

unsigned settings::loader_thread_count() {
    return impl_->loader_thread_count_;
}

std::string const& settings::log_level() const {
    return impl_->log_level_;
}

bool settings::is_slave() const {
    return impl_->type_ == "slave";
}

extern void set_log_level(log_message_level level);
void read_settings(std::string const& path_to_file) {

    // Поиск пути для чтения файла. Если путь не пустой, то читается этот файл
    // Если путь пустой, то перечитывается файл из настроек
    std::string path = path_to_file;
    if (path.empty()) {
        auto old_settings = get_settings();
        if (old_settings) {
            path = old_settings.settings_path();
        }
    }


    settings::instance_ = settings::parse(path);
    set_log_level(parse_log_level(settings::instance_.log_level()));
}

bool settings::use_core_dump() const {
    return impl_->use_core_dump_;
}
