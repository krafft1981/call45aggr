
#include <ctime>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <time.h>

#include "BDbResult.h"
#ifdef OLD_LOG
#include "log/old_log.hpp"
#else
#include "log/log.hpp"
#endif
#include "common/join_through.hpp"

BDbResult::BDbResult(PGresult * resArg, std::vector<std::string>&& noticies)
    :
    res(resArg),
    count(PQntuples(resArg)),
    index(-1),
    noticies_(std::move(noticies)){
    if (!noticies_.empty()) {
#ifndef OLD_LOG
        Log::debug("DBNoticies. ", join_through(noticies_, "\n"), log_delay<120>());
#endif
    }
}

BDbResult::BDbResult(BDbResult&& bd) noexcept{
    if (this == &bd)
        return ;

    index = bd.index;
    res = bd.res;
    count = bd.count;

    noticies_ = std::move(bd.noticies_);

    bd.index = -1;
    bd.count = 0;
    bd.res = nullptr;
}

BDbResult& BDbResult::operator=(BDbResult&& bd) noexcept{
    if (this == &bd)
        return *this;

    if (res) {
        PQclear(res);
    }
    res = bd.res;
    index = bd.index;
    count = bd.count;

    noticies_ = std::move(bd.noticies_);

    bd.res = nullptr;
    bd.index = -1;
    bd.count = 0;

    return *this;
}

BDbResult::~BDbResult() {
    if (res) {
        PQclear(res);
        res = nullptr;
    }
    index = -1;
    count = 0;
}

size_t BDbResult::size() {
    return (size_t)count;
}

size_t BDbResult::position() {
    return (size_t)index;
}

char * BDbResult::get(int r, const char* name) {
    int fid = PQfnumber(res, name);
    return PQgetvalue(res, r, fid);
}

bool BDbResult::is_null(const char* name) {
    int fid = PQfnumber(res, name);
    return (bool)PQgetisnull(res, index, fid);
}

char * BDbResult::get(const char* name) {
    int fid = PQfnumber(res, name);
    return PQgetvalue(res, index, fid);
}

int BDbResult::get_i(const char* name) {
    int fid = PQfnumber(res, name);
    return atoi(PQgetvalue(res, index, fid));
}

time_t BDbResult::get_t(const char* name) {
    int fid = PQfnumber(res, name);
    struct std::tm tm;
    std::istringstream iss(PQgetvalue(res, index, fid));
    iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    return mktime(&tm);
}

double BDbResult::get_d(const char* name) {
    int fid = PQfnumber(res, name);
    return atof(PQgetvalue(res, index, fid));
}

long long int BDbResult::get_ll(const char* name) {
    int fid = PQfnumber(res, name);
    return atoll(PQgetvalue(res, index, fid));
}

bool BDbResult::get_b(const char* name) {
    int fid = PQfnumber(res, name);
    return 't' == *PQgetvalue(res, index, fid);
}

string BDbResult::get_s(const char* name) {
    int fid = PQfnumber(res, name);
    return PQgetvalue(res, index, fid);
}

void BDbResult::fill_cs(const char* name, char * str, size_t size) {
    int fid = PQfnumber(res, name);
    strncpy(str, PQgetvalue(res, index, fid), size - 1);
}

bool BDbResult::next() {
    index++;
    return count > 0 && index < count;
}

void BDbResult::set(int posid) {
    if (index < count) {
        index = posid;
    }
}

bool BDbResult::previous() {
    index--;
    return count > 0 && index < count;
}

bool BDbResult::first() {
    if (count > 0) {
        index = 0;
        return true;
    } else
        return false;
}

bool BDbResult::last() {
    if (count > 0) {
        index = count - 1;
        return true;
    } else
        return false;
}

void BDbResult::force_free() {
    if (res != 0) {
        PQclear(res);
        res = 0;
    }
}
