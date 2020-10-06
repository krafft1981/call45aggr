#pragma once

#include "ObjList.h"
#include <data/model/CallLegLinkOnline.h>
#include <data/BaseData.hpp>

class CallLegLinkOnlineList : public ObjList<CallLegLinkOnline> {
protected:
    std::string sql(BDb* db) override;
    void parse_item(BDbResult& row, CallLegLinkOnline* item) const override;

private:
    virtual void after_load() override;
    virtual void before_load() override;

    std::set<std::string> uniqueid_;
    std::map<std::string, CallLegLinkOnline> uniqueidOrig_;
    std::map<std::string, CallLegLinkOnline> uniqueidTerm_;

public:
    bool uniqueid_check(std::string uniqueid);
    CallLegLinkOnline* get_orig_term(std::string uniqueid_orig, std::string uniqueid_term);
};
