#pragma once

#include <vector>
#include <type_traits>
#include <assert.h>
#include <stdlib.h>

template <typename T>
struct array_view_iterator {
    typedef std::ptrdiff_t difference_type;
    typedef std::random_access_iterator_tag iterator_category;
    typedef T* pointer;
    typedef T& reference;
    typedef T value_type;

    array_view_iterator(T* ptr)
        : ptr_(ptr) {}

    bool operator!=(array_view_iterator const& other) const { return ptr_ != other.ptr_; }
    bool operator==(array_view_iterator const& other) const { return ptr_ == other.ptr_; }

    bool operator>(array_view_iterator const& other) const { return ptr_ > other.ptr_; }
    bool operator>=(array_view_iterator const& other) const { return ptr_ >= other.ptr_; }

    bool operator<(array_view_iterator const& other) const { return ptr_ < other.ptr_; }
    bool operator<=(array_view_iterator const& other) const { return ptr_ <= other.ptr_; }

    array_view_iterator& operator++() {
        ++ptr_;
        return *this;
    }

    array_view_iterator operator++(int) {
        ++ptr_;
        return array_view_iterator(ptr_ - 1);
    }

    array_view_iterator& operator--() {
        --ptr_;
        return *this;
    }

    array_view_iterator operator--(int) {
        --ptr_;
        return array_view_iterator(ptr_ + 1);
    }


    array_view_iterator  operator+(difference_type n) const { return array_view_iterator(ptr_ + n); }
    array_view_iterator operator-(difference_type n) const { return array_view_iterator(ptr_ - n); }

    array_view_iterator& operator+=(difference_type n) {
        ptr_ += n;
        return *this;
    }
    array_view_iterator& operator-=(difference_type n) {
        ptr_ -= n;
        return *this;
    }

    difference_type  operator-(const array_view_iterator& rhs) const {
        return ptr_ - rhs.ptr_;
    }

    reference operator[](difference_type n) const {
        return *(ptr_ + n);
    }

    reference operator*() const { return *ptr_; }
    pointer operator->() const { return ptr_; }

private:
    T* ptr_;
};

template <typename T>
struct array_view {

    using value_type = typename std::add_const<typename std::decay<T>::type>::type;
    using pointer_type = value_type *;
    using reference_type = value_type &;

    template<unsigned N>
    array_view(value_type(&v)[N])
            : ptr_(&v), size_(N) {}

    array_view(pointer_type ptr, size_t size)
            : ptr_(ptr), size_(size) {}

    array_view(std::vector<T> const& vec)
            : ptr_(vec.data())
            , size_(vec.size()) {}

    array_view(array_view const& other) {
        ptr_ = other.ptr_;
        size_ = other.size_;
    }

    array_view()
            : ptr_(nullptr)
            , size_(0) {}

    ~array_view() {}

    bool empty() const {
        return size_ == 0 || ptr_ == nullptr;
    }

    explicit operator bool() const {
        return !empty();
    }

    size_t size() const { return size_; }

    reference_type operator[](size_t index) const {
        assert(index < size_);
        return *(ptr_ + index);
    }

    reference_type front() const {
        assert(ptr_ != nullptr && size_ > 0);
        return *ptr_;
    }

    reference_type back() const {
        assert(ptr_ != nullptr && size_ > 0);
        return *(ptr_ + size_ - 1);
    }

    pointer_type data() const {
        return ptr_;
    }

    array_view_iterator<value_type> begin() const { return array_view_iterator<value_type>(ptr_); }
    array_view_iterator<value_type> end() const { return array_view_iterator<value_type>(ptr_ + size_); }

private:
    pointer_type ptr_;
    size_t size_;
};

//template<typename T> array_view(T*, size_t) -> array_view<T>;
