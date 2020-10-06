#pragma once

#include <map>
#include <string>
#include "ObjList.h"
#include <data/model/CallLeg.h>
#include <data/BaseData.hpp>

class CallLegList : public ObjList<CallLeg> {
protected:
    std::string sql(BDb* db) override;
    void parse_item(BDbResult& row, CallLeg* item) const override;

public:
    using InputTuple = std::tuple<std::string, std::string>;
    CallLegList(InputTuple const &tup);
    CallLeg* getByUniqueId(std::string uniqueid) const;

private:
    virtual void after_load() override;

    std::map<std::string, CallLeg*> records_;

    std::string startTime_;
    std::string stopTime_;
};
