#pragma once

#include <map>
#include <vector>
#include <string>
#include "ObjList.h"
#include <data/model/CallLegLink.h>
#include <data/BaseData.hpp>

class CallLegLinkList : public ObjList<CallLegLink> {
protected:
    std::string sql(BDb* db) override;
    void parse_item(BDbResult& row, CallLegLink* item) const override;

public:
    using InputTuple = std::tuple<std::string, std::string>;
    CallLegLinkList(InputTuple const &tup);
    std::string const getLastTime() const;

    std::vector<CallLegLink*> getByUniqueIdOrig(std::string uniqueid) const;

private:
    virtual void after_load() override;

    std::map<std::string, std::vector<CallLegLink*>> origRecords_;

    std::string startTime_;
    std::string stopTime_;
    std::string lastTime_;
};
