#pragma once

#include <set>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

#include <systems/Timer.h>
#include "ArgumentsWrapper.h"

class BDb;

using std::string;
using std::tuple;

enum class PullerPriority {
    Critical = 0,
    High  = 1,
    Low = 2
};


class Puller {
public:
    struct Info {
        Timer timer;
        int pull_count_full = 0;
        int pull_count_partial = 0;
        int pull_count_errors = 0;
        std::size_t last_copied_bytes = 0;
        std::size_t total_copied_bytes = 0;
    };

    Puller(
        const Event& event,
        const SrcTable& src_table,
        const DstTable& dst_table,
        const Key& key,
        const SrcSqlWhere& src_sql_where,
        const Fields& fields,
        PullerPriority  priority
    );

    ~Puller();

    const std::string& get_event() const;
    Info get_info() const;
    size_t copied_bytes_in_last_operation() const;

    void pull(std::function<void(size_t)> delayer, std::vector<int64_t> const& ids, std::vector<int64_t> const& versions) const;

    void clear();

    PullerPriority priority() const;

private:
    std::string getQueryFields() const;
    //std::string getFilterIds() const;

    // \brief создает запросы в зависимости от параметров
    // \param values - значения для вычисления запросов
    // \return условие для запроса строк и запрос на чистку таблицы
    tuple<string, string> createUpdateQueries(std::vector<int64_t> const &values) const;

    bool is_full_pull_required(std::vector<int64_t> const& ids) const;

    const std::string _event;
    const std::string _src_table;
    const std::string _dst_table;
    const std::string _key;
    const std::string _src_sql_where;
    const std::vector<std::string> _fields;
    const PullerPriority _priority;
    const bool _only_full_pull_is_supported;

    bool _full_pull_required = true;
    std::set<std::string> _ids_to_pull;
    mutable size_t        copied_bytes_;

    mutable std::mutex _lock; // _lock is used only for info, because all other fields are accessed only from ThreadSync
    mutable Info _info;


    // Специальное состояние для зачистки в основном потоке
    mutable std::mutex            clean_mutex_;
    mutable std::vector<int64_t>  versions_to_clean_;
};

std::ostream& operator << (std::ostream& os, PullerPriority priority);
