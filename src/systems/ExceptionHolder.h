#pragma once

#include <memory>

class ExceptionHolderImpl;
class ExceptionHolder {
public:
    ExceptionHolder();

    // Сохраняет состояние исключения
    void save_exception(std::exception_ptr ex);

    // Выкидывает хранимое исключение
    void try_rethrow();

    bool have_exception() const;

private:
    // implicit shared
    std::shared_ptr<ExceptionHolderImpl> instance_;
};

// Получает объект для текущего потока
ExceptionHolder tl_exception_holder();
