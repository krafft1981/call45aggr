#include "BDb.h"

#include <algorithm>
#include <cstdlib>
#include <errno.h>

#include <boost/algorithm/string.hpp>

#include <common.hpp>
#include "DbException.h"

#ifdef OLD_LOG
#include <log/old_log.hpp>
#else
#include <log/log.hpp>
#endif

//#include "../logging/Log.h"
#include "../systems/Timer.h"
#include "SpeedLimiter.h"
#include <common/split.h>
#include <common/join_through.hpp>

namespace {

std::string removeFramingBrackets(const std::string& s) {
    if ((s.length() < 2) || (s.front() != '(') || (s.back() != ')')) {
        throw std::logic_error{ std::string{"extractFromBrackets: invalid input \""} + s + "\"" };
    }
    return s.substr(1, s.length() - 2);
}

std::vector<std::string> splitValues(const std::string& values_str) {
    std::vector<std::string> values = split_any(values_str, ",", split_opts::skip_blank);
    for (auto& value: values) {
        boost::algorithm::trim(value);
        if (value.empty()) {
            throw std::logic_error{ std::string{"splitValues: invalid values_str \""} + values_str + "\"" };
        }
    }
    return values;
}

class PGresultPtr {
public:
    PGresultPtr(PGresult* result):
        _result{ result }
    {}

    ExecStatusType status() const {
        return PQresultStatus(_result.get());
    }

private:
    struct Deleter {
        void operator()(PGresult* pg_result) const {
            PQclear(pg_result);
        }
    };

    const std::unique_ptr<PGresult, Deleter> _result;
};

class PGBufferFinalizer {
public:
    PGBufferFinalizer(char** pBuffer):
        _pBuffer{ pBuffer }
    {}

    ~PGBufferFinalizer() {
        char* const buffer = *_pBuffer;
        if (buffer != nullptr) {
            PQfreemem(buffer);
        }
    }

private:
    PGBufferFinalizer(const PGBufferFinalizer&) = delete;
    PGBufferFinalizer(PGBufferFinalizer&&) = delete;
    PGBufferFinalizer& operator=(const PGBufferFinalizer&) = delete;
    PGBufferFinalizer& operator=(PGBufferFinalizer&&) = delete;

    char** const _pBuffer;
};

class BDConnectScope {
public:
    BDConnectScope(BDb* db):
        _wasConnected{ db->connected() },
        _db{ db }
    {
        if (_wasConnected) {
            return;
        }
        if (!_db->connect()) {
            throw Exception{ "Database connection error" };
        }
    }

    ~BDConnectScope() {
        if (_wasConnected) {
            return;
        }
        _db->disconnect();
    }

private:
    BDConnectScope(const BDConnectScope&) = delete;
    BDConnectScope(BDConnectScope&&) = delete;
    BDConnectScope& operator=(const BDConnectScope&) = delete;
    BDConnectScope& operator=(BDConnectScope&&) = delete;

    const bool _wasConnected;
    BDb* const _db;
};

} // namespace

BDb::BDb() {
    this->conn = 0;
    this->need_advisory_lock = 0;
}

BDb::BDb(const string &connstr) {
    this->conn = 0;
    this->connstr = connstr;
    this->need_advisory_lock = 0;
}

BDb::~BDb() {
    disconnect();
}

void BDb::setCS(const string &connstr) {
    this->connstr = connstr;
}

PGconn * BDb::getConn() {
    return conn;
}

bool BDb::connect() {
    if (connected()) return true;
    conn = PQconnectdb(connstr.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
        DbException e(conn);
        disconnect();
        throw e;
    }

    if (need_advisory_lock != 0) {
        string query = "select pg_try_advisory_lock(" + to_string(need_advisory_lock) + ") as locked";
        PGresult *res = PQexec(conn, query.c_str());
        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            DbException e(conn, "BDb::try_advisory_lock");
            PQclear(res);
            disconnect();
            throw e;
        }
        if ('t' != *PQgetvalue(res, 0, 0)) {
            PQclear(res);
            disconnect();
            throw Exception("Can not lock key" + to_string(need_advisory_lock), "BDb::try_advisory_lock");
        }
        if (res != nullptr) {
            PQclear(res);
        }
    }

    {
        PGresult *res = PQexec(conn, "set session time zone '+00'");
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            DbException e(conn, "BDb::connect set timezone");
            PQclear(res);
            disconnect();
            throw e;
        }
        if (res != nullptr) {
            PQclear(res);
        }
    }

    // Вывод игнорируется, т.к. устанавливается только в этом месте
    (void)PQsetNoticeProcessor(conn,[](void* obj, char const* message){
        reinterpret_cast<BDb*>(obj)->on_notice(message);
    }, this);

    return true;
}

bool BDb::connected() {
    return conn != 0;
}

void BDb::disconnect() {
    if (conn != 0) {
        PQfinish(conn);
        conn = 0;
    }
}

void BDb::needAdvisoryLock(int key) {
    need_advisory_lock = key;
}

void BDb::exec(const char * squery) {

    noticies_.clear();

#ifndef OLD_LOG
    Log::debug("[", __PRETTY_FUNCTION__ , "] query: " , squery, log_delay<120>());
#endif

    int attemps_count = connected() ? 2 : 1;

    string errors;

    while (attemps_count-- > 0) {
        try {
            connect();

            PGresult *res = PQexec(conn, squery);
            if (PQresultStatus(res) != PGRES_COMMAND_OK) {
                DbException e(conn);
                PQclear(res);
                res = 0;
                if ( !ping() )
                    disconnect();
                throw e;
            }

            if (res != 0) {
                PQclear(res);
            }
            return;
        } catch (DbException &e) {
            errors += e.what() + "\n";
        }
    }

//   char short_query[256];
//    strncpy(short_query, squery, sizeof (short_query) - 1);
//    errors += "SQL: ";
//    errors += short_query;
    throw DbException(errors, "BDb::exec");
}

void BDb::exec(const ParamQuery& query) {
    try {
        connect();

        for (size_t row_idx = 0; row_idx < query.levelsCount(); ++row_idx) {

            // Пустые уровни могут быть случайно добавлены
            if (query.isEmpty(row_idx)) {
                continue;
            }

            PGresult *res = PQexecParams(conn, query.getQuery().data(), static_cast<int>(query.getWidth()),
                                         nullptr, query.parameters(row_idx), nullptr, nullptr, 0);

            ExecStatusType resultCode = PQresultStatus(res);
            if (resultCode != PGRES_TUPLES_OK && resultCode != PGRES_COMMAND_OK) {
                DbException e(conn);
                PQclear(res);
                res = 0;
                if (!ping())
                    disconnect();
                throw e;
            }
            if (res != 0) {
                PQclear(res);
            }
        }
    } catch (DbException &e) {
        e.addTrace (std::string{__FILE__} + ":" + std::to_string(__LINE__));
        throw;
    }

}

BDbResult BDb::query(const ParamQuery& query) {

    noticies_.clear();

    try {
        connect();

        PGresult *res = PQexecParams(conn, query.getQuery().data(), static_cast<int>(query.getWidth()),
                                     nullptr, query.parameters(), nullptr,
                                     nullptr, 0);
        ExecStatusType resultCode = PQresultStatus(res);
        if (resultCode != PGRES_TUPLES_OK && resultCode != PGRES_COMMAND_OK) {
            DbException e(conn);
            PQclear(res);
            if (!ping())
                disconnect();
            throw e;
        }
        if (!res) {
            throw DbException(std::string{"Query didn't return result:"} + query.getQuery());
        }
        return BDbResult {res};

    } catch (DbException &e) {
        e.addTrace (std::string{__FILE__} + ":" + std::to_string(__LINE__));
        throw e;
    }

}


void BDb::exec(const string &squery) {
    exec(squery.c_str());
}

BDbResult BDb::query(const char * squery) {

    noticies_.clear();

#ifndef OLD_LOG
    Log::debug("[", __PRETTY_FUNCTION__ , "] query: " , squery, log_delay<120>());
#endif

    int attemps_count = connected() ? 2 : 1;

    string errors="BDb::query("+string(squery)+"): ";

    while (attemps_count-- > 0) {
        try {
            connect();

            PGresult *res = PQexec(conn, squery);
            if (PQresultStatus(res) != PGRES_TUPLES_OK) {
                DbException e(conn);
                PQclear(res);
                res = 0;
                disconnect();
                throw e;
            }

            if (res != 0) {
                return BDbResult(res, std::move(noticies_));
            } else {
                disconnect();
            }
        } catch (DbException &e) {
            errors += e.what() + "\n";
        }
    }

    //char short_query[256];
    //strncpy(short_query, squery, sizeof (short_query) - 1);
    //errors += "SQL: ";
    //errors += short_query;
    throw DbException(errors, "BDb::query");
}

BDbResult BDb::query(const string &squery) {
    return query(squery.c_str());
}

BDbResult BDb::query_reduce_select(
    const std::string& columns, const std::string& table, const std::string& keyColumn, const std::string& keyValues, const std::string& reducingCondition
) {
    const auto values = splitValues(removeFramingBrackets(boost::algorithm::trim_copy(keyValues)));
    if (values.empty()) {
        throw std::logic_error{"BDb::query_reduce_select: keyValues is empty"};
    }

    const auto singleValueSelectStatement = [&] (std::size_t i) {
        return std::string{"select "} + columns + " from " + table + " where " + keyColumn + "=" + values.at(i) + " " + reducingCondition;
    };

    std::string squery;
    if (values.size() == 1) {
        squery = singleValueSelectStatement(0);
    } else {
        squery.push_back('(');
        for (std::size_t i = 0; i < values.size(); ++i) {
            if (i > 0) {
                squery += "union";
            }
            squery.push_back('(');
            squery += singleValueSelectStatement(i);
            squery.push_back(')');
        }
        squery.push_back(')');
        squery += reducingCondition;
    }

    return query(squery);
}

BDbResult BDb::query_fastcalc_last_id(
        const std::string& columns, const std::string& schema_table,  const std::string& timeColumn , const std::string& keyValues)
{
    const auto values = splitValues(removeFramingBrackets(boost::algorithm::trim_copy(keyValues)));
    if (values.empty()) {
        throw std::logic_error{"BDb::query_reduce_select: keyValues is empty"};
    }

    std::string squery_sub;

    for (auto& x : values) {
        squery_sub += x + std::string{","};
    }

    squery_sub.pop_back();

    std::string squery = std::string{"select "} + columns + std::string{" from billing.get_hub_last_id('"} +
                         schema_table + std::string{"','"} + timeColumn + std::string{"','{"} + squery_sub + std::string{"}'::integer[]);"};

    return query(squery);
}

bool BDb::rel_exists (string rel) {

    string check = "SELECT to_regclass('" + rel + "')";
    auto res = query(check.c_str());
    return res.next() && res.get_s(0).size()>0;
}

std::size_t BDb::copy(
        const std::string& src_table, const std::string& dst_table, const std::string& columns,
        const std::string& condition, BDb* db_src, BDb* db_dst, double bandwidth_limit_mbits
) {
    return copy_ex(src_table, dst_table, columns, columns, condition, db_src, db_dst, bandwidth_limit_mbits);
}

std::size_t BDb::copy_ex(
        const std::string& src_table, const std::string& dst_table, const std::string& src_columns, const std::string& dst_columns,
        const std::string& condition, BDb* db_src, BDb* db_dst, double bandwidth_limit_mbits
) {
    SpeedLimiter limiter(bandwidth_limit_mbits);
    return copy_ex(src_table, dst_table, src_columns, dst_columns, condition, db_src, db_dst,
                   [&](size_t bytes_read) mutable {
                       limiter.on_progress_updated(bytes_read);
                   });
}

std::size_t BDb::copy(const std::string &src_table, const std::string &dst_table, const std::string &columns,
                      const std::string &condition, BDb *db_src, BDb *db_dst, OnCopyCallback bandwidthCallback) {
    return copy_ex(src_table, dst_table, columns, columns, condition, db_src, db_dst, bandwidthCallback);
}

std::size_t BDb::copy_ex(const std::string &src_table, const std::string &dst_table, const std::string &src_columns,
                         const std::string &dst_columns, const std::string &condition, BDb *db_src, BDb *db_dst,
                         OnCopyCallback bandwidthCallback) {

    if (src_columns.empty() || dst_columns.empty()) {
        throw std::logic_error{ "empty columns string is not supported. Use \"*\" instead" };
    }

    std::string select_query = std::string{ "select " } + src_columns + " from " + src_table;
    if (!condition.empty()) {
        select_query += std::string{" where "} + condition;
    }
    const std::string dst_table_with_columns = dst_columns == "*" ? dst_table : dst_table + "(" + dst_columns + ")";

    const std::string query_read = std::string{ "copy (" } + select_query + ") to stdout with binary";
    const std::string query_write = std::string{ "copy " } + dst_table_with_columns + " from stdin with binary";

    const BDConnectScope db_src_connect_scope{ db_src };
    const BDConnectScope db_dst_connect_scope{ db_dst };

    if (PGresultPtr{PQexec(db_src->getConn(), query_read.c_str())}.status() != PGRES_COPY_OUT) {
        throw DbException{ db_src->getConn(), "BDb::copy::from" };
    }

    if (PGresultPtr{PQexec(db_dst->getConn(), query_write.c_str())}.status() != PGRES_COPY_IN) {
        throw DbException{ db_dst->getConn(), "BDb::copy::to" };
    }

    std::size_t total_bytes_read = 0;
    bandwidthCallback(0);
    while (true) {
        char* buffer = nullptr;
        const PGBufferFinalizer bufferFinalizer{ &buffer };
        const int bytes_read = PQgetCopyData(db_src->getConn(), &buffer, 0);
        if ((buffer != nullptr) && (bytes_read > 0)) {
            if (PQputCopyData(db_dst->getConn(), buffer, bytes_read) != 1) {
                throw DbException{ "error", "BDb::copy::transfer" };
            }
        }
        if (bytes_read < 0) {
            break;
        }
        total_bytes_read += static_cast<std::size_t>(bytes_read);
        bandwidthCallback(bytes_read);
    }

    if (PGresultPtr{PQgetResult(db_src->getConn())}.status() != PGRES_COMMAND_OK) {
        throw DbException{ db_src->getConn(), "BDb::copy::from::result" };
    }

    if (PQputCopyEnd(db_dst->getConn(), nullptr) != 1) {
        throw DbException{ db_dst->getConn(), "BDb::copy::to::end" };
    }

    if (PGresultPtr{PQgetResult(db_dst->getConn())}.status() != PGRES_COMMAND_OK) {
        throw DbException{ db_dst->getConn(), "BDb::copy::to::result" };
    }

    return total_bytes_read;
}

bool BDb::ping() {
    try {
        BDbResult res = query("SELECT 9 as result");
        return (res.size() == 1) && (atoi(res.get(0, "result")) == 9);
    } catch (Exception &e) {
        Log::error(e.what());
    }
    return false;
}

bool BDb::test(const string &connstr) {
    BDb db(connstr);
    return db.ping();
}

void BDb::on_notice(char const * mes) {
    noticies_.push_back(mes);
}
