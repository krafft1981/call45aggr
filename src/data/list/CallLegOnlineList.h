#pragma once

#include "ObjList.h"
#include <data/model/CallLegOnline.h>
#include <data/BaseData.hpp>

class CallLegOnlineList : public ObjList<CallLegOnline> {
protected:
    std::string sql(BDb* db) override;
    void parse_item(BDbResult& row, CallLegOnline* item) const override;

private:
    virtual void after_load() override;
    
    std::set<std::string> uniqueid_;

public:
    bool uniqueid_check(std::string uniqueid);
};
