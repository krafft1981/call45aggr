#pragma once

#include <vector>
#include <string>

enum class split_opts {
    allow_blank = 0,
    skip_blank = 1
};

/* Делит строку согласно списку сепараторов
 * source_str - строка
 * separators - список односимвольных сепараторов
 * options    - опции разделения строки оставить пустые строки (allow_blank) или удалить (skip_blank)
 * \return список разеделенных строк
 */
std::vector<std::string> split_any(std::string const& source_str, char const* separators, split_opts options =  split_opts::allow_blank) ;

/* Парсит значения типов numeric, разделенные сепаратарами. Удаляет пустые строки
 * source_str - строка (split_any)
 * separators - список односимвольных сепараторов (split_any)
 * \return список значений
 */
template<typename Type>
std::vector<Type> split_to(std::string const& source_str, char const* separators);

/* Заменяет все вхождения одной строки на другую
 * src - исходная строка
 * from - заменяемая подстрока
 * to -  новая подстрока
 * \return src с заменой строки from на to
 */
std::string replace_all_subs(std::string const& src, std::string from, std::string to);



// Функции для обрезания строк
std::string &ltrim(std::string &s) ;
std::string &rtrim(std::string &s) ;
std::string &trim(std::string &s) ;
