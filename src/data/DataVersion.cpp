#include <atomic>
#include "DataVersion.hpp"

static std::atomic_size_t global_data_version {1};

size_t data_version() {
    return global_data_version.load(std::memory_order_relaxed);
}

size_t fetch_data_version() {
    return global_data_version.fetch_add(1, std::memory_order_relaxed);
}
