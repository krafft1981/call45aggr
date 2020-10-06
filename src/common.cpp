#include <iostream>
#include "common.hpp"

#define UNIXTIME_50010101 95649120000
#ifdef OLD_LOG
#include <log/old_log.hpp>
#else
#include "log/log.hpp"
#endif

// for base64 decoding
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/remove_whitespace.hpp>

#include <algorithm>

template<typename ResultType, typename FunctionResultType>
ResultType to_number(FunctionResultType (cast)(char const* ,char**, int), char const* str){
    char* end;
    ResultType result = numeric_cast<ResultType>(cast(str, &end, 10));
    if (*end != '\0'){
        throw std::runtime_error(concat("[", __PRETTY_FUNCTION__, "] Cannot convert string \"", str, "\" "));
    }
    return result;
}

template<>
int to<int>(char const* str){
    return  to_number<int>(std::strtol, str);
}

template<>
unsigned to<unsigned>(char const* str){
    return  to_number<unsigned>(std::strtoul, str);
}

template<>
unsigned short to<unsigned short>(char const* str){
    return  to_number<unsigned short>(std::strtoul, str);
}


template<>
long to<long>(char const* str) {
    return to_number<long>(std::strtol, str);
}

template<>
long long to<long long>(char const* str) {
    return to_number<long long>(std::strtoll, str);
}

template<>
unsigned long to<unsigned long>(char const* str) {
    return to_number<unsigned long>(std::strtoul, str);
}

template<>
unsigned long long to<unsigned long long>(char const* str) {
    return to_number<unsigned long long>(std::strtoull, str);
}

template<>
double to<double>(char const* str) {
    return to_number<double, double>([](char const* str, char** result, int )   {
        return std::strtod(str, result);
    }, str);
}

std::string escape_string(std::string str) {

    std::string out;
    for (auto it : str) {
        switch(it) {
            case 0x27: break;
            case 0x22: break;
            case 0x3f: break;
            case 0x5c: break;
            default:
                out.append(1, it);
        }
    }

    return out;
}

std::string string_time(const time_t dt) {
    char buff[64] = {};

    struct tm timeinfo;
    gmtime_r(&dt, &timeinfo);
    if (timeinfo.tm_year < 0 || timeinfo.tm_year > 3000) {
        timeinfo.tm_year = 0;
        timeinfo.tm_mon = 0;
        timeinfo.tm_mday = 1;
        timeinfo.tm_hour = 0;
        timeinfo.tm_min = 0;
        timeinfo.tm_sec = 0;
    }
    strftime(buff, 40, "%Y-%m-%d %H:%M:%S", &timeinfo);
    return std::string(buff);
}

static std::string string_time( char const* format, time_t time, struct tm *(time_r) (const time_t * __timer,
                                                                                      struct tm * __tp) ){
    struct tm timeinfo;
    time_r(&time, &timeinfo);

    char buffer[256];
    strftime(buffer, sizeof(buffer), format, &timeinfo);

    return std::string(buffer);
}

std::string string_gm_time(char const* format, time_t time) {
    return string_time(format, time, &gmtime_r);
}

std::string string_local_time(char const* format, time_t time) {
    return string_time(format, time, &localtime_r);
}

template<typename T>
std::vector<T> extractIntegralValues (const std::string& str) {
    static_assert(std::is_integral<T>::value, "Only integral types"); // Работаем только с целочисленными

    std::vector<T> container;
    std::string number;
    number.reserve(32);
    for (const auto& character : str) {
        if (std::isdigit(character) || character == '-')
            number.push_back(character);
        else {
            if (!number.empty()) {
                try {
                    container.push_back(to<T>(number));
                }
                catch (...) {}
                number.clear ();
            }
        }
    }
    if (!number.empty()) {
        try {
            container.push_back(to<T>(number));
        }
        catch (...) {}
    }
    return container;
}

template<typename T>
std::set<T> extractIntegralValuesToSet (const std::string& str) {
    static_assert(std::is_integral<T>::value, "Only integral types"); // Работаем только с целочисленными

    std::set<T> container;
    std::string number;
    number.reserve(32);
    for (const auto& character : str) {
        if (std::isdigit(character) || character == '-')
            number.push_back(character);
        else {
            if (!number.empty()) {
                try {
                    container.insert(to<T>(number));
                }
                catch (...) {}
                number.clear ();
            }
        }
    }
    if (!number.empty()) {
        try {
            container.insert(to<T>(number));
        }
        catch (...) {}
    }
    return container;
}

template std::vector<int> extractIntegralValues<int>(const std::string& str);
template std::vector<PhoneNumber> extractIntegralValues<PhoneNumber>(const std::string& str);
template std::vector<unsigned short> extractIntegralValues<unsigned short>(const std::string& str);

template std::set<int> extractIntegralValuesToSet<int>(const std::string& str);


time_t parseDateTime(const char *str) {
    struct tm ttt;
    if (sscanf(str, "%d-%d-%d %d:%d:%d",
               &ttt.tm_year, &ttt.tm_mon, &ttt.tm_mday,
               &ttt.tm_hour, &ttt.tm_min, &ttt.tm_sec) > 0
            ) {
        ttt.tm_year -= 1900;
        ttt.tm_mon -= 1;
        ttt.tm_isdst = 0;
        ttt.tm_yday = 0;
        return timegm(&ttt);
    } else {
        return 0;
    }}

std::string string_date(const time_t dt, int debugFlag) {
    char buff[20];

    if (dt > UNIXTIME_50010101) {
        Log::error("string_date: bad unix time ", debugFlag);
        return std::string("5000-01-01");
    }

    struct tm timeinfo;
    gmtime_r(&dt, &timeinfo);
    if (timeinfo.tm_year < 0 || timeinfo.tm_year > 3000) {
        timeinfo.tm_year = 0;
        timeinfo.tm_mon = 0;
        timeinfo.tm_mday = 1;
    }
    strftime(buff, 20, "%Y-%m-%d", &timeinfo);
    return std::string(buff);
}


time_t get_day_ts(time_t timestamp, short timezone_offset) {
    struct tm timeinfo;
    timestamp += 3600 * timezone_offset;
    gmtime_r(&timestamp, &timeinfo);
    timeinfo.tm_isdst = 0;
    timeinfo.tm_wday = 0;
    timeinfo.tm_yday = 0;
    timeinfo.tm_hour = 0;
    timeinfo.tm_min = 0;
    timeinfo.tm_sec = 0;
    return timegm(&timeinfo) - 3600 * timezone_offset;
}

time_t get_day_end_ts(time_t timestamp, short timezone_offset) {
    return get_day_ts(timestamp, timezone_offset) + 86399;
}

time_t get_month_ts(time_t timestamp, short timezone_offset) {
    struct tm timeinfo;
    timestamp += 3600 * timezone_offset;
    gmtime_r(&timestamp, &timeinfo);
    timeinfo.tm_isdst = 0;
    timeinfo.tm_wday = 0;
    timeinfo.tm_yday = 0;
    timeinfo.tm_hour = 0;
    timeinfo.tm_min = 0;
    timeinfo.tm_sec = 0;

    timeinfo.tm_mday = 1;

    return timegm(&timeinfo) - 3600 * timezone_offset;
}

time_t get_month_end_ts(time_t timestamp, short timezone_offset) {
    struct tm timeinfo;
    timestamp += 3600 * timezone_offset;
    gmtime_r(&timestamp, &timeinfo);
    timeinfo.tm_isdst = 0;
    timeinfo.tm_wday = 0;
    timeinfo.tm_yday = 0;
    timeinfo.tm_hour = 0;
    timeinfo.tm_min = 0;
    timeinfo.tm_sec = 0;

    timeinfo.tm_mday = 1;
    if (++timeinfo.tm_mon == 12) {
        timeinfo.tm_mon = 0;
        timeinfo.tm_year++;
    }

    return timegm(&timeinfo) - 1 - 3600 * timezone_offset;
}

std::string decode_base64(std::string input) {

    /*
     * COPY PASTE https://stackoverflow.com/a/46358091
     * Очень неэффективный способ. Нужно копирование строки
     */

    using namespace boost::archive::iterators;
    typedef transform_width<binary_from_base64<remove_whitespace
         <std::string::const_iterator> >, 8, 6> ItBinaryT;

    // If the input isn't a multiple of 4, pad with =
    size_t num_pad_chars((4 - input.size() % 4) % 4);
    input.append(num_pad_chars, '=');

    size_t pad_chars(std::count(input.begin(), input.end(), '='));
    std::replace(input.begin(), input.end(), '=', 'A');

    std::string output(ItBinaryT(input.begin()), ItBinaryT(input.end()));
    output.erase(output.end() - pad_chars, output.end());

    return output;
}

std::string encode_base64(std::string const& in) {
    using uchar = unsigned char;

    std::string out;

    int val=0, valb=-6;
    for (uchar c : in) {
        val = (val<<8) + c;
        valb += 8;
        while (valb>=0) {
            out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val>>valb)&0x3F]);
            valb-=6;
        }
    }
    if (valb>-6) out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val<<8)>>(valb+8))&0x3F]);
    while (out.size()%4) out.push_back('=');
    return out;
}


