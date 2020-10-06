#pragma once

#ifdef CATCH_TESTS

#include "MockBDb.h"

#else

#include <libpq-fe.h>

#include <map>
#include <list>
#include <string>
#include <functional>

#include "BDbResult.h"
#include "BDbTransaction.h"
#include "ParamQuery.h"

#include <common.hpp>

using OnCopyCallback = std::function<void(size_t)>;

class BDb {
protected:
    PGconn * conn;
    string connstr;
    int need_advisory_lock;

public:
    BDb();
    BDb(const string &connstr);
    ~BDb();

    void setCS(const string &connstr);
    string getCS() {return this->connstr;};

    PGconn * getConn();
    bool connect();

    bool connected();

    void disconnect();

    void needAdvisoryLock(int key);

    template<typename ... QueryParts>
    void exec2(QueryParts&& ... parts);
    void exec(const char * squery);
    void exec(const string &squery);
    void exec(const ParamQuery& query);

    template<typename ... QueryParts>
    BDbResult query2(QueryParts&& ... parts);
    BDbResult query(const char * squery);
    BDbResult query(const string &squery);
    BDbResult query(const ParamQuery& query);

    // This method execute queries like
    // "select <columns> from <table> where <keyColumn> in <keyValues> <reducingCondition>"
    //
    // by union'ing separate requests like
    // "("
    // "  (select <columns> from <table> where <keyColumn> = <keyValues>.at(0) <reducingCondition>)"
    // "  union"
    // "  (select <columns> from <table> where <keyColumn> = <keyValues>.at(1) <reducingCondition>)"
    // "  ..."
    // ") <reducingCondition>"
    //
    // "reducingCondition" must be commutative and associative,
    // i.e. reducingCondition(x0, x1, ..., y0, y1, ...) = reducingCondition(reducingCondition(x0, x1, ...), reducingCondition(y0, y1, ...))
    // and also reducingCondition(x0, x1, ..., y0, y1, ...) = reducingCondition(reducingCondition(x0, y0), reducingCondition(x1, y1), ...), etc
    //
    // Motivation: postgresql 9.4 can't use index tree to effectively execute the first query, but it can use index tree to execute union'ed query
    //
    // For example,
    //   columns = "id, connect_time"
    //   table = "calls_raw.calls_raw_201704"
    //   keyColumn = "server_id"
    //   keyValues = "(13, 75, 76, 91, 97)"
    //   reducingCondition = "order by id desc limit 1"
    BDbResult query_reduce_select(
        const std::string& columns, const std::string& table, const std::string& keyColumn,
        const std::string& keyValues, const std::string& reducingCondition
    );

    BDbResult query_fastcalc_last_id(const string &columns, const string &schema_table, const string &timeColumn,
                                     const string &keyValues);

    static std::size_t copy(
        const std::string& src_table, const std::string& dst_table, const std::string& columns,
        const std::string& condition, BDb* db_src, BDb* db_dst, double bandwidth_limit_mbits = -1.0
    );

    static std::size_t copy_ex(
        const std::string& src_table, const std::string& dst_table, const std::string& src_columns, const std::string& dst_columns,
        const std::string& condition, BDb* db_src, BDb* db_dst, double bandwidth_limit_mbits = -1.0
    );

    // Функции для копирования таблиц. Для контроля пропускной способности принимают коллбек последним аргументом
    static std::size_t copy(
            const std::string& src_table, const std::string& dst_table, const std::string& columns,
            const std::string& condition, BDb* db_src, BDb* db_dst, OnCopyCallback bandwidthCallback
    );
    static std::size_t copy_ex(
            const std::string& src_table, const std::string& dst_table, const std::string& src_columns, const std::string& dst_columns,
            const std::string& condition, BDb* db_src, BDb* db_dst, OnCopyCallback bandwidthCallback
    );

    bool ping();
    bool rel_exists (string rel);
    static bool test(const string &connstr);

    void on_notice(char const*);

    std::vector<std::string> noticies_;

};

template<typename ... QueryParts>
inline void BDb::exec2(QueryParts&& ... parts){
    exec(concat(std::forward<QueryParts>(parts)...).c_str());
}

template<typename ... QueryParts>
inline BDbResult BDb::query2(QueryParts&& ... parts){
    return query(concat(std::forward<QueryParts>(parts)...));
}

#endif