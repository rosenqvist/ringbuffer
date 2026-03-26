#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <optional>

template <typename T, std::size_t N> class RingBufferSPSC {
    static_assert(N > 0, "Capacity must be greater than zero");

  private:
    static constexpr std::size_t kBufSize = N + 1;

    std::array<T, kBufSize> data_{};

    alignas(64) std::atomic<std::size_t> head_{0};
    alignas(64) std::atomic<std::size_t> tail_{0};
};
