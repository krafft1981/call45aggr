#pragma once

#include <vector>
#include <string>
#include <memory>

using std::shared_ptr;
using std::make_shared;
using std::vector;
using std::string;
using std::initializer_list;

class MockData{

    using raw_data_t = vector<string>;

public:
    MockData(initializer_list<string> values)
            : data_(make_shared<raw_data_t>(values))
    {}

    MockData(vector<string> && data)
            : data_(make_shared<raw_data_t>(std::move(data))){
    }

    MockData(MockData&&) = default;
    MockData(MockData const& )  = default;
    ~MockData() = default;

    MockData& operator=(MockData&&) = default;
    MockData& operator=(MockData const& )  = default;

    string const& at(size_t idx) const { return data_->at(idx);}

    size_t size() const { return data_->size(); }

private:
    shared_ptr<raw_data_t> data_;
};
