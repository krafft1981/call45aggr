#pragma once

#include <ostream>


// Можно было сделать парсер выражений, как в catch2, и убить
// на это кучу времени конечно, поэтому выражения будут чуть проще
template<typename Lhs, typename Rhs>
struct simple_expression {
    Lhs const& lhs_;
    Rhs const& rhs_;
    char const* expression_;
    char const* operation_;
    bool result_;

    bool filtered_ = false;
    bool not_filtered_ = false;

    simple_expression(Lhs const& lhs, Rhs const& rhs, bool result, char const* expression, char const* operation)
            : lhs_(lhs),
              rhs_(rhs),
              expression_(expression),
              operation_(operation),
              result_(result)
    {}

    simple_expression& filtered() {
        filtered_ = true;
        return *this;
    }

    simple_expression& not_filtered() {
        not_filtered_ = true;
        return *this;
    }

    explicit operator bool() const {
        return result_;
    }

    friend std::ostream& operator << (std::ostream& os, simple_expression const& expr) {
        if (expr.filtered_) {
            os << "FILTERED BY ";
        } else if (expr.not_filtered_){
            os << "NOT FILTERED BY ";
        }
        os << "(" << expr.expression_ << ") ";
        os << (expr.result_ ? "" : "!") << "(" << expr.lhs_ << " " << expr.operation_ << " " <<  expr.rhs_ << ")";

        return os;
    }
};


// IDE не может в c++17
template<typename Lhs, typename Rhs>
simple_expression<Lhs, Rhs> make_simple_expression(Lhs const& lhs, Rhs const& rhs, bool result, char const* expression, char const* operation) {
    return simple_expression<Lhs, Rhs>(lhs, rhs, result, expression, operation);
};


template<typename ValueType>
struct named_value {
    ValueType const& ref_;
    char const* name_;

    named_value(ValueType const& value, char const* name)
            : ref_(value),
            name_(name)
    {}

    friend std::ostream& operator << (std::ostream& os, named_value const& val){
        os << val.name_ << " = ";

        // pointer workaround
        if constexpr (std::is_pointer_v<std::decay_t<ValueType>>){
            if (val.ref_ == nullptr) {
                os << "nullptr";
            } else {
                os << *val.ref_;
            }

        } else {
            os << val.ref_;
        }

        return os;
    }

};

template<typename ValueType>
named_value<ValueType> make_named_value(ValueType const& v, char const* name) {
    return named_value<ValueType>(v,name);
}

#define EXPR(lhs, op, rhs) \
    make_simple_expression(lhs, rhs, lhs op rhs, #lhs " " #op " " #rhs, #op)

#define VAL(v) make_named_value(v, #v)