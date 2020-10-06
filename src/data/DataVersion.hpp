#pragma once
#include <stddef.h>

// Небольшая оптимизация, чтобы не дергать множество раз репозиторий
// Вполне логично перенести эти параметры в демон
size_t data_version();
size_t fetch_data_version();
