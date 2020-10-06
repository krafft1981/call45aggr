#include <iostream>
#include <boost/asio.hpp>
#include "daemon.hpp"
#include "application.hpp"
#include "data/Repository.hpp"
#include "log/log.hpp"
#include "life_span.hpp"

static time_t start_time = time(nullptr);

namespace app {

void start_init_data(time_t timeout = 0) {
    auto task = []() {
        try {
            repo::init_data();
            apply_life_span_events(life_span_event::INIT_DB);
        } catch (std::exception &ex) {
            Log::error("Unable to init data. Exception: ", ex.what());
            start_init_data(250);
        } catch (...) {
            Log::error("Unknown exception during init_data");
            start_init_data(250);
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

int run() try {
    apply_life_span_events(life_span_event::START);
    application::start();
    apply_life_span_events(life_span_event::INIT_APP);
    start_init_data();
    application::wait();
    return 0;
}
catch(std::exception &ex ){
    std::string fatal_error = concat("Fatal Error: ", ex.what());
    std::cerr << fatal_error << "\n";
    Log::critical(fatal_error);
    return -1;
}
catch (...) {
    std::cerr << "Unknown error\n";
    Log::critical("Unknown error");
    return -1;
}

void stop() {
    apply_life_span_events(life_span_event::STOP);
    application::stop();
}

void register_cyclic_task(std::function<void()> routine, char const *name,
                          time_t timeout_in_ms, std::function<void(std::ostream&)> dump) {
    application::instance().delete_cyclic_task(name);
    application::instance().register_cyclic_task(std::move(routine), name, timeout_in_ms, dump);
}

void delete_cyclic_task(char const* name) {
    application::instance().delete_cyclic_task(name);
}

MergeService * getMergeService() {
    return application::instance().getMergeService();
}

reusable_object<BDb> source_conn() {
    auto callback = [](int poolId, std::shared_ptr<BDb> val) {
        application::instance().back_source_connection(val);
    };
    auto connection = application::instance().source_connection();
    return reusable_object<BDb>(callback, connection);
}

reusable_object<BDb> online_conn(int poolId) {
    auto callback = [](int poolId, std::shared_ptr<BDb> val) {
        application::instance().back_online_connection(poolId, val);
    };

    auto connection = application::instance().online_connection(poolId);
    return reusable_object<BDb>(callback, poolId, connection);
}

reusable_object<BDb> output_conn() {
    auto callback = [](int poolId, std::shared_ptr<BDb> val) {
        application::instance().back_output_connection(val);
    };
    auto connection = application::instance().output_connection();
    return reusable_object<BDb>(callback, connection);
}

int online_pool_count() {
    return application::instance().online_connection_pool_count();
}

void run_async_task(std::function<void()> async_routine) {
    application::instance().io_context().post(async_routine);
}

time_t uptime() {
    return time(nullptr) - start_time;
}

bool running() {
    return application::running();
}

} // namespace daemon
