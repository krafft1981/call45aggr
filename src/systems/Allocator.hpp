#pragma once

#include <vector>
#include <memory>

using std::vector;
using std::unique_ptr;

struct Allocator{
    static constexpr size_t extent_size = 0x10000;

    using byte_t = unsigned char;
    using extent_t = unique_ptr<byte_t[] >;

    Allocator();
    Allocator(Allocator const& ) = delete;
    Allocator(Allocator&&) = default;
    ~Allocator() = default;

    template<typename T>
    T* allocate();

    size_t consumedMemory() const;

private:

    void* allocate_aligned(size_t size, size_t alignment);
    void  allocate_extent();

    // Позиция в последнем блоке памяти. По умолчанию выделяется
    size_t            position_ = extent_size;
    vector<extent_t>  storage_;
};

template<typename T>
T* Allocator::allocate() {
    return reinterpret_cast<T*>(allocate_aligned(sizeof(T), alignof(T)));
}
