
#include "web/view.hpp"
#include "aggregated.hpp"
#include "log/log.hpp"

http_response aggregated::handle(const http_request &request) try {

    // Проверяется, есть ли сам урл в пути или же есть урл-корень
    for (std::string const& path : { request.path_, canonize_url(request.path_)}) {
        auto page_it = views_.find(path);
        if (page_it == views_.end()) {
            continue;
        }
        return page_it->second->handle(request);
    }

    Log::error("Page not found ", request.path_, Log::allow<log_message_level::VERBOSE>,". Query: ", request.body_);
    return not_found("Page not found");
} catch(std::exception& ex) {
    Log::error("Error for query ", request.path_, ". Exception: ", ex.what(), ". Body: ",request.body_ );
    return server_error("Internal server error.");
} catch (...) {
    Log::error("Unknown error for url ", request.path_, ". Body: ", request.body_);
    return server_error("Internal server error.");
}

std::string aggregated::canonize_url(std::string const& url) {
    auto idx = url.find_last_of('/');

    // cut url to path
    return std::string(url.begin(), url.begin() + idx);
}
