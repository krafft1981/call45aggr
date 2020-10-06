#include "split.h"
#include <common.hpp>
#include <algorithm>

std::vector<std::string>  split_any(std::string const& src, char const* separators, split_opts opts) {
    std::vector<std::string> result;
    result.reserve(src.size() / 8);

    auto it = src.begin();
    auto end = src.end();

    bool skip_empty = (opts == split_opts::skip_blank);

    auto matches = [](char findable, char const* seps){
        while(*seps != '\0'){
            if (unlikely(findable == *seps)){
                return true;
            }
            ++seps;
        }
        return false;
    };

    while (it != end){

        // this one must be delete every possible empty string

        auto first_it = it;
        auto last_it = it;
        while (last_it != end) {
            if (matches(*last_it, separators)){
                break;
            }
            ++last_it;
        }

        it = (last_it == end ? end : last_it + 1);

        if (first_it == last_it && skip_empty){
            continue;
        }

        result.emplace_back(first_it, last_it);
    }

    return result;
}

template<typename Type>
std::vector<Type> split_to(std::string const& source_str, char const* separators){
    std::vector<Type> result;

    auto src = split_any(source_str, separators, split_opts::skip_blank);
    result.reserve(src.size());

    for (std::string& value : src){
        result.push_back(to<Type>(trim(value)));
    }

    return result;
}

template std::vector<double> split_to<double>(std::string const&, char const*);
template std::vector<int> split_to<int>(std::string const&, char const*);
template std::vector<unsigned> split_to<unsigned>(std::string const&, char const*);

std::string replace_all_subs(std::string const& src, std::string from, std::string to){
    std::string result;
    result.reserve(src.size());

    size_t substr_pos = 0;
    size_t prev_pos = 0;

    auto it = [&](size_t i){ return src.begin() + i;};

    using it_t = decltype(src.begin());
    auto write_result = [&result](it_t begin, it_t end) mutable {
        std::copy(begin, end, std::back_inserter(result));
    };

    while ((substr_pos = src.find(from, prev_pos)) != std::string::npos){
        write_result(it(prev_pos), it(substr_pos));
        write_result(to.begin(), to.end());
        prev_pos = substr_pos + from.size();
    }

    write_result(it(prev_pos), src.end());

    return result;
}


// trim from start
std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                    std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}
