#pragma once

#include <cstddef>
#include <ctime>
#include <vector>

class BDb;

class BaseObjCounter {
public:
    //virtual void load(BDb* db, std::vector<long long int> const& ids) = 0;
    virtual std::size_t size() const = 0;
    virtual std::size_t dataSize() const = 0;

    virtual ~BaseObjCounter(){}

    std::time_t loadTime() const;

protected:
    std::time_t loadtime = 0;
    size_t data_version_ = 0;
};
