#pragma once

#include <functional>

// Определяет глобальные события, когда должны выполняться функции
enum class life_span_event {
    START    = 1, // До запуска приложения
    INIT_APP = 2, // Сразу после запуска приложения
    INIT_DB  = 3, // После начальной инициализации данных
    STOP     = 4, // Перед полной остановкой приложения
};

std::ostream& operator << (std::ostream& os, life_span_event e);

struct lf_routine {
    std::string           name;
    std::function<void()> routine;
};

void register_life_span_event(life_span_event event, int priority, lf_routine&&  routine) noexcept;
void apply_life_span_events(life_span_event event);

// use
// static life_span ls(life_span_event::START, 10, []() { StaticObject::instance()->start(); });
struct life_span {
    life_span(std::string const& name,life_span_event event, int priority, std::function<void()> routine) noexcept {
        register_life_span_event(event, priority, {name, routine});
    }

    // Вызывается сразу после запуска объекта приложения, и перед полным завершением программы
    life_span(int priority, std::string const& name, std::function<void()> start_routine, std::function<void()> stop_routine){
        register_life_span_event(life_span_event::INIT_APP, priority, {name, std::move(start_routine)});
        register_life_span_event(life_span_event::STOP, -priority, {name, std::move(stop_routine)});
    }
};

#define LS_VARIABLE(a, b) a##b
#define LS_VARNAME_PROXY(prefix, ln) LS_VARIABLE(prefix, ln)
#define LS_VARNAME(prefix) LS_VARNAME_PROXY(prefix, __LINE__)

#define ON_START_P(name, priority, fun)                                    \
static life_span LS_VARNAME(ls)(name, life_span_event::START, priority, fun);
#define ON_INIT_APP_P(name, priority, fun)                                 \
static life_span LS_VARNAME(ls)(name, life_span_event::INIT_APP, priority, fun);
#define ON_INIT_DB_P(name, priority, fun)                                  \
static life_span LS_VARNAME(ls)(name, life_span_event::INIT_DB, priority, fun);
#define ON_STOP_P(name, priority, fun)                                     \
static life_span LS_VARNAME(ls)(name, life_span_event::STOP, priority, fun);

// Регистрирует функцию с нулевым приритетом
#define ON_START(name, fun) ON_START_P(name, 0, fun)
#define ON_INIT_APP(name, fun) ON_INIT_APP_P(name, 0, fun)
#define ON_INIT_DB(name, fun) ON_INIT_DB_P(name, 0, fun)
#define ON_STOP(name, fun) ON_STOP_P(name, 0, fun)
