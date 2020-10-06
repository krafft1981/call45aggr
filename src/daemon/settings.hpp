#pragma once

#include <memory>
#include <vector>
#include <string>

struct settings;

// \return current program settings wrapped by implicit shared object
settings get_settings();

void read_settings(std::string const& path_to_file);

namespace Json{
    class Value;
}

// Структура, которая помогает спрятать некоторые символы, которые
// не предназначены для публичного показа
struct setting_mask {
    std::string path;    // Путь в дереве настроек
    std::string regexp;  // Регулярное выражение для замены
    std::string replace; // Выражение для замены
};

// implicit shared объект настроек для случая, если понадобится перечитать настройки
struct settings_impl;
struct settings {

    /*
     * Creation method
     * \param path_to_file - path to config file. If empty reread config
     * \return old global settings if it exists and error take place
     *         default settings if empty path provided or error take
     *                          place and global settings not setted up
     */
    static settings parse(std::string const& path_to_file);

    /*
     *
     * settings functions
     *
     */
    unsigned threads_count() const;
    unsigned short web_port() const;
    unsigned max_call_time() const;
    unsigned table_reload_time() const;
    unsigned sql_insert_buffer_size() const;
    unsigned max_queue_size() const;
    unsigned read_threshold_second() const;
    unsigned read_waiting_second() const;

    std::string const& source_connection_string() const;
    std::string const& output_connection_string() const;
    std::vector<std::string> const& online_connection_string() const;

    std::string const& path_to_logs() const;

    unsigned short graylog_port() const;
    std::string const& graylog_host() const;
    std::string const& graylog_source() const;

    unsigned instance_id() const;
    std::string const& instance_name() const;

    size_t db_bandwidth() const;

    size_t sync_thread_period() const;

    // Количество параллельно обновляемых баз данных
    unsigned loader_thread_count();

    std::string const& log_level() const;

    bool is_slave() const;

    Json::Value dump();

    /*
     * \return path to current settings file
     */
    std::string settings_path() const;

    /*
     * \return masking
     */
    std::vector<setting_mask> settings_masks() const;

    bool use_core_dump() const;

    /*
     * sugar for logic operations
     */
    explicit operator bool() const {
        return static_cast<bool>(impl_);
    }

private:
    std::shared_ptr<settings_impl> impl_;
    static settings instance_;

    friend settings get_settings();
    friend void read_settings(std::string const&);
};
