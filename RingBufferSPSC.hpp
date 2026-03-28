#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <optional>

template <typename T, std::size_t N> class RingBufferSPSC {
    static_assert(N > 0, "Capacity must be greater than zero");

  public:
    static constexpr std::size_t capacity() noexcept { return N; }

    bool try_push(const T& value) noexcept {
        const auto head = head_.load(std::memory_order_relaxed);
        const auto next_head = (head + 1) % kBufSize;

        if (next_head == tail_.load(std::memory_order_acquire)) {
            return false;
        }

        data_[head] = value;
        head_.store(next_head, std::memory_order_release);

        return true;
    }

    std::optional<T> try_pop() noexcept {
        const auto tail = tail_.load(std::memory_order_relaxed);

        if (tail == head_.load(std::memory_order_acquire)) {
            return std::nullopt;
        }

        T value = data_[tail];
        tail_.store((tail + 1) % kBufSize, std::memory_order_release);

        return value;
    }

    std::size_t size() const noexcept {
        const auto head = head_.load(std::memory_order_acquire);
        const auto tail = tail_.load(std::memory_order_acquire);

        if (head >= tail) {
            return head - tail;
        }

        return kBufSize - (tail - head);
    }

    bool empty() const noexcept {
        return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
    }

    bool full() const noexcept {
        const auto head = head_.load(std::memory_order_relaxed);
        return (head + 1) % kBufSize == tail_.load(std::memory_order_acquire);
    }

    std::optional<T> peek() const noexcept {
        const auto tail = tail_.load(std::memory_order_relaxed);

        if (tail == head_.load(std::memory_order_acquire)) {
            return std::nullopt;
        }
        return data_[tail];
    }

    void clear() noexcept {
        head_.store(0, std::memory_order_relaxed);
        tail_.store(0, std::memory_order_relaxed);
    }

    // These are visualization helpers meant to expose internal state for ImGui
    // Not part of the ring buffer API.

    [[nodiscard]] std::size_t head() const noexcept {
        return head_.load(std::memory_order_acquire);
    }

    [[nodiscard]] std::size_t tail() const noexcept {
        return tail_.load(std::memory_order_acquire);
    }

    const T& operator[](std::size_t index) const noexcept { return data_[index]; }

  private:
    static constexpr std::size_t kBufSize = N + 1;

    std::array<T, kBufSize> data_{};

    alignas(64) std::atomic<std::size_t> head_{0};
    alignas(64) std::atomic<std::size_t> tail_{0};
};
