#pragma once

#include "log_message.hpp"
#include <common/concat.hpp>
#include <common/osstream.hpp>
#include "log_delay.hpp"
#include "log_level.hpp"
#include <common/meta/for_each_argument.hpp>
#include "parameters.hpp"

namespace Log {

// Для случая когда нужно отправлять сложные сообщения с json
void send(log_message &&mes);

// Останавливает рабготу потока на определенном уровне логирования
// Нужно для случаев, когда надо выводить основную часть сообщения
// на одном уровне логирования, а некую дополнительную информацию на другом
template <log_message_level  level>
inline std::ostream& allow(std::ostream& os){
    if (level > log_level()){
        os.setstate(std::ios::failbit);
    }
    return os;
}

// Разбор конкретного аргумента, что приходит в логи
// Некоторые типы данных обрабатываются специальным образом
template<typename ArgumentType>
inline void place_argument(std::ostream& os, log_message& mes, ArgumentType&& arg) {
    using Type = std::decay_t<ArgumentType>;
    if constexpr (std::is_same_v<Type, parameters>) {
        arg.dump(mes.json);
    } else if constexpr (std::is_same_v<Type, log_delay_impl>){
        mes.delay_in_seconds_ = arg.delay_;
    } else if constexpr (std::is_same_v<Type, log_message_destination>) {
        mes.destination = arg;
    } else {
        os << std::forward<ArgumentType>(arg);
    }
}

template <typename... Args>
inline void log_impl(log_message_level level, Args &&... args) {

    if (level > log_level()){
        return;
    }

    log_message mes;
    mes.timestamp = time(nullptr);
    mes.level = level;
    osstream os(mes.message);

    for_each_argument([&](auto&& arg) {
        place_argument(os, mes, std::forward<decltype(arg)>(arg));
        }, std::forward<Args>(args)...);

    send(std::move(mes));
}

template <typename... Args>
inline void debug(Args &&... args) {
    log_impl(log_message_level::DEBUG, std::forward<Args>(args)...);
}

template <typename... Args>
inline void verbose(Args &&... args) {
    log_impl(log_message_level::VERBOSE, std::forward<Args>(args)...);
}

template <typename... Args>
inline void info(Args &&... args) {
    log_impl(log_message_level::INFO, std::forward<Args>(args)...);
}

template <typename... Args>
inline void notice(Args &&... args) {
    log_impl(log_message_level::NOTICE, std::forward<Args>(args)...);
}

template <typename... Args>
inline void warning(Args &&... args) {
    log_impl(log_message_level::WARNING, std::forward<Args>(args)...);
}

template <typename... Args>
inline void error(Args &&... args) {
    log_impl(log_message_level::ERROR, std::forward<Args>(args)...);
}

template <typename... Args>
inline void critical(Args &&... args) {
    log_impl(log_message_level::CRITICAL, std::forward<Args>(args)...);
}

} // namespace log
