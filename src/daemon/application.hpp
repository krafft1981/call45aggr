#pragma once

#include <thread>
#include <vector>
#include <list>
#include <unordered_map>
#include <boost/asio/io_context.hpp>
#include "cyclic_task.hpp"
#include "db/DbPool.hpp"
#include "db/DbException.h"

class listener;
using listener_ptr_t = std::shared_ptr<listener>;

class MergeService;
using merge_service_ptr_t = std::shared_ptr<MergeService>;

// Скрытый отовсюду инстанс приложения
struct application {

    application();
    ~application();

    static void start();
    static void init_data(time_t timeout = 0);
    static void reread();
    static void stop();
    static void wait();
    static bool running();

    static application& instance() ;

    void init();

    // asio фактически управляет всеми потоками в приложении
    void stop_asio();
    void wait_asio();

    // Интерфейс для управления циклическими заданиями
    void register_cyclic_task(std::function<void()> routine, char const *name, time_t timeout_in_ms, std::function<void(std::ostream&os)> dump  = {}) ;
    void delete_cyclic_task(char const* name);

    // Управление пуллами базы данных
    std::shared_ptr<BDb> source_connection();
    std::shared_ptr<BDb> output_connection();
    std::shared_ptr<BDb> online_connection(int poolId);

    int online_connection_pool_count();

    void back_source_connection(std::shared_ptr<BDb> conn);
    void back_output_connection(std::shared_ptr<BDb> conn);
    void back_online_connection(int poolId, std::shared_ptr<BDb> conn);

    boost::asio::io_context& io_context();

    // list of current tasks
    std::vector<cyclic_task_info> cyclic_tasks();
    std::shared_ptr<cyclic_task> get_cyclic_task(std::string const& name);

    MergeService * getMergeService();

private:
    // Удаляет все циклические задания, и выполняет заложенные в них процедуры
    void stop_all_cyclic_tasks();

    // boost::asio
    boost::asio::io_context io_context_;
    boost::asio::signal_set signal_set_;

    // Пусть сервера работают в едином контексте
    // Возможно в будущем придется распределить потоки для каждого сервера
    // во избежание загрузки каких-то важных частей программы
    std::vector<std::thread>    io_threads_;
    std::vector<listener_ptr_t> servers_;

    // Поток соединения полученных даных
    std::thread merge_service_thread_;
    merge_service_ptr_t merge_service_instance_;

    // Механизм постановки циклических задач
    std::mutex  cyclic_task_mutex_;
    std::unordered_map<std::string, std::shared_ptr<cyclic_task>> cyclic_tasks_;

    // Пуллы для различных баз данных
    DbPool source_db_pool_;
    DbPool output_db_pool_;
    std::map<int, DbPool> online_db_pool_list_;

    // жизнь глобальной переменной начинается всех функций
    // нужно прочитать конфиг к моменту появления объекта приложения
    static std::shared_ptr<application> instance_;
};
