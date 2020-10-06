
#include "web/view.hpp"
#include "cyclic_task.hpp"
#include "common/string_streambuf.hpp"
#include "rendering.hpp"
#include "daemon/application.hpp"
#include "params.hpp"

http_response cyclic_task_view::handle(const http_request &request) {

    http_response response;
    response.mime_type_ = "text/html";
    utils::string_streambuf sb(response.body_);
    std::ostream            os(&sb);

    render_header(os, "");

    auto cyclic_task = application::instance().get_cyclic_task(request.parameters_.at("name"));
    if (!cyclic_task) {
        os << "Task not found";
        return response;
    }

    cyclic_task->dump_html(os);


    return response;
}
