#pragma once

#include <time.h>
#include <vector>
#include "systems/ScopeGuard.h"
#include "common/array_view.hpp"

namespace repo {
void init_data();

// Необходимо каждый раз вызывать эту функцию перед использованием репозитория
bool prepare(time_t t = time(nullptr));
void clear();

} // namespace repo

// Определять перед repo::prepare для очистки в конце использования
// Нужен для предотвращения раздутия данных до неадекватных размеров
// Репозиторий живет в thread_local. При каждом вызове prepare
// в него сохраняется ссылка на текущий слепок данных.
// При вызове prepare в нескольких разных потоках один поток может удерживать
// устаревшие данные, и не будет обновляться до следующего вызова prepare,
// что может не происходить достаточно продолжительное время. Вследствие этого
// любой поток, который может быть по разным причинам незадействован, будет
// хранить огромный массив старых данных.
#define REPOSITORY_CLEANER          \
    ScopeGuard repositoryScopeGuard([](){ repo::clear(); });
