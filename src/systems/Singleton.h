#pragma once

template <typename T>
class Singleton
{
public:
    static T* instance() {
        static T instance;
        return &instance;
    }

protected:
    Singleton() = default;

private:
    Singleton(const Singleton<T>&) = delete;
    Singleton(Singleton<T>&&) = delete;
    Singleton<T>& operator=(const Singleton<T>&) = delete;
    Singleton<T>& operator=(Singleton<T>&&) = delete;
};
