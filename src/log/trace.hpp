#pragma once

#include "trace_node.hpp"
#include <common.hpp>
#include <systems/ScopeGuard.h>
#include "trace_color.hpp"
#include <common/comact_pretty_function.hpp>

namespace trace {
// Функции времени жизни рутового объекта
void start(std::string message, compact_pretty_function function, char const *file, int line);

void stop();

void add_node(std::string message, compact_pretty_function function, char const *file, int line);

void rem_node(); // from back acctualliy

trace_node *current_trace_node();

trace_node *trace_root();

#define VARIABLE(a, b) a##b
#define VARNAME_PROXY(prefix, ln) VARIABLE(prefix, ln)
#define VARNAME(prefix) VARNAME_PROXY(prefix, __LINE__)

#define START_TRACE(...)                                                       \
    trace::start(concat(__VA_ARGS__), FUNCTION_NAME, __FILE__, __LINE__);      \
    auto VARNAME(stop_trace_) = makeScopeGuard(&trace::stop);

#define ADD_TRACE_NODE(...)                                                    \
    trace::add_node(concat(__VA_ARGS__),  FUNCTION_NAME, __FILE__, __LINE__);  \
    auto VARNAME(drop_trace_node_) = makeScopeGuard(&trace::rem_node);

template <typename... Serializable>
inline void serialize(leaf_type type, Serializable &&... ser) {
    auto node = current_trace_node();
    if (likely(node == nullptr)) {
        return;
    }

    concrete_color col = extract_color(ser...);

    // Нужно как-то перенести trace
    node->add_leaf(type, concat(std::forward<Serializable>(ser)...), color_name(col));
}

template <typename... Serializable>
inline void input(Serializable &&... ser) {
    serialize(leaf_type::INPUT_DATA, std::forward<Serializable>(ser)...);
}

// TODO: Изменить result так, чтобы можно было устанавливать статус сообщения
template <typename... Serializable>
inline void result(Serializable &&... ser) {
    serialize(leaf_type::RESULT_DATA, std::forward<Serializable>(ser)...);
}

template <typename... Serializable>
inline void debug(Serializable &&... ser) {
    serialize(leaf_type::DEBUG, std::forward<Serializable>(ser)...);
}

template <typename... Serializable>
inline void error(Serializable &&... ser) {
    serialize(leaf_type::ERROR, std::forward<Serializable>(ser)..., red());
}

template <typename... Serializable>
inline void info(Serializable &&... ser) {
    serialize(leaf_type::INFO, std::forward<Serializable>(ser)...);
}

} // namespace trace
