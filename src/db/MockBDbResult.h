#pragma once

#include "MockData.h"

// Состояние для MockData
class MockBDbResult {
protected:
    MockData data_;

    vector<string> current_state_;
    int64_t index_;

    void parse_current_state();

public:
    explicit MockBDbResult(MockData data);
    MockBDbResult(const MockBDbResult&) = delete;
    MockBDbResult(MockBDbResult&&) noexcept = default;
    MockBDbResult& operator=(const MockBDbResult&) = delete;
    MockBDbResult& operator=(MockBDbResult&& res) noexcept = default;

    ~MockBDbResult() = default;
    size_t size();
    size_t position();
    char const* get(int r, int f);
    bool is_null(int f);
    char const* get(int f);
    int get_i(int f);
    double get_d(int f);
    long long int get_ll(int f);
    bool get_b(int f);
    string get_s(int f);
    void fill_cs(int f, char * str, size_t size);
    bool next();
    bool previous();
    bool first();
    bool last();
    void force_free();
};

using BDbResult = MockBDbResult;
