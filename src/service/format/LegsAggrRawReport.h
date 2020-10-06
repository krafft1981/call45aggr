#pragma once

#include <sstream>
#include "BaseReport.h"
#include "data/model/CallRecord.h"

class LegsAggrRawReport : public BaseReport {
public:
    LegsAggrRawReport();
    std::string header();
    void build (CallRecord* record, std::ostream& out);

private:
    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> newCharReader;
};
