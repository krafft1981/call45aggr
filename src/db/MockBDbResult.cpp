#include "MockBDbResult.h"
#include <common.hpp>
#include <boost/algorithm/string.hpp>
#include <common/split.cpp>

MockBDbResult::MockBDbResult(MockData data)
        : data_(data),
        index_(-1)
{}

size_t MockBDbResult::size() {  return data_.size();}

size_t MockBDbResult::position() {
    return static_cast<size_t>(index_);
}

char const* MockBDbResult::get(int , int ) {
    assert(false);
    return "";
}

bool MockBDbResult::is_null(int f) {
    return data_.at(f) == "NULL";
}

char const* MockBDbResult::get(int f) {
    return current_state_.at(f).c_str();
}

int MockBDbResult::get_i(int f) {
    return atoi(get(f));
}

double MockBDbResult::get_d(int f) {
    return atof(get(f));
}

long long int MockBDbResult::get_ll(int f) {
    return atoll(get(f));
}

bool MockBDbResult::get_b(int f) {
    return strcmp(get(f),"t") == 0;
}

string MockBDbResult::get_s(int f) {
    return current_state_.at(f);
}

void MockBDbResult::fill_cs(int f, char * str, size_t size) {
    strncpy(str, get(f), size - 1);
}

void MockBDbResult::parse_current_state(){
    std::string const& src = data_.at(index_);
    current_state_ =  split_any( src, "|");
    for (std::string& s : current_state_){
        boost::trim(s);
    }
}

bool MockBDbResult::next() {
    if (index_ == static_cast<int64_t>(data_.size()) - 1){
        return false;
    }
    ++index_;
    parse_current_state();
    return true;
}

bool MockBDbResult::previous() {
    if (index_ == 0){
        return false;
    }
    --index_;
    parse_current_state();
    return true;
}

bool MockBDbResult::first() {
    if (data_.size() == 0){
        return false;
    }
    index_ = 0;
    parse_current_state();
    return true;
}

bool MockBDbResult::last() {
    if (data_.size() == 0){
        return false;
    }
    index_ = data_.size() -1;
    parse_current_state();
    return false;
}

void MockBDbResult::force_free(){}
