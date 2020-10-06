#pragma once

#include <string>
#include <list>
#include <vector>

/**
 * This class is used to perform PQexecParams
 */
class ParamQuery {

    std::string query_;
    std::size_t width_ = 0;

    // Параметризованный запрос поддерживавет множество наборов параметров
    std::vector<std::vector<char*>> rows_;

public:

    ParamQuery () = default;
    ParamQuery (ParamQuery&& );
    ParamQuery& operator = (ParamQuery&&) noexcept ;

    /**
     * @param width shouldn't be 0
     */
    ParamQuery (const char* query, std::size_t width);
    /**
     * @param width shouldn't be 0
     */
    ParamQuery (std::string&& query, std::size_t width);

    ~ParamQuery ();

    bool isEmpty() const;
    bool isEmpty (size_t row_idx) const;
    std::size_t getWidth () const;
    std::size_t levelsCount() const;
    const std::string& getQuery () const;

    const char* const* parameters(size_t row_idx = 0) const;

    void endRow();

    ParamQuery& addValue (std::string&& value);
    ParamQuery& addValue (std::string const& value);
    ParamQuery& addValue (const char* value);
    ParamQuery& addNull();

    template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T> || std::is_integral_v<T>>>
    ParamQuery& addValue(T v) {
        return addValue(std::to_string(v));
    }
};
