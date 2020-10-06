#pragma once

#include <string>
#include <sstream>
#include "data/model/CallRecord.h"

class BaseReport {
public:
    virtual std::string header() = 0;
    virtual void build(CallRecord * record, std::ostream& out) = 0;
};
