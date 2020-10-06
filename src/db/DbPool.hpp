#pragma once

#include "BDb.h"
#include <memory>
#include <string>
#include <map>
#include <mutex>
#include "BDb.h"
#include "common/reusable_object.hpp"

class DbPool {

public:
    // Строку можно заменить на коллбек для получения соединения,
    DbPool(std::string const& connection_string);

    std::shared_ptr<BDb> get();

    // Вернуть соединение в пул
    void back(std::shared_ptr<BDb> conn);

    // Удалить старые соединения
    void delete_older_then(time_t threshold);

    // Пулл будет более денамичным. Будет реагировать на изменение конфига
    void set_connection_string(std::string const& connection_string);

    bool empty() const;

private:

    std::string connection_string_;

    // Соединения к базе используются во множестве потоков
    mutable std::mutex mutex_;

    // Соединения отсортированы по имени
    std::map<time_t, std::shared_ptr<BDb>> connections_;
};
