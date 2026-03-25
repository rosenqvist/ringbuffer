#pragma once

#include <array>
#include <cstddef>
#include <optional>

template <typename T, std::size_t N> class RingBufferUnbounded {
    static_assert(N > 0, "Capacity must be greater than zero");
    static_assert((N & (N - 1)) == 0, "Capacity must be a power of two");

    [[nodiscard]] std::size_t size() const noexcept { return head_ - tail_; }
    [[nodiscard]] bool empty() const noexcept { return head_ == tail_; }
    [[nodiscard]] bool full() const noexcept { return head_ - tail_ == N; }

    [[nodiscard]] std::size_t mask(std::size_t index) const noexcept { return index & (N - 1); }

    bool push(const T& value) noexcept {
        bool overwrote = false;

        if (head_ - tail_ == N) {
            ++tail_;
            overwrote = true;
        }

        data_[mask(head_)] = value;
        ++head_;

        return overwrote;
    }

    std::optional<T> pop() noexcept {
        if (head_ == tail_) {
            return std::nullopt;
        }

        T value = data_[mask(tail_)];
        ++tail_;

        return value;
    }

    bool try_push(const T& value) noexcept {
        if (head_ - tail_ == N) {
            return false;
        }

        data_[mask(head_)] = value;
        ++head_;

        return true;
    }

    std::optional<T> peek() const noexcept {
        if (head_ == tail_) {
            return std::nullopt;
        }
        return data_[mask(tail_)];
    }

    void clear() noexcept {
        head_ = 0;
        tail_ = 0;
    }

    // Raw unbounded counters
    [[nodiscard]] std::size_t head_raw() const noexcept { return head_; }
    [[nodiscard]] std::size_t tail_raw() const noexcept { return tail_; }

    // Masked positions are actual array slots (what the visualizer will draw)
    [[nodiscard]] std::size_t head() const noexcept { return mask(head_); }
    [[nodiscard]] std::size_t tail() const noexcept { return mask(tail_); }

  private:
    std::array<T, N> data_{};
    std::size_t head_{0};
    std::size_t tail_{0};
};