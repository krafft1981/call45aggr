#pragma once

#include <atomic>
//#include <map>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "Puller.h"
#include <db/BDb.h>
#include <systems/Singleton.h>
#include "PullWorker.h"
#include "SharedSpeedLimiter.h"

class PullManager:
    public Singleton<PullManager>
{
    friend class Singleton<PullManager>;
    PullManager();

    // Формирует хвост запроса, который убирает
    // все возможные запущенные событий
    std::string excludeEventsSql();

public:
    int get_errors_count() const;
    int get_event_count() const;
    std::vector<const Puller*> get_pulls() const;
    void pull();

private:

    // Механизм инициализации
    // Реализован в PullersList
    void add(
            const Event& event,
            const SrcTable& src_table,
            const DstTable& dst_table,
            const Key& key,
            const SrcSqlWhere& src_sql_where,
            const Fields& fields,
            PullerPriority priority);
    void init();

    void clear_event_table();

    const int part_size = 250;
    const double bandwidth_limit_mbits;
    std::unordered_map<std::string, std::unique_ptr<Puller>> pullers;
    std::map<PullerPriority, PullWorker> puller_workers;

    SharedSpeedLimiter speed_limiter;
    int errors_count;

};
