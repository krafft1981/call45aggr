#pragma once

#include <cstddef>
#include <ctime>

class BDb;

class BaseObjList {
public:
    virtual void load(BDb* db) = 0;
    virtual std::size_t size() const = 0;
    virtual std::size_t dataSize() const = 0;

    virtual ~BaseObjList(){}

    std::time_t loadTime() const;

protected:
    std::time_t loadtime = 0;
};
