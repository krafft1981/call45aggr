#include "data/model/PstnRecord.h"
#include "daemon/settings.hpp"
#include "PstnRecordReport.h"

std::string PstnRecordReport::header() {

    return "insert into () values";
}

void PstnRecordReport::build(CallRecord * record, std::ostream& out) {

}
