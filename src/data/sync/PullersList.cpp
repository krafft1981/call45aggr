#include "PullManager.h"
//#include "../classes/AppBill.h"
//#include "../classes/Config.h"
/*namespace {

std::string defs_src_sql_where() {
    if (!app().conf.isApiHostMode()) {
        return
            std::string{}
            +
            " ( not deleted and "
#ifndef TESTING_ENABLED
            " date_to > ( now() - '2 month'::interval ) and "
#endif
              "  pricelist_id in (select id from voip.pricelist where region in "
            + app().conf.get_sql_regions_list() + " or is_global ) ) ";
    }
    return {};
}

std::string pricelist_src_sql_where() {
    if (!app().conf.isApiHostMode()) {
        return
            std::string{}
            + " ( region  in "
            + app().conf.get_sql_regions_list()
            + " or is_global ) ";
    }
    return {};
}

}
*/

void PullManager::add(
        const Event& event,
        const SrcTable& src_table,
        const DstTable& dst_table,
        const Key& key,
        const SrcSqlWhere& src_sql_where,
        const Fields& fields,
        PullerPriority priority){
    // TODO new -> make_unique
    pullers[event.data()] = std::unique_ptr<Puller>(new Puller(event, src_table,dst_table, key, src_sql_where, fields, priority));
}

void PullManager::init(){

/*
    add(Event{"billing_uu_sim_card"},
        SrcTable{"billing_uu.sim_card"},
        DstTable{"nispd_billing_uu.sim_card"},
        Key{"iccid"},
        SrcSqlWhere{""},
        Fields{"imei", "iccid", "client_account_id", "is_active", "status_id"},
        PullerPriority::Critical);

    add(Event{"billing_uu_sim_imsi_partner"},
        SrcTable{"billing_uu.sim_imsi_partner"},
        DstTable{"nispd_billing_uu.sim_imsi_partner"},
        Key{"id"},
        SrcSqlWhere{""},
        Fields{"id",
               "name",
               "term_trunk_id",
               "orig_trunk_id",
               "is_active",
               "mvno_region_id",
               "is_default",
               "location_id",
               "fake_lu"},
        PullerPriority::Critical);

    add(Event{"billing_uu_sim_imsi"},
        SrcTable{"billing_uu.sim_imsi"},
        DstTable{"nispd_billing_uu.sim_imsi"},
        Key{"imsi"},
        SrcSqlWhere{""},
        Fields{"imsi",
               "iccid",
               "msisdn",
               "did",
               "is_anti_cli",
               "is_roaming",
               "actual_from",
               "is_active",
               "status_id",
               "actual_to",
               "partner_id",
               "is_default"},
        PullerPriority::Critical);


    // Таблица имеет приоритет High. Может быть случай, когда 
    // резервный сервер выполнит запрос к этой таблице без 
    // обновленной таблицы public.server (приоритет Critical)
    // В этом случае в таблице будут неверные данные, но такой
    // случай крайне маловероятен. 
    add(Event{"service_number"},
        SrcTable{"billing.service_number"},
        DstTable{"nispd_billing.service_number"},
        Key{"id"},
        SrcSqlWhere{""},
        Fields{"id",
               "server_id",
               "client_account_id",
               "did",
               "activation_dt",
               "expire_dt",
               "lines_count",
               "tech_number",
               "tech_number_operator_id",
               "fmc_trunk_id",
               "mvno_trunk_id",
               "route_to_vats_trunk_enable",
               "is_mobile_outbound",
               "route_to_vats_trunk_hard_enable"},
        PullerPriority::High);


    add(Event{"nnp_package"},
        SrcTable{"billing_uu.package"},
        DstTable{"nispd_billing_uu.package"},
        Key{"tariff_id"},
        SrcSqlWhere{""},
        Fields{"tariff_id",
               "service_type_id",
               "tarification_free_seconds",
               "tarification_interval_seconds",
               "tarification_type",
               "tarification_min_paid_seconds",
               "currency_id",
               "is_include_vat",
               "is_termination",
               "location_id",
               "price_min"},
        PullerPriority::Critical);

    add(Event{"nnp_account_tariff_light"},
        SrcTable{"billing_uu.account_tariff_light"},
        DstTable{"nispd_billing_uu.account_tariff_light"},
        Key{"id"},
        SrcSqlWhere{""},
        Fields{"id",
               "account_tariff_id",
               "account_client_id",
               "tariff_id",
               "activate_from",
               "deactivate_from",
               "coefficient",
               "price",
               "service_type_id"},
        PullerPriority::High);



    add(Event{"client"},
        SrcTable{"billing.clients"},
        DstTable{"nispd_billing.clients"},
        Key{"id"},
        SrcSqlWhere{""},
        Fields{"id",
               "voip_limit_day",
               "voip_limit_mn_day",
               "voip_disabled",
               "balance",
               "credit",
               "amount_date",
               "last_payed_month",
               "organization_id",
               "price_include_vat",
               "timezone_offset",
               "is_blocked",
               "anti_fraud_disabled",
               "account_version",
               "effective_vat_rate",
               "currency"},
        PullerPriority::Critical);

    add(Event{"nnp_package_data"},
        SrcTable{"billing_uu.package_data"},
        DstTable{"nispd_billing_uu.package_data"},
        Key{"id"},
        SrcSqlWhere{""},
        Fields{"id",
               "tariff_id",
               "nnp_pricelist_id"},
        PullerPriority::Critical
    );


    add(Event{"nnp_package_sms"},
        SrcTable{"billing_uu.package_sms"},
        DstTable{"nispd_billing_uu.package_sms"},
        Key{"id"},
        SrcSqlWhere{""},
        Fields{"id",
               "tariff_id",
               "nnp_pricelist_id"},
        PullerPriority::Critical
    );


    add(Event{"nnp_package_sms"},
        SrcTable{"billing_uu.package_sms"},
        DstTable{"nispd_billing_uu.package_sms"},
        Key{"id"},
        SrcSqlWhere{""},
        Fields{"id",
               "tariff_id",
               "nnp_pricelist_id"},
        PullerPriority::Critical
    );

    add(Event{"number"},
        SrcTable{"auth.number"},
        DstTable{"auth.number"},
        Key{"id"},
        SrcSqlWhere{""},
        Fields{"id", "server_id", "name", "prefixlist_ids", "sw_shared"},
        PullerPriority::Critical);


    add(Event{"prefixlist"},
        SrcTable{"auth.prefixlist"},
        DstTable{"auth.prefixlist"},
        Key{"id"},
        SrcSqlWhere{""},
        Fields{"id",
               "server_id",
               "name",
               "type_id",
               "is_auto_update",
               "dt_update",
               "dt_prepare",
               "sw_shared",
               "nnp_filter_json",
               "version",
               "invert",
               "normalization_disabled",
               "strong_matching"
        },
        PullerPriority::High);

    add(Event{"prefixlist_prefix"},
        SrcTable{"auth.prefixlist_prefix"},
        DstTable{"auth.prefixlist_prefix"},
        Key{""},
        SrcSqlWhere{""},
        Fields{"prefixlist_id", "prefix"},
        PullerPriority::High);
*/

/*
    add(Event{"server"},
        SrcTable{"public.server"},
        DstTable{"public.server"},
        Key{"id"},
        SrcSqlWhere{""},
        Fields{"id",
               "low_balance_outcome_id",
               "blocked_outcome_id",
               "calling_station_id_for_line_without_number",
               "min_price_for_autorouting",
               "our_numbers_id",
               "service_numbers",
               "hub_id",
               "emergency_prefixlist_id",
               "h_call_sync_delay",
               "h_cdr_sync_delay",
               "h_call_save_delay",
               "h_cdr_proc_wait_count",
               "h_call_save_wait_count",
               "h_thread_error_count",
               "h_radius_request_delay",
               "h_event_management",
               "h_local_events",
               "name",
               "nas_ip_address",
               "active",
               "rc_mgmn_outcome_id",
               "mcn_prefixlist_id",
               "fsb_blacklist_id",
               "vats_trunk_id",
               "rc_mgmn_action_disable",
               "ast_trunk_group_id",
               "is_route_to_class5",
               "is_route_to_class5_phase1_enable",
               "number_id_filter_b_route_to_class5",
               "cpc_id",
               "global_replacement_id",
               "fsb_b_blacklist_id",
               "is_open_numeric_plan_enabled",
               "is_autotest_error_enabled"},
        PullerPriority::Critical);
*/

/*
    add(Event{"nnp_pricelist"},
        SrcTable{"billing_uu.pricelist"},
        DstTable{"nispd_billing_uu.pricelist"},
        Key{"id"},
        SrcSqlWhere{""},
        Fields{"id",
               "name",
               "currency_id",
               "orig",
               "date_created",
               "date_start",
               "date_end",
               "pricelist_version",
               "basic_pricelist_location_id",
               "is_global",
               "is_active",
               "service_type_id"},
        PullerPriority::Low);

    add(Event{"nnp_pricelist_location"},
        SrcTable{"billing_uu.pricelist_location"},
        DstTable{"nispd_billing_uu.pricelist_location"},
        Key{"id"},
        SrcSqlWhere{""},
        Fields{"id",
               "pricelist_id",
               "location_id",
               "delta_price",
               "mcc",
               "mnc",
               "rounding_threshold"},
        PullerPriority::Low);

    add(Event{"nnp_pricelist_prefix_price"},
        SrcTable{"billing_uu.pricelist_prefix_price"},
        DstTable{"nispd_billing_uu.pricelist_prefix_price"},
        Key{"id"},
        SrcSqlWhere{""},
        Fields{"id",
               "pricelist_filter_b_id",
               "prefix_b",
               "b_number_price",
               "b_number_connect_price",
               "change_flag",
               "date_from",
               "date_to"},
        PullerPriority::Low);

    add(Event{"nnp_pricelist_filter_a"},
        SrcTable{"billing_uu.pricelist_filter_a"},
        DstTable{"nispd_billing_uu.pricelist_filter_a"},
        Key{"id"},
        SrcSqlWhere{""},
        Fields{"id",
               "pricelist_location_id",
               "nnp_destination",
               "nnp_country",
               "nnp_operator",
               "nnp_region",
               "nnp_city",
               "nnp_ndc_type",
               "mode_selected",
               "f_inv_nnp_destination",
               "f_inv_nnp_country",
               "f_inv_nnp_operator",
               "f_inv_nnp_region",
               "f_inv_nnp_city",
               "f_inv_nnp_ndc_type"
               },
        PullerPriority::Low);

    add(Event{"nnp_pricelist_filter_b"},
        SrcTable{"billing_uu.pricelist_filter_b"},
        DstTable{"nispd_billing_uu.pricelist_filter_b"},
        Key{"id"},
        SrcSqlWhere{""},
        Fields{"id",
               "pricelist_filter_a_id",
               "nnp_destination",
               "nnp_country",
               "nnp_operator",
               "nnp_region",
               "nnp_city",
               "nnp_ndc_type",
               "mode_selected",
               "interconnect_price",
               "ported_num_price",
               "tarification_free_seconds",
               "tarification_interval_seconds",
               "tarification_type",
               "tarification_min_paid_seconds",
               "f_inv_nnp_destination",
               "f_inv_nnp_country",
               "f_inv_nnp_operator",
               "f_inv_nnp_region",
               "f_inv_nnp_city",
               "f_inv_nnp_ndc_type"
               },
        PullerPriority::Low);

    add(Event{"camel_gt"},
        SrcTable{"auth.camel_gt"},
        DstTable{"auth.camel_gt"},
        Key("id"),
        SrcSqlWhere{""},
        Fields{ "id",
                "gt",
                "oper",
                "country",
                "area",
                "loc",
                "country_code",
                "region_id",
                "operator_id"
        },
        PullerPriority::High
    );

    add(Event{"camel_route_table"},
        SrcTable{"auth.camel_route_table"},
        DstTable{"auth.camel_route_table"},
        Key("id"),
        SrcSqlWhere{""},
        Fields{ "id",
                "name",
                "object_comment",
                "server_id"
        },
        PullerPriority::High
    );

    add(Event{"camel_route_table_route"},
        SrcTable{"auth.camel_route_table_route"},
        DstTable{"auth.camel_route_table_route"},
        Key(""),
        SrcSqlWhere{""},
        Fields{
              "camel_route_table_id",
              "order",
              "gt_number_id",
              "a_number_id",
              "b_number_regexp",
              "outcome_id",
              "outcome_args",
              "is_locked",
              "object_comment",
              "b_number_id"
        },
        PullerPriority::High
    );

    add(Event{"camel_trunk"},
        SrcTable{"auth.camel_trunk"},
        DstTable{"auth.camel_trunk"},
        Key("id"),
        SrcSqlWhere{""},
        Fields{
                "id",
                "name",
                "prefixlist_id",
                "camel_route_table_id",
                "server_id",
                "trunk_mcn",
                "trunk_tele2",
                "insert_cdr"
        },
        PullerPriority::High
    );

    add(Event{"camel_outcome"},
        SrcTable{"auth.camel_outcome"},
        DstTable{"auth.camel_outcome"},
        Key("id"),
        SrcSqlWhere{""},
        Fields{
                "id",
                "name",
                "type_id",
                "arguments",
                "f_use_arguments",
                "f_set_credit_limit",
                "f_check_b_number",
                "server_id"
        },
        PullerPriority::High
    );

    add(Event{"camel_trunk_number_preprocessing"},
        SrcTable{"auth.camel_trunk_number_preprocessing"},
        DstTable{"auth.camel_trunk_number_preprocessing"},
        Key("id"),
        SrcSqlWhere{""},
        Fields{
                "id",
                "camel_trunk_id",
                "order",
                "src",
                "noa",
                "length",
                "prefix",
                "abc_mode",
                "object_comment",
                "mod_type",
                "start_pos",
                "end_pos",
                "mod_value",
                "regex",
                "acc",
                "auth",
                "avoid_mod"
        },
        PullerPriority::High
    );

    add(Event{"nnp_number_range"},
        SrcTable{"nnp.number_range"},
        DstTable{"nnp.number_range"},
        Key{"id"},
        SrcSqlWhere{""},
        Fields{"id",
               "country_prefix",
               "country_code",
               "ndc",
               "number_from",
               "number_to",
               "is_mob",
               "is_active",
               "operator_id",
               "region_id",
               "insert_time",
               "insert_user_id",
               "update_time",
               "update_user_id",
               "city_id",
               "full_number_from",
               "full_number_to",
               "ndc_type_id"},
        PullerPriority::Low);

    add(Event{"nnp_country"},
        SrcTable{"nnp.country"},
        DstTable{"nnp.country"},
        Key{"code"},
        SrcSqlWhere{""},
        Fields{"code", "name", "name_rus", "prefixes", "is_open_numbering_plan",
               "use_weak_matching","default_operator", "default_type_ndc"},
        PullerPriority::Low);


    add(Event{"nnp_operator"},
        SrcTable{"nnp.operator"},
        DstTable{"nnp.operator"},
        Key{"id"},
        SrcSqlWhere{""},
        Fields{"id", "name"},
        PullerPriority::Low);

    add(Event{"nnp_region"},
        SrcTable{"nnp.region"},
        DstTable{"nnp.region"},
        Key{"id"},
        SrcSqlWhere{""},
        Fields{"id", "name", "parent_id"},
        PullerPriority::Low);
*/
}
