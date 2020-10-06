#include <boost/algorithm/string/replace.hpp>
#include "web/view.hpp"
#include "log.hpp"
#include "rendering.hpp"
#include "common/string_streambuf.hpp"
#include "log/log_instance.hpp"
#include "common.hpp"

http_response log_view::handle(const http_request &request) {
    http_response response;

    response.mime_type_ = "text/html";
    utils::string_streambuf sb(response.body_);
    std::ostream            os(&sb);

    render_header(os,"Log");

    std::string text;
    utils::string_streambuf sb2(text);
    std::ostream            os2(&sb2);

    for (auto& mes  : last_messages()) {
        os2 << "[" << string_time(mes->timestamp) << "] [" << mes->thread_name << "] [" << mes->level << "]    " << mes->message << "\n";
    }

    boost::replace_all(text, "<", "&lt");
    boost::replace_all(text, ">", "&gt");

    os << "<pre>\n";
    os << text;
    os << "</pre>";
    //os <<  "<pre>Not implemented</pre>";

    return response;
}
