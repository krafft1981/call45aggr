#pragma once

namespace meta {
    template<bool isPointer>
    struct accessor;

    template<>
    struct accessor<true> {
        template<typename Type, typename Field>
        static auto &access_field(Type t, Field f) {
            return t->*f;
        }

        template<typename Type>
        static auto access_value(Type&& t) {
            return *t;
        }
    };

    template<>
    struct accessor<false> {
        template<typename Type, typename Field>
        static auto &access_field(Type &&t, Field f) {
            return t.*f;
        }

        template<typename Type>
        static auto access_value(Type&& t) {
            return t;
        }
    };

    template<typename T, typename Field>
    inline auto access_field(T &&v, Field &&f) {
        using clear_type = std::decay_t<T>;
        using accessor = accessor<std::is_pointer_v<clear_type>>;
        return accessor::access_field(v, f);
    }


    template<typename T>
    inline auto access_value(T &&v) {
        using clear_type = std::decay_t<T>;
        using accessor = accessor<std::is_pointer_v<clear_type>>;
        return accessor::access_value(v);
    }
}
