#include "ExceptionHolder.h"


class ExceptionHolderImpl {
public:

    void save_exception(std::exception_ptr ex){
        exception_ = ex;
    }

    void clean_exception() {
        exception_ = std::exception_ptr();
    }

    void try_rethrow(){
        if (!exception_) {
            return;
        }

        auto ex = exception_;
        clean_exception();
        std::rethrow_exception(ex);
    }

    bool have_exception() const {
        return static_cast<bool>(exception_);
    }

private:
    // implicit shared
    std::exception_ptr  exception_;
};

ExceptionHolder::ExceptionHolder()
        : instance_(std::make_shared<ExceptionHolderImpl>())
{}

void ExceptionHolder::save_exception(std::exception_ptr ex){
    instance_->save_exception(ex);
}

void ExceptionHolder::try_rethrow(){
    instance_->try_rethrow();
}

bool ExceptionHolder::have_exception() const {
    return instance_->have_exception();
}

ExceptionHolder tl_exception_holder(){
    thread_local ExceptionHolder tl;
    return tl;
}
