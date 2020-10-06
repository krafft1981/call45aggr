#pragma once
#include <memory>
#include <string>
#include <vector>
#include <common/comact_pretty_function.hpp>

namespace Json {
    class Value;
}

namespace trace {
enum class node_type { leaf = 1, node = 2 };

enum class leaf_type {
    INFO = 0x00,
    INPUT_DATA = 0x01,
    RESULT_DATA = 0x02,
    ERROR = 0x03,
    DEBUG = 0x04

};

struct trace_node {

    trace_node(std::string message, compact_pretty_function function, char const *file = "", int line = 0);

    void add_leaf(leaf_type t, std::string message, std::string color);

    trace_node *add_node(std::string name, compact_pretty_function function, char const *file, int line);

    void set_color(std::string color);

    Json::Value convertToJson() const;

  private:
    std::string color_;

    compact_pretty_function function_;

    // От типа зависит, что будет содержать строка
    node_type   type_;
    std::string name_or_message_;
    leaf_type   leaf_node_type_{};

    // Эти поля всегда статичны
    char const *file_;
    int         line_;

    using ptr_t = std::unique_ptr<trace_node>;
    std::vector<ptr_t> children_;
};
} // namespace trace
