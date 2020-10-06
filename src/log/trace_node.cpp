
#include "trace_node.hpp"
#include <common/split.h>
#include <json/json.h>
#include <string>
#include <common/concat.hpp>

namespace trace {
trace_node::trace_node(std::string message, compact_pretty_function function = compact_pretty_function(), char const *file, int line)
    : type_(node_type::node), function_(function), name_or_message_(message), file_(file),
      line_(line) {}

Json::Value trace_node::convertToJson() const {

    Json::Value result;
    if (type_ == node_type::node){
        result["file"] = file_;
        result["line"] = line_;
        result["name"] = name_or_message_;
        result["color"] = color_;
        if (function_) {
            result["function"] = concat(function_);
        }
        int counter = 0;
        for (auto& child : children_){
            result["nodes"][counter++] = child->convertToJson();
        }
    } else {
        //TODO: Добавить leaf_node_type
        result["message"] = name_or_message_;
        result["color"] = color_;
        result["type"] = [](leaf_type t) {
            switch (t) {
                case leaf_type::INFO:        return "INFO";
                case leaf_type::INPUT_DATA:  return "INPUT";
                case leaf_type::RESULT_DATA: return "RESULT";
                case leaf_type::ERROR: return "ERROR";
                case leaf_type::DEBUG: return "DEBUG";
            }
        }(leaf_node_type_);
    }


    return result;
}

void trace_node::add_leaf(leaf_type leaf_type, std::string multimessage,
                          std::string color) {
    for (std::string const &message : split_any(multimessage, "\n")) {
        children_.push_back(std::make_unique<trace_node>(message));
        children_.back()->type_ = node_type::leaf;
        children_.back()->leaf_node_type_ = leaf_type;
        children_.back()->set_color(color);
    }
}

trace_node *trace_node::add_node(std::string name, compact_pretty_function function, char const *file, int line) {
    children_.push_back(std::make_unique<trace_node>(name, function, file, line));
    children_.back()->type_ = node_type::node;
    return children_.back().get();
}

void trace_node::set_color(std::string color) { color_ = color; }
} // namespace trace
