#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "application.hpp"
#include "settings.hpp"
#include "web/server.hpp"
#include "log/log_instance.hpp"
#include "log/log.hpp"
#include "thread_name.hpp"
#include "web/fabrique.hpp"
#include "data/Repository.hpp"
#include "data/Loader.hpp"
#include "service/MergeService.h"

std::shared_ptr<application> application::instance_;

// Флаг должен быть создан до приложения
bool daemon_running = true;

application::application()
        : io_context_(get_settings().threads_count()),
          signal_set_(io_context_),
          source_db_pool_(get_settings().source_connection_string()),
          output_db_pool_(get_settings().output_connection_string()) {

    for (auto& connstr : get_settings().online_connection_string()) {
        online_db_pool_list_.insert(make_pair(online_db_pool_list_.size(), connstr));
    }
}

application::~application() {}

std::shared_ptr<listener> create_server(std::shared_ptr<view> view, boost::asio::io_context& ctx, unsigned short port) {
    tcp::endpoint ep(boost::asio::ip::address::from_string("0.0.0.0"), port);
    return std::make_shared<listener>(ctx, ep, view);
}

void application::init() {

    signal_set_.add(SIGTERM);
    signal_set_.add(SIGINT);
    signal_set_.async_wait([](boost::system::error_code const& ec, int signal) {
        std::cerr << "Caught signal " << signal <<"\n";
        application::stop();
    });

    auto settings = get_settings();

    servers_ =
            {
                    create_server(create_default_handler(), io_context_, settings.web_port())
            };

    for (auto& srv : servers_) {
        srv->run();
    }

    for (unsigned idx = 0; idx < settings.threads_count(); ++idx) {
	    io_threads_.emplace_back(
    	    [this, idx]() { 
		        io_context_.run();
	        }
	    );
    }

    start_data_loader();

    register_cyclic_task([this]() {
        source_db_pool_.delete_older_then(20);
    }, "source_db_pool_cleaner", 10000);

    register_cyclic_task([this]() {
        for(auto& [key, value] : online_db_pool_list_) {
            value.delete_older_then(20);
        }
    }, "online_db_pool_cleaner", 10000);

    register_cyclic_task([this]() {
        output_db_pool_.delete_older_then(20);
    }, "output_db_pool_cleaner", 10000);

    merge_service_instance_ = std::make_shared<MergeService>();

    merge_service_thread_ = std::thread([this]() {
        merge_service_instance_->run();
    });
}

void application::stop_asio() {

    for (auto& listnr : servers_) {
        listnr->stop();
    }
    io_context_.stop();
}

void application::wait_asio() {
    io_context_.run();
    std::for_each(io_threads_.begin(), io_threads_.end(),
                  std::mem_fn(&std::thread::join));
}

void application::start() {
    if (application::instance_) {
        return;
    }

    // Установка имени потока
    set_current_thread_name("main");

    // Флаг работы всего приложения
    daemon_running = true;

    // Запуск логгирования до запуска приложения
    start_log();

    Log::notice("start");

    // Запуск приложения
    application::instance_ = std::make_shared<application>();
    application::instance_->init();

    application::instance_->register_cyclic_task([](){ flush_logs();},"logs_flusher", 500);
    application::instance_->register_cyclic_task([](){ clear_logs();}, "logs_cleaner", 3600000);
}

void application::stop() {
    if (!application::instance_) {
        return;
    }

    daemon_running = false;

    Log::notice("stopping");

    // В задачах, выполняемых по таймеру могут оставаться задачи, которые зависят от
    // boost::asio. Возможно здесь нужно добавить таймаут для них
    application::instance_->stop_all_cyclic_tasks();
    application::instance_->stop_asio();

    // Остановка логгирования
    stop_log();
}

MergeService * application::getMergeService() {

    return merge_service_instance_.get();
}

void application::wait() {
    if (!application::instance_) {
        return;
    }
    application::instance_->wait_asio();
    application::instance_.reset();
}

application &application::instance() {
    assert(instance_);
    return *instance_;
}

void application::register_cyclic_task(std::function<void()> routine, char const *name, time_t timeout_in_ms, std::function<void(std::ostream&)> dump) {
    std::lock_guard<std::mutex> lock(cyclic_task_mutex_);
    auto task = std::make_shared<cyclic_task>(io_context_, std::move(routine), name, timeout_in_ms);
    task->html_dump_ = dump;
    cyclic_tasks_[name] = task;
    task->arm();
}

void application::delete_cyclic_task(char const *name) {
    std::lock_guard<std::mutex> lock(cyclic_task_mutex_);
    auto it = cyclic_tasks_.find(name);
    if (it != cyclic_tasks_.end()) {
        it->second->stop();
        cyclic_tasks_.erase(it);
    }
}

void application::stop_all_cyclic_tasks() {
    std::lock_guard<std::mutex> lock(cyclic_task_mutex_);
    for (auto& nt : cyclic_tasks_) { // nt = name/task
        nt.second->stop();
        nt.second->routine_();
    }
    cyclic_tasks_.clear();
}

std::shared_ptr<BDb> application::source_connection() {
    return source_db_pool_.get();
}

void application::back_source_connection(std::shared_ptr<BDb> conn) {
    source_db_pool_.back(conn);
}

int application::online_connection_pool_count() {
    return online_db_pool_list_.size();
}

std::shared_ptr<BDb> application::online_connection(int poolId) {
    auto current = online_db_pool_list_.find(poolId);
    return current->second.get();
}

void application::back_online_connection(int poolId, std::shared_ptr<BDb> conn) {
    auto current = online_db_pool_list_.find(poolId);
    current->second.back(conn);
}

std::shared_ptr<BDb> application::output_connection() {
    return output_db_pool_.get();
}

void application::back_output_connection(std::shared_ptr<BDb> conn) {
    output_db_pool_.back(conn);
}

bool application::running() {
    return daemon_running;
}

boost::asio::io_context &application::io_context() {
    return io_context_;
}

std::vector<cyclic_task_info> application::cyclic_tasks() {
    std::lock_guard<std::mutex>   lock(cyclic_task_mutex_);
    std::vector<cyclic_task_info> result;
    std::transform(cyclic_tasks_.begin(), cyclic_tasks_.end(),
                   std::back_inserter(result),
                   [](auto &p) { return p.second->info(); });
    return result;
}

void application::init_data(time_t timeout) {
    auto task = []() {
        try {
            repo::init_data();
        } catch (std::exception &ex) {
            Log::error("Unable to init data. Exception: ", ex.what());
            application::init_data(10);
        } catch (...) {
            Log::error("Unknown exception during init_data");
            application::init_data(10);
        }
    };
    if (timeout == 0) {
        application::instance().io_context().post(task);
    } else {
        auto timer = std::make_shared<boost::asio::deadline_timer>(application::instance().io_context());
        timer->expires_from_now(boost::posix_time::milliseconds(timeout));
        timer->async_wait([timer, task](boost::system::error_code const& ec) { task(); });
    }
}

std::shared_ptr<cyclic_task> application::get_cyclic_task(std::string const &name) {

    std::lock_guard<std::mutex> lock(cyclic_task_mutex_);
    if (cyclic_tasks_.find(name) != cyclic_tasks_.end()) {
        return cyclic_tasks_.at(name);
    }

    return shared_ptr<cyclic_task>();
}
