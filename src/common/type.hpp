#pragma once

template<typename Class, typename ValueType>
using member_ptr_t = ValueType Class::*;
