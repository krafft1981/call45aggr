#pragma once

#include "array_view.hpp"
#include "type.hpp"
#include <algorithm>

// Сахар для поиска значений в сортированных контейнерах-массивах
// \param begin, end - начало и конец диапазона поиска
// \param seeking - искомое значение
// \param mem_ptr - ссылка на член класса, по которому производится поиск
// \return часть контейнера-массива, которая удовлетворяет запросу
template<typename IteratorType, typename DataType, typename ValueType>
inline array_view<DataType> extract_range(IteratorType begin, IteratorType end,
                                          ValueType seeking, member_ptr_t<DataType, ValueType> mem_ptr) {

    auto lb_cmp = [=](const DataType & left, ValueType seeking) { return (left.*mem_ptr) < seeking; };
    auto lb_it = std::lower_bound(begin, end, seeking, lb_cmp);
    if (lb_it == end){
        return array_view<DataType>(nullptr, 0);
    }

    auto ub_cmp = [=](ValueType seeking, const DataType & right) { return seeking < (right.*mem_ptr); };
    auto ub_it = std::upper_bound(lb_it, end, seeking, ub_cmp);

    return array_view<DataType>(&*lb_it, static_cast<size_t>(std::distance(lb_it,ub_it)));
}

template<typename DataType, typename ValueType>
inline array_view<DataType> extract_range(std::vector<DataType> const& container,
ValueType seeking, member_ptr_t<DataType, ValueType> mem_ptr ){
    return extract_range(container.begin(), container.end(), seeking, mem_ptr);
}
