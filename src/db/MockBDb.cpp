
#include "MockBDb.h"
#include <common.hpp>
#include <common/split.h>

MockBDb::MockBDb(MockData data)
        : data_(data)
{}

MockBDb::MockBDb(initializer_list<string> list)
    : data_(list){
}

MockBDb::MockBDb(char const* src)
    : data_({}){
    vector<string> rows = split_any(src, "\n", split_opts::skip_blank);

    //// Удаление всех пустых строк
    //rows.erase(std::remove_if(rows.begin(),rows.end(), std::mem_fn(&std::string::empty)), rows.end());

    data_ = MockData(std::move(rows));
}

MockBDb::~MockBDb() {}

void MockBDb::setCS(const string&) {}


bool MockBDb::connect(){ return false; }

bool MockBDb::connected(){ return false; }

void MockBDb::disconnect(){}

void MockBDb::needAdvisoryLock(int ){}

void MockBDb::exec(const char *){}

void MockBDb::exec(const string &){}

void MockBDb::exec(const ParamQuery &){}

MockBDbResult MockBDb::query(const char *) { return MockBDbResult(data_);}

MockBDbResult MockBDb::query(const string &){ return MockBDbResult(data_);}

MockBDbResult MockBDb::query(const ParamQuery &){ return MockBDbResult(data_);}

bool MockBDb::ping(){ return true; }

bool MockBDb::rel_exists(string ) { return false; }
