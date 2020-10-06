#pragma once

template<typename SourceType>
struct Wrapper {

    template<typename ... Values>
    Wrapper(Values && ... values)
            : data_{std::forward<Values>(values)...}
    {}

    SourceType const& data() const {
        return data_;
    }

private:
    const SourceType data_;
};

using Event = Wrapper<std::string>;
using SrcTable = Wrapper<std::string>;
using DstTable = Wrapper<std::string>;
using Key = Wrapper<std::string>;
using SrcSqlWhere = Wrapper<std::string>;
using Fields = Wrapper<std::vector<std::string>>;
