#include <map>
#include "life_span.hpp"
#include "log/log.hpp"
#include "health_register.hpp"

struct life_span_storage {
    using tasks_by_priority_t  = std::multimap<int, lf_routine>;
    using events_map_t = std::map<life_span_event, tasks_by_priority_t >;

    events_map_t events_;

    void add_event(life_span_event event, int priority, lf_routine&& routine) noexcept {
        events_[event].insert(tasks_by_priority_t::value_type(priority, std::move(routine)));
    }

    template<typename ... Args>
    void register_error(std::string const& name, Args&& ... args){
        auto message = concat(std::forward<Args>(args)...);
        health_register::error(name, message);
        Log::error(message);
    }

    void apply_event(life_span_event event) noexcept {
        for (auto& p : events_[event]) {
            try {
                p.second.routine();
            } catch(std::exception& ex) {
                register_error(p.second.name, "life_span_event::", event,". Error: ", ex.what());
            } catch (...){
                register_error(p.second.name, "life_span_event::", event,". Unknown exception");
            }
        }
    }
};

static life_span_storage& instance() {
    static life_span_storage stor;
    return stor;
}

void register_life_span_event(life_span_event event, int priority, lf_routine&&  routine) noexcept {
    instance().add_event(event, priority, std::move(routine));
}

void apply_life_span_events(life_span_event event) {
    instance().apply_event(event);
}

std::ostream &operator<<(std::ostream &os, life_span_event e) {
    switch (e) {
        case life_span_event::START:    os << "START";    return os;
        case life_span_event::INIT_APP: os << "INIT_APP"; return os;
        case life_span_event::INIT_DB:  os << "INIT_DB";  return os;
        case life_span_event::STOP:     os << "STOP";     return os;
    }
    return os;
}
