#pragma once
#include <ostream>

struct BaseModel {
    virtual void dump (std::ostream& trace) = 0;
};
