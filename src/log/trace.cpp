#include "trace.hpp"
#include <assert.h>
#include <memory>
#include <vector>

namespace trace {
namespace {
struct instance {
    std::vector<trace_node *>   stack_;
    std::unique_ptr<trace_node> root_;
};
} // namespace

static instance &tl() {
    thread_local instance t;
    return t;
}

trace_node *trace_root() { return tl().root_.get(); }

trace_node *current_trace_node() {
    instance &cur = tl();
    if (cur.root_) {
        return cur.stack_.back();
    }
    return nullptr;
}

void start(std::string message, compact_pretty_function function, char const *file, int line) {
    instance &cur = tl();
    if (cur.root_) {
        throw std::runtime_error("[trace] Root allready set");
    }
    cur.root_ = std::make_unique<trace_node>(message, function, file, line);
    cur.stack_ = {cur.root_.get()};
}
void stop() {
    instance &cur = tl();
    cur.root_.reset();
    cur.stack_.clear();
}

void add_node(std::string message, compact_pretty_function function, char const *file, int line) {
    instance &cur = tl();
    if (!cur.root_) {
        return;
    }
    auto *new_node = cur.stack_.back()->add_node(message, function, file, line);
    cur.stack_.push_back(new_node);
}

void rem_node() {
    instance &cur = tl();
    cur.stack_.pop_back();
    // В этот момент времени стэк должен срабатывать правильно
    assert(!cur.stack_.empty());
}

} // namespace trace
