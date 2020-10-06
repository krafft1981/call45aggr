#include "Allocator.hpp"
#include <string.h>

void* Allocator::allocate_aligned(size_t size, size_t alignment){
    byte_t* current_position = storage_.back().get() + position_;

    // Количество байт для выравнивания
    size_t bytes_for_alignment  =  reinterpret_cast<size_t>(current_position) % alignment;
    if (bytes_for_alignment != 0){
        bytes_for_alignment = alignment - bytes_for_alignment;
    }

    if (position_ + bytes_for_alignment + size > extent_size){
        allocate_extent();
        return allocate_aligned(size,alignment);
    }

    byte_t* ptr = current_position + bytes_for_alignment ;
    position_ += bytes_for_alignment + size;
    return ptr;
}

void  Allocator::allocate_extent(){
    storage_.emplace_back(new byte_t[extent_size]);
    position_ = 0;
    memset(storage_.back().get(), 0, extent_size);
}

Allocator::Allocator() {
    // По умолчанию есть хотя бы один элемент
    allocate_extent();
}

size_t Allocator::consumedMemory() const {
    return storage_.size() * extent_size;
}
