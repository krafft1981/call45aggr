#pragma once

namespace trace {
    enum class concrete_color {
        none = 0,
        red = 1,
        green = 2,
        blue = 3
    };

    inline std::ostream& operator<<(std::ostream& os, concrete_color){
        return os;
    }


    template<concrete_color Col>
    struct trace_color {
    };

    inline char const *color_name(concrete_color col) {
        if (col == concrete_color::red) {
            return "red";
        }

        if (col == concrete_color::green) {
            return "green";
        }

        if (col == concrete_color::blue) {
            return "blue";
        }

        return "";
    }


    template<typename T>
    struct color_extractor {
        static concrete_color color() {
            return concrete_color::none;
        }
    };

    template<concrete_color cols>
    struct color_extractor<trace_color<cols>> {
        static concrete_color color() {
            return cols;
        }
    };

    template<typename T>
    inline concrete_color extract_single_color() {
        return color_extractor<T>::color();
    }


    template<typename ... Arguments>
    inline concrete_color extract_color(Arguments &&... args) {
        concrete_color cols[] = { (extract_single_color<std::decay_t < Arguments>>(), ...)};
        for (auto col : cols) {
            if (col != concrete_color::none) {
                return col;
            }
        }
        return concrete_color::none;
    }

    template<concrete_color N>
    inline std::ostream &operator<<(std::ostream &os, trace_color<N> const &) {
        return os;
    }

    inline auto red() { return trace_color<concrete_color::red>(); }
    inline auto green() { return trace_color<concrete_color::green>(); }
    inline auto blue() { return trace_color<concrete_color::blue>(); }
    inline auto none() { return trace_color<concrete_color::none>(); }
}
