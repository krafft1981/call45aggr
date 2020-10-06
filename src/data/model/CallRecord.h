#pragma once
#include <map>
#include <ctime>
#include <vector>
#include "CallLeg.h"
#include "CallLegLink.h"
#include "BaseModel.h"

struct CallRecord : public BaseModel {

    std::string orig;
    std::time_t startTime;

    std::map<std::string, CallLeg> legs;
    std::vector<CallLegLink>      links;

    CallLeg                        a;
    std::vector<CallLeg>           b;

    CallRecord(CallLeg * leg, std::time_t stemp) {
        legs.insert(make_pair(leg->uniqueid, *leg));
        orig = leg->uniqueid;
        startTime = stemp;
    }

    void dump (std::ostream& trace) {
        trace << "CallRecord start" << std::endl;

        for(auto leg : legs) {
            trace << "leg: " << leg.second.uniqueid << " " << leg.second.start_time;
            trace << std::endl;
        }

        for(auto link : links) {
            trace << "link: " << link.uniqueid_orig << " -> " << link.uniqueid_term << " " << link.link_start_time;
            trace << std::endl;
        }

        trace << "CallRecord stop with legs: (" << legs.size() << ") links: (" << links.size() << ")" << std::endl;
    };
};
