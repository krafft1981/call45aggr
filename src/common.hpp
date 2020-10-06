#pragma once

#include <string>
#include <set>
#include <vector>
#include "common/numeric.hpp"

template<typename T>
T to(char const* str);

template<typename T>
inline T to(std::string const& str) {
    return to<T>(str.c_str());
}

template<typename T>
inline T safe_to(std::string const& src) {
    try {
        return to<T>(src);
    } catch(...) {
        return T{};
    }
}

std::string escape_string(std::string str);
std::string string_date(const time_t dt, int debugFlag = -1);

std::string string_time(const time_t dt) ;

std::string string_gm_time(char const* format, time_t time);
std::string string_local_time(char const* format, time_t time);

time_t parseDateTime(const char * str);

// определяется метка времени конца/начала дня/месяца
time_t get_day_ts(time_t timestamp, short timezone_offset = 0);
time_t get_day_end_ts(time_t timestamp, short timezone_offset = 0);

time_t get_month_ts(time_t timestamp, short timezone_offset = 0);
time_t get_month_end_ts(time_t timestamp, short timezone_offset = 0);


// Функция записывает в строку, скорее всего, бинарные данные
std::string decode_base64(std::string src);
std::string encode_base64(std::string const& src);


/**
 * Возвращает вектор целых чисел, содержащихся в строке.
 * Числа не должны быть разделены минусом(-), тк минус
 * используется для отрицательных чисел.
 *
 * @param str строка с числами
 *
 * @return вектор с числами
 */
template<typename T>
std::vector<T> extractIntegralValues (const std::string& str) ;

template<typename T>
std::set<T> extractIntegralValuesToSet (const std::string& str) ;

#if defined(__GNUC__) || defined(__ICL) || defined(__clang__)
#define likely(x)       __builtin_expect(static_cast<bool>(x),1)
#define unlikely(x)     __builtin_expect(static_cast<bool>(x),0)
#else
#define likely(x)       x
#define unlikely(x)     x
#endif

#include "types.hpp"
