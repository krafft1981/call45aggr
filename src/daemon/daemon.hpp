#pragma once

#include <string>
#include <functional>
#include "common/reusable_object.hpp"
#include "service/MergeService.h"

class BDb;

// Функции для контроля времени жизни различных частей системы
// Функции для доступа к статическим объектам системы

namespace app /*daemon life span*/{
    int run();
    void stop(); // stop everything

    // Задачи, выполняемые по таймеру
    void register_cyclic_task(std::function<void()> routine, char const* name, time_t timeout_in_ms, std::function<void(std::ostream&)> dump = {});
    void delete_cyclic_task(char const* name);
    void run_async_task(std::function<void()> async_routine);

    MergeService* getMergeService();

    reusable_object<BDb> source_conn();
    reusable_object<BDb> output_conn();
    reusable_object<BDb> online_conn(int poolId);

    int online_pool_count();

    time_t uptime();

    // Механизм, отвечающий на время жизни всего приложения
    // Возможно в будущем стоит отправить сигнал завершения
    bool running();
}
