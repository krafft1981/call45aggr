#include <mutex>
#include <set>
#include "Loader.hpp"
#include "daemon/daemon.hpp"
#include "daemon/settings.hpp"
#include "common/join_through.hpp"
#include "db/BDb.h"
#include "log/log.hpp"
#include "daemon/health_register.hpp"

namespace {
using loading_table_task_ft = void (*)(BDb*);
using loading_counter_task_ft = void(*)(BDb*, std::vector<long long int> const&);

struct loading_info {
    std::string event_name_;
    std::vector<long long int> ids_;
    std::vector<long long int> versions_;

    void shrink() {
        bool shrinkable = (std::find(ids_.begin(), ids_.end(), 0) != ids_.end());
        if (shrinkable) {
            ids_ = {0};
        }
    }
};

struct loader : public std::enable_shared_from_this<loader> {

    void load();

private:

    std::vector<std::string> loading_events();

    void add_load_event(std::string const& name);
    void rem_load_event(std::string const& name);
    bool start_load_event(loading_info&& info);

    loading_table_task_ft loading_task_by_name(std::string const& tab_name);
    loading_counter_task_ft  loading_counter_task_by_name(std::string const& tab_name);

    std::mutex            loading_events_mutex_;
    std::set<std::string> loading_events_;
};

void loader::load() {

    auto settings = get_settings();

    auto cur_loading_tables = loading_events();

    // Таблицы грузятся
    if (settings.loader_thread_count() <= cur_loading_tables.size()) {
        return;
    }

    size_t available_count = settings.loader_thread_count() - cur_loading_tables.size();

//    auto db = app::asterisk_output_conn();
//    auto res = db->query ("");
//    std::unordered_map<std::string, loading_info> info;
//    while (res.next()) {
//        std::string event = res.get(0);
//        long long int version = qres.get_ll(1);
//        long long int row_id = qres.get_ll(2);
//        info[event].versions_.push_back(version);
//        info[event].ids_.push_back(row_id);
//    }

    // Важен порядок загрузки
/*
    std::map<int, loading_info> sorted_info;
    for (auto&p : info) {
        auto version = p.second.versions_.front();
        sorted_info[version] = std::move(p.second);
        sorted_info[version].event_name_ = p.first;
    }

    for (auto& p : sorted_info){
        if (available_count-- == 0) {
            return;
        }

        start_load_event(std::move(p.second));
    }
*/
}

std::vector<std::string> loader::loading_events() {
    std::lock_guard<std::mutex> lock(loading_events_mutex_);
    return std::vector<std::string>(loading_events_.begin(),
                                    loading_events_.end());
}

void loader::add_load_event(std::string const &name) {
    std::lock_guard<std::mutex> lock(loading_events_mutex_);
    loading_events_.insert(name);
}

void loader::rem_load_event(std::string const &name) {
    std::lock_guard<std::mutex> lock(loading_events_mutex_);
    loading_events_.erase(name);
}

void delete_event(BDb* db, std::string const& name, std::vector<long long> const& versions){
//    auto versions_str = concat("('", join_through(versions, "', '"), "')");
//    db->exec2("DELETE FROM event.queue WHERE event='", name, "' and VERSION in ", versions_str, "");
//    Log::verbose("Event \"", name, "\" with version(s) ", versions_str, " has been removed from event.queue");
}

std::string table_loading_event(std::string const& name) {
    return concat("Table Loading Event ", name);
}

std::string counters_loading_event(std::string const& name) {
    return concat("Counters Loading Event ", name);
}

void table_loading_error(std::string const& name, std::string const& text) {
    Log::error("Exception during loading db event ", name, ". Error:", text);
    health_register::critical(table_loading_event(name),text);
}


void counter_loading_error(std::string const& name, std::string const& text) {
    Log::error("Exception during loading db event ", name, ". Error:", text);
    health_register::critical(counters_loading_event(name),text);
}

void tables_loading(loading_table_task_ft task, std::string const &name,
                    loading_info const &info) {
/*
    auto db = app::asterisk_source_conn();
    try {
        task(db.get());
        delete_event(db.get(), name, info.versions_);
    } catch (std::exception &ex) {
        table_loading_error(name, ex.what());
    } catch (...) {
        table_loading_error(name, "Unknown error");
    }
*/
}

void counters_loading(loading_counter_task_ft task, std::string const &name, loading_info const &info) {
//    auto db = app::asterisk_output_conn();
//    try {
//        task(db.get(), info.ids_);
//        delete_event(db.get(), name, info.versions_);
//    } catch (std::exception &ex) {
//        counter_loading_error(name, ex.what());
//    } catch (...) {
//        counter_loading_error(name, "Unknown error");
//    }
}

bool loader::start_load_event(loading_info&& info) {

    info.shrink();
/*
    if (auto task = loading_task_by_name(info.event_name_); task != nullptr ) {

        add_load_event(info.event_name_);

        app::run_async_task([obj = shared_from_this(), task, info = std::move(info)] {
            tables_loading(task, info.event_name_, info);
            obj->rem_load_event(info.event_name_);
        });

        return true;
    } else if (auto task = loading_counter_task_by_name(info.event_name_); task != nullptr){
        add_load_event(info.event_name_);

        app::run_async_task([obj = shared_from_this(), task, info = std::move(info)] {
            counters_loading(task, info.event_name_, info);
            obj->rem_load_event(info.event_name_);
        });

        return true;
    } else {
        delete_event(app::asterisk_source_conn().get(), info.event_name_, info.versions_);
        return false;
    }
*/
}

std::unordered_map<std::string, loading_table_task_ft> event_tasks = {

/*
    {"billing_uu_sim_card",
     [](BDb *db) { Container::instance()->simCard.load(db); }},
    {"billing_uu_sim_imsi_partner",
     [](BDb *db) {
        Container::instance()->simPartner.load(db);
        Container::instance()->clientsImsi.load(db);}},
    {"billing_uu_sim_imsi",
     [](BDb *db) {
        Container::instance()->simImsi.load(db);
        Container::instance()->clientsImsi.load(db);
    }},
    {"service_number",
     [](BDb *db) {
        Container::instance()->serviceNumber.load(db);
        Container::instance()->clientsImsi.load(db);
    }},
    {"billing_uu_package",
     [](BDb *db) { Container::instance()->nnpPackage.load(db); }},
    {"billing_uu_account_tariff_light",
     [](BDb *db) { Container::instance()->nnpAccountTariffLight.load(db); }},
    {"clients", [](BDb *db) {
        Container::instance()->client.load(db);
        Container::instance()->clientsImsi.load(db);
    }},
    {"billing_uu_package_data",
     [](BDb *db) { Container::instance()->packageData.load(db); }},
    {"billing_uu_package_sms",
     [](BDb *db) { Container::instance()->packageSms.load(db); }},
    {"billing_uu_pricelist",
     [](BDb *db) { Container::instance()->nnpPricelist.load(db); }},
    {"billing_uu_pricelist_location",
     [](BDb *db) { Container::instance()->nnpPricelistLocation.load(db); }},
    {"billing_uu_pricelist_prefix_price",
     [](BDb *db) { Container::instance()->nnpPricelistPrefixPrice.load(db); }},
    {"billing_uu_pricelist_filter_a",
     [](BDb *db) { Container::instance()->nnpPricelistFilterA.load(db); }},
    {"billing_uu_pricelist_filter_b",
     [](BDb *db) { Container::instance()->nnpPricelistFilterB.load(db); }},
    {"camel_gt",
     [](BDb *db) { Container::instance()->camelGt.load(db);}},
    {"camel_trunk",
     [](BDb *db) { Container::instance()->camelTrunk.load(db);}},
    {"camel_trunk_number_preprocessing",
     [](BDb *db) { Container::instance()->camelTrunkNumberPreprocessing.load(db);}},
    {"camel_route_table",
     [](BDb *db) { Container::instance()->camelRouteTable.load(db);}},
    {"camel_route_table_route",
            [](BDb *db) { Container::instance()->camelRouteTableRoute.load(db); }},
    {"camel_outcome",
     [](BDb* db) { Container::instance()->camelOutcome.load(db); }},
    {"prefixlist",
     [](BDb *db) {
        Container::instance()->prefixlist.load(db);
        Container::instance()->number.load(db);
    }},
    {"prefixlist_prefix",
     [](BDb* db) { Container::instance()->prefixlistPrefix.load(db); }},
    {"number",
     [](BDb* db) { Container::instance()->number.load(db); }},
    {"nnp_number_range",
     [](BDb* db){ Container::instance()->nnpNumberRange.load(db);}},
    {"nnp_country",
    [](BDb* db){ Container::instance()->nnpCountry.load(db);}},
    {"nnp_operator",
    [](BDb* db){ Container::instance()->nnpOperator.load(db);}},
    {"nnp_region",
    [](BDb* db){ Container::instance()->nnpRegion.load(db);}}
*/
};

loading_table_task_ft
loader::loading_task_by_name(std::string const &tab_name) {
    if (event_tasks.find(tab_name) != event_tasks.end()) {
        return event_tasks.at(tab_name);
    }
    return nullptr;
}

std::unordered_map<std::string, loading_counter_task_ft> counter_tasks = {

/*
    {"ocs_imsi_sms_counters",
     [](BDb *db, std::vector<long long int> const &ids) {
         Counters::instance()->imsiSms.load(db, ids);
         QuotaManager::updateSmsQuotas(ids);
     }},
    {"ocs_imsi_voice_counters",
     [](BDb *db, std::vector<long long int> const &ids) {
         Counters::instance()->imsiVoice.load(db, ids);
         QuotaManager::updateVoiceQuotas(ids);
     }},
    {"ocs_imsi_data_counters",
     [](BDb *db, std::vector<long long int> const &ids) {
         Counters::instance()->imsiData.load(db, ids);
         QuotaManager::updateDataQuotas(ids);
     }},
    {"ocs_imsi_service_register",
     [](BDb *db, std::vector<long long int> const &ids) {
         Counters::instance()->imsiServiceRegister.load(db, ids);
     }}
*/
};

loading_counter_task_ft
loader::loading_counter_task_by_name(std::string const &tab_name) {
    auto it= counter_tasks.find(tab_name);
    if (it != counter_tasks.end()){
        return it->second;
    }
    return nullptr;
}

} // namespace

void start_data_loader() {
    auto ldr = std::make_shared<loader>();
    app::register_cyclic_task([ldr](){ ldr->load(); }, "loader_task", 1000);
}
