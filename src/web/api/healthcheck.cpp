#include <json/json.h>
#include "healthcheck.hpp"
#include "web/view.hpp"
#include "daemon/health_register.hpp"
#include "daemon/settings.hpp"
#include "daemon/daemon.hpp"
#include "common/concat.hpp"

http_response healthcheck_view::handle(const http_request &request) {

    http_response result;

    auto settings = get_settings();

    Json::Value val;
    val["autoLockFinance"] = true;
    val["currentCalls"] = 1;
    val["instanceId"] = settings.instance_id();
    val["item0"]["itemId"] = "call45aggr";
    val["item0"]["itemVal"] = "No errors";
    val["item0"]["statusId"] = "STATUS_OK";
    val["item0"]["statusMessage"] = "No errors";
    val["regionList"][0]["regionId"] = settings.instance_id();
    val["regionList"][0]["regionName"] = settings.instance_name();
    val["runTime"] = app::uptime();

    int i = 1;
    for (auto& ht : health_register::states()) {
        auto item = concat("item", i++);
        val[item]["itemId"] = ht.item;
        val[item]["itemVal"] = "";
        val[item]["statusId"] = [&ht]() {
           switch(ht.status_type) {
               case health_status::WARNING: return "STATUS_WARNING";
               case health_status::ERROR: return "STATUS_ERROR";
               case health_status::CRITICAL: return "STATUS_CRITICAL";
           }
        }();
        val[item]["statusMessage"] = ht.status_message;
    }

    result.mime_type_ = "application/json";
    result.code_ = 200;
    result.body_ = concat(val);
    return result;
}
