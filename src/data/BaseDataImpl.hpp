#pragma once

#include <mutex>
#include <type_traits>
#include <utility>
#include <mutex>

#include <db/BDb.h>
#include <systems/Timer.h>
#include "list/BaseObjList.h"
#include "BaseData.hpp"
#include "DataVersion.hpp"
#include <chrono>

template <typename T, typename ... ListInputs>
class BaseDataImpl : public BaseData {

    static_assert(std::is_base_of<BaseObjList, T>::value, "T must be derived from BaseObjList");

public:

    BaseDataImpl(ListInputs ...inputs)
            : listInputs(std::forward<ListInputs>(inputs)...)
    {}

    void load(BDb* db) override {
        LoadingTransaction loadingTransaction( this, listInputs );
        loadingTransaction->load(db);
        loadingTransaction.setCompleted();
    }

    bool ready() const override {
        return state == BaseDataState::LOADED;
    }

    std::shared_ptr<const BaseObjList> get_base() const override {
        return data;
    }

    virtual time_t get_time_in_ms() const override {
        return loading_start_ + last_loading_time_;
    }

    virtual time_t last_loading_time() const override {
        if (state == BaseDataState::LOADING) {
            return now() - loading_start_;
        }
        return last_loading_time_;
    }

    std::shared_ptr<const T> get() const {
        return std::dynamic_pointer_cast<const T>(data);
    }

    std::shared_ptr<T>  get() {
        return std::const_pointer_cast<T>(
                std::dynamic_pointer_cast<const T>(data));
    }
    
protected:
    class LoadingTransaction {

        // Если передан хотя бы один параметр в констркутор BaseDataImpl,
        // То вызывается эта функция-конструктор с кортежем из переданных аргуменов
        template<typename ...Types>
        std::shared_ptr<BaseObjList> create(std::tuple<Types...> const& t){
            return std::shared_ptr<BaseObjList>(new T(t));
        }

        // На случай, если параметров не было
        std::shared_ptr<BaseObjList> create(std::tuple<> const& t){
            return std::shared_ptr<BaseObjList>(new T());
        }

    public:
        LoadingTransaction(BaseDataImpl<T, ListInputs...>* that, std::tuple<ListInputs...> const& tup):
                _that{ that }
        {
            _newData = create(tup);

            if (_that->state == BaseDataState::LOADING) {
                throw std::logic_error{ "can't execute nested loading transaction" };
            }
            _that->start_loading();
            _that->state = BaseDataState::LOADING;
        }

        ~LoadingTransaction() {
            _that->end_loading();
            _that->state = _newState;
            if (_newState == BaseDataState::LOADED) {
                _that->data = std::move(_newData);
                _that->version_ = fetch_data_version();
            } 
        }

        void setCompleted() {
            _newState = BaseDataState::LOADED;
        }

        T* operator->() const {
            return dynamic_cast<T*>(_newData.get());
        }

    private:
        BaseDataImpl<T, ListInputs...>* const _that;
        BaseDataState _newState = BaseDataState::NOT_LOADED;
        std::shared_ptr<BaseObjList> _newData;
    };

    static time_t now() {
        using namespace std::chrono;
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }

    void start_loading() {
        loading_start_ = now();
    }

    void end_loading() {
        last_loading_time_ = now() - loading_start_;
        total_time_in_ms_ += last_loading_time_;
    }

private:

    time_t last_loading_time_ = 0;
    time_t loading_start_ = 0;

    std::shared_ptr<const BaseObjList> data;
    std::tuple<ListInputs...> listInputs;
};

template<typename T, typename ... Arguments>
inline std::shared_ptr<BaseData> MakeData(Arguments&& ... args){
    return std::shared_ptr<BaseData>(
            new BaseDataImpl<T, typename std::decay<Arguments>::type...>
                    (std::forward<Arguments>(args)...));
}
