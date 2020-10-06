#include <regex>
#include <json/json.h>
#include "config.hpp"
#include "daemon/settings.hpp"
#include "common/string_streambuf.hpp"
#include "common/split.h"
#include "rendering.hpp"
#include "version.hpp"

void replace_helper(std::vector<std::string> const& path, Json::Value& current, setting_mask const& mask, unsigned idx = 0) {
    if (idx == path.size()) {
        std::regex regex(mask.regexp);
        current = std::regex_replace(current.asString(),regex, mask.replace);
        return;
    }
    replace_helper(path, current[path[idx]], mask, idx + 1);
}

void replace(setting_mask const& mask, Json::Value& root){
    auto path = split_any(mask.path, "/");
    replace_helper(path, root, mask);
}

http_response config_view::handle(http_request const &request) {
    http_response resp;

    resp.mime_type_ = "text/html; charset=utf-8";
    utils::string_streambuf sb(resp.body_);
    std::ostream            os(&sb);

    render_header(os, "Config");

    os << "build date: " << build_datetime() << "<br/>\n";
    os << "built on commit: <b><a href=\"https://github.com/welltime/call45aggr/commit/";
    os << commit_hash() << "\">" << commit_hash() << "</a></b><br/>\n";
    os << "author: " << commit_author_name() << "<br/>\n";
    os << "commit date: " << commit_datetime() << "<br/>\n";
    os << "commit message: " << commit_message() << "<br/><br/>\n";
    os << "<hr><br/>\n";

    auto settings = get_settings();

    auto settings_value = settings.dump();

    // Маскирование важных частей конфига перед выводом
    for (auto& modification : settings.settings_masks()) {
        replace(modification,settings_value);
    }

    Json::StreamWriterBuilder writer;
    writer.settings_["commentStyle"] = "All";
    writer.settings_["indentation"] = "   ";
    std::unique_ptr<Json::StreamWriter> wr(writer.newStreamWriter());

    os << "<pre>";
    wr->write(settings_value,&os);
    os << "</pre>\n";

    return resp;
}
