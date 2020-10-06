#pragma once

// Здесь буквально должно быть немного зависимостей, т.к. неизвестно
// в каких местах будет использоваться этот код.

#include <boost/utility/string_view.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/verb.hpp>
#include <map>
#include <vector>

struct http_request {
    struct field {
        boost::beast::http::field name_;
        boost::string_view value_;
    };

    std::vector<field>       fields_;
    std::string              path_;
    boost::string_view       body_;
    boost::beast::http::verb method_ ;
    std::map<std::string, std::string> parameters_;
};

struct http_response {
    struct field {
        std::string name_;
        std::string value_;
    };

    // Дополнительные необязательные поля
    std::vector<field> fields_;
    std::string        body_;
    std::string        mime_type_;
    int                code_ = 200;
};

// Объект контроллера подключается к слушателю порта.
// Один контроллер на порт.
struct view {
    // Функция вызывается в множестве потоков для обработки запросов
    virtual http_response handle(http_request const& request) = 0;

    virtual ~view(){}
};

http_response error_response(int code, char const* reason);
http_response bad_request(char const* reason);
http_response not_found(char const* reason);
http_response server_error(char const* reason);

// TODO: Функции помощники для разбора хедера