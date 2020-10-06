
#pragma  once

#include <memory>

#include "list/BaseObjList.h"
#include <assert.h>

enum class BaseDataState { NOT_LOADED, LOADING, LOADED };

class BaseData {

public:
    virtual void load(BDb* db) = 0;
    virtual bool ready() const = 0;
    virtual std::shared_ptr<const BaseObjList> get_base() const = 0;
    virtual time_t get_time_in_ms() const = 0;
    virtual time_t last_loading_time() const = 0;
    virtual ~BaseData(){}

    time_t total_loading_time() const {
        return total_time_in_ms_;
    }

    BaseDataState get_state() const{
        return state;
    }

    size_t version() const {
        return version_;
    }

protected:
    BaseDataState state = BaseDataState::NOT_LOADED;
    size_t version_ = 0;
    time_t total_time_in_ms_ = 0;

};

// ImplicitShared сахар для BaseData
template<typename ListType>
class DataAccessor {
    std::shared_ptr<BaseData> data_;
    //std::shared_ptr<const ListType> data_;
public:

    DataAccessor(std::shared_ptr<BaseData> data)
            : data_(data)
    {
        //static_assert(std::is_base_of<ListType, BaseData>::value, "Wrong derived type");
    }

    std::shared_ptr<const ListType> get() const {
        return std::dynamic_pointer_cast<const ListType>(data_->get_base());
    }

    void load(BDb* db) {
        data_->load(db);
    }

    bool ready() const {
        return data_->ready();
    }

    size_t get_load_time_in_ms() const {
        return data_->get_time_in_ms();
    };

    BaseDataState get_state() const{
        return data_->get_state();
    }

    operator bool() const {
        return static_cast<bool>(data_->get_base());
    }

    // Если понадобится перебор одинаковых элементов
    // не придется доставать инстанцированную часть напрямую
    operator std::shared_ptr<BaseData>(){
        return data_;
    }
};
