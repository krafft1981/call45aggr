#pragma once

#include "BaseReport.h"

class DataVoipRecordReport : public BaseReport {
public:
    std::string header();
    void build(CallRecord * record, std::ostream& out);
};
