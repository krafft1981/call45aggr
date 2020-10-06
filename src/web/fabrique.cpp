#include "fabrique.hpp"
#include "aggregated.hpp"
#include "page/config.hpp"
#include "page/health.hpp"
#include "page/root.hpp"
#include "page/log.hpp"
#include "page/state.hpp"
#include "page/data.hpp"
#include "page/sync.hpp"
#include "page/online.hpp"
#include "page/time.hpp"
#include "page/merge.hpp"
#include "page/leg.hpp"
#include "page/link.hpp"
#include "page/records.hpp"
#include "page/cyclic_task.hpp"
#include "page/favicon.hpp"

// Создает апи со страницами
std::shared_ptr<view> create_default_handler() {
    std::shared_ptr<aggregated> result = std::make_shared<aggregated>();

    result->add<favicon_view>("/favicon.ico");
    result->add<config_view>("/config");
    result->add<health_view>("/health");
    result->add<root_view>("/");
    result->add<log_view>("/log");
    result->add<sync_view>("/sync");
    result->add<state_view>("/state");
    result->add<data_view>("/data");
    result->add<cyclic_task_view>("/cyclic_task");
    result->add<online_view>("/online");
    result->add<time_view>("/time");
    result->add<merge_view>("/merge");
    result->add<leg_view>("/leg");
    result->add<records_view>("/records");
    result->add<link_view>("/link");

    return result;
}
