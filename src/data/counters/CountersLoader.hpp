#pragma once

#include <memory>
#include "ObjCounter.hpp"
#include <common.hpp>
#include <data/DataVersion.hpp>

// Этот объект нужен для помощи в загрузке счетчиков
template<typename T>
class CountersLoader {

public:
    CountersLoader() {}


    void load(BDb* db, std::vector<long long int> const& ids)  {

        // Пока пусть будет так.
        // TODO: Позже разрешить вопрос с загрузкой счетчиков
        auto currentCounter = (counter_ ? counter_ : std::make_shared<T>());

        // Запрос и парсинг данных переопределны для каждого объекта
        auto updatedData = currentCounter->load(db, ids);
        bool haveDeletedRows = (updatedData.size() != ids.size());

        // Изменены индексы контейнера
        if (!counter_ || haveDeletedRows || haveNewRows(ids)) {
            auto newCounter = std::make_shared<T>();
            newCounter->data = currentCounter->data;

            for (auto const& id : ids) {
                newCounter->data.erase(id);
            }
            for (auto& v : updatedData) {
                // Любой id должен быть преобразуем к long long int
                newCounter->data[v.id()] = std::move(v);
            }

            newCounter->data_version_ = fetch_data_version();

            counter_ = newCounter;
        } else { // Изменен сам контейнер
            // Копирование не изменяет имси
            for (auto const& v :updatedData){
                counter_->data[v.id()] = v;
            }
        }
    }

    std::shared_ptr<T const> get() const{
        return counter_;
    }

private:

    bool haveNewRows(std::vector<long long int> const& ids) {

        if (!counter_){
            return true;
        }

        auto const& data = counter_->getData();
        for (long long int const& id : ids) {
            if (data.find(id) == data.end()){
                return true;
            }
        }
        return false;
    }

    std::shared_ptr<T> counter_;
};
