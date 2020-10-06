#pragma once

#ifdef CATCH_TESTS

#include "MockBDbResult.h"

#else

#include <string>
#include <libpq-fe.h>
#include <vector>

using namespace std;

class BDbResult {
protected:
    PGresult * res;
    int count;
    int index;
    std::vector<std::string> noticies_;
public:
    explicit BDbResult(PGresult * res, std::vector<std::string> && noticies = {});
    BDbResult(const BDbResult&) = delete;
    BDbResult(BDbResult&&) noexcept;
    BDbResult& operator=(const BDbResult&) = delete;
    BDbResult& operator=(BDbResult&& res) noexcept;

    ~BDbResult();
    size_t size();
    size_t position();
    char * get(int r, const char* name);
    bool is_null(const char* name);
    char * get(const char* name);
    int get_i(const char* name);
    double get_d(const char* name);
    long long int get_ll(const char* name);
    bool get_b(const char* name);
    time_t get_t(const char* name);
    string get_s(const char* name);
    void fill_cs(const char* name, char * str, size_t size);
    bool next();
    void set(int posid);
    bool previous();
    bool first();
    bool last();
    void force_free();
};
#endif