#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include "web/view.hpp"

struct aggregated : public view {
    virtual http_response handle(http_request const& request) override;

    template<typename ConcreteView, typename ... Arguments>
    void add_api_root(boost::string_view url, Arguments&& ... args) {
        static_assert(std::is_base_of_v<view, ConcreteView>, "Class must be derived from view");
        views_[canonize_url(url.to_string())] = std::make_shared<ConcreteView>(std::forward<Arguments>(args)...);
    }

    template<typename ConcreteView, typename ... Arguments>
    void add(boost::string_view url, Arguments &&... args) {
        static_assert(std::is_base_of_v<view, ConcreteView>, "Class must be derived from view");
        views_[url.to_string()] = std::make_shared<ConcreteView>(std::forward<Arguments>(args)...);
    }

    // \brief использовать один обработчик на несколько url'oв
    // \param dst_url - урл назначения
    // \param src_url - использовать обработчик по этому урлу
    void add_existed(boost::string_view dst_url, boost::string_view src_url){
        views_[dst_url.to_string()] = views_.at(src_url.to_string());
    }

private:

    std::string canonize_url(std::string const& url);

    // canonized_url to view
    std::unordered_map<std::string, std::shared_ptr<view>> views_;
};