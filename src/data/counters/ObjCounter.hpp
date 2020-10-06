#pragma once

#include "BaseObjCounter.hpp"
#include <unordered_map>
#include <db/BDb.h>
#include <memory>

template<typename T>
class CountersLoader;

template <typename T>
class ObjCounter : public BaseObjCounter {
public:

    using ValueType = T;

    // Загружает (и только загружает) некоторое количество данных
    std::vector<T> load(BDb* db, std::vector<long long int> const& ids) {
        BDbResult res = db->query(sql(ids));

        loadtime = time(NULL);

        before_load();

        std::vector<T> tmp_data(res.size());
        while (res.next()) {
            parse_item(res, &tmp_data.at(res.position()));
        }

        after_load();

        return tmp_data;
    }

    virtual std::size_t size() const override {
        return data.size();
    }

    virtual std::size_t dataSize() const override {
        return sizeof(T) * data.size();
    }

    const T* get(long long int const& i) const {
        return &data.at(i);
    }

    T* get(long long int const& i) {
        return &data.at(i);
    }

    const std::unordered_map<long long int, T> & getData() const { return data; }

    std::shared_ptr<ObjCounter<T>> deepCopy() const {
        auto newObject=  std::make_shared<ObjCounter<T>>();
        newObject->data = data;
        return newObject;
    }

protected:
    virtual std::string sql(std::vector<long long int> const& ids) const = 0;
    virtual void parse_item(BDbResult& row, T* item) const = 0;
    virtual void after_load() {};
    virtual void before_load() {};

    // imsi to some data
    std::unordered_map<long long int, T> data;

    template<typename TheSameType>
    friend class CountersLoader;
};
