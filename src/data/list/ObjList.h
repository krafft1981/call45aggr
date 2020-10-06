#pragma once

#include "BaseObjList.h"
#include "../common.h"
#include "db/BDb.h"
#include "systems/Exception.h"

template <typename T>
class ObjList : public BaseObjList {
public:
    // TODO ? may be "load" have to be non-virtual method (or virtual+final), so all additional logic descendants must keep in "after_load" method
    virtual void load(BDb* db) override {

        BDbResult res = db->query(sql(db));
        
        loadtime = time(NULL);

        before_load();

        data.clear();
        data.resize(res.size());

        while (res.next()) {
            parse_item(res, &data.at(res.position()));
        }

        after_load();
    }

    virtual std::size_t size() const override {
        return data.size();
    }

    virtual std::size_t dataSize() const override {
        return sizeof(T) * data.size();
    }

    const T* get(std::size_t i) const {
        return &data.at(i);
    }

    T* get(std::size_t i) {
        return &data.at(i);
    }

    const std::vector<T> & getData() const { return data; }

protected:
    virtual std::string sql(BDb* db = nullptr) = 0;
    virtual void parse_item(BDbResult& row, T* item) const = 0;
    virtual void after_load() {};
    virtual void before_load() {};

    std::vector<T> data;
};
