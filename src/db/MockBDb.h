#pragma once

#include "MockBDbResult.h"
#include "MockData.h"

class ParamQuery;

// Просто список строк, который рассматривается как результат определенного запроса
class MockBDb {
protected:
    MockData data_;
public:
    MockBDb(MockData data);

    MockBDb(initializer_list<string> list);

    MockBDb(char const* src);

    MockBDb(MockBDb const& ) =default;

    MockBDb(MockBDb&&) = default;

    ~MockBDb();

    void setCS(const string &connstr);

    string getCS() { return ""; };

    //PGconn *getConn();

    bool connect();

    bool connected();

    void disconnect();

    void needAdvisoryLock(int key);

    void exec(const char *squery);

    void exec(const string &squery);

    void exec(const ParamQuery &query);

    MockBDbResult query(const char *squery);

    MockBDbResult query(const string &squery);

    MockBDbResult query(const ParamQuery &query);

    bool ping();

    bool rel_exists(string rel);
};

// forward declaration workaround
class BDb : public MockBDb {
public:
    BDb(MockBDb const& value)
            : MockBDb(value)
    {}

    template<typename ... QueryParts>
    void exec2(QueryParts&& ... parts) {}

    template<typename ... QueryParts>
    BDbResult query2(QueryParts&& ... parts) {
        // Результат запроса берется из подгруженных данных
        return query("");
    }
};
