#include "ParamQuery.h"
#include "DbException.h"
#include "../common/concat.hpp"

ParamQuery::ParamQuery (const char* query, std::size_t width)
        : ParamQuery(std::string(query), width)
{}

ParamQuery::ParamQuery (std::string&& query, std::size_t width)
        :
        query_ (std::move(query)),
        width_ (width)
{
    rows_.emplace_back();
    rows_.front().reserve(width);
}

ParamQuery::ParamQuery (ParamQuery&& query)
        :
        query_       (std::move(query.query_)),
        width_       (query.width_),
        rows_(std::move(query.rows_)){
    query.width_ = 0;
}

ParamQuery& ParamQuery::operator = (ParamQuery&& query) noexcept {
    query_ = std::move(query.query_);
    width_ = query.width_;
    rows_ = std::move(query.rows_);

    query.width_ = 0;

    return *this;
}


ParamQuery::~ParamQuery() {
    for (auto& level : rows_){
        for (char* ptr : level) {
            delete[] ptr;
        }
    }
}


bool ParamQuery::isEmpty() const {
    for (auto& level : rows_) {
        if (!level.empty()) {
            return false;
        }
    }
    return true;
}

bool ParamQuery::isEmpty (size_t row_idx) const {
    if (rows_.size() <= row_idx) {
        return true;
    }
    // Всегда будет один уровень
    return rows_[row_idx].empty ();
}

std::size_t ParamQuery::getWidth () const {
    return width_;
}

const std::string& ParamQuery::getQuery () const {
    return query_;
}

ParamQuery& ParamQuery::addValue (std::string&& value) {
    return addValue(value.c_str());
}

ParamQuery& ParamQuery::addValue (std::string const& value) {
    return addValue(value.c_str());
}

ParamQuery& ParamQuery::addValue (const char* src) {
    char* dst = new char[strlen(src) + 1];
    strcpy(dst, src);
    rows_.back().push_back(dst);
    return *this;
}


ParamQuery& ParamQuery::addNull() {
    rows_.back().push_back(nullptr);
    return *this;
}

const char *const *ParamQuery::parameters(size_t row_idx) const {

    if (rows_.size() < row_idx) {
        throw std::runtime_error(concat("Выход за границы уровня параметров ", row_idx, "(из ", rows_.size(), ")"));
    }

    // Игнор пустых значений
    if (rows_[row_idx].empty()) {
        return nullptr;
    }

    if (rows_[row_idx].size() != width_) {
        throw std::runtime_error(concat("Неверное количество параметров для запроса \"", query_, "\". Ожидается: ", width_, ", передается: ", rows_[row_idx].size()));
    }

    return rows_[row_idx].data();
}

void ParamQuery::endRow(){
    // Добавление новой строки для записи
    rows_.emplace_back();
    rows_.back().reserve(width_);
}

std::size_t ParamQuery::levelsCount() const {
    return rows_.size();
}
