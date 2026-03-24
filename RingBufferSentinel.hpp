#pragma once

#include <array>
#include <cstddef>
#include <optional>

template <typename T, std::size_t N> class RingBufferCount {
  public:
    static constexpr std::size_t capacity() noexcept { return N; }

    [[nodiscard]] bool empty() const noexcept { return head_ == tail_; }

    [[nodiscard]] bool full() const noexcept { return (head_ + 1) % kBufSize == tail_; }

    [[nodiscard]] std::size_t size() const noexcept {
        if (head_ >= tail_) {
            return head_ - tail_;
        }
        return kBufSize - (tail_ - head_);
    }

    bool push(const T& value) noexcept {
        bool overwrote = false;

        if ((head_ + 1) % kBufSize == tail_) {
            tail_ = (tail_ + 1) % kBufSize;
            overwrote = true;
        }
        data_[head_] = value;
        head_ = (head_ + 1) % kBufSize;

        return overwrote;
    }

    std::optional<T> pop() noexcept {
        if (head_ == tail_) {
            return std::nullopt;
        }

        T value = data_[tail_];
        tail_ = (tail_ + 1) % kBufSize;

        return value;
    }

    bool try_push(const T& value) noexcept {
        if ((head_ + 1) % kBufSize == tail_) {
            return false;
        }

        data_[head_] = value;
        head_ = (head_ + 1) % kBufSize;

        return true;
    }

    std::optional<T> peek() const noexcept {
        if (head_ == tail_) {
            return std::nullopt;
        }
        return data_[tail_];
    }

    void clear() noexcept {
        head_ = 0;
        tail_ = 0;
    }

    // These are visualization helpers meant to expose internal state for ImGui
    // Not part of the ring buffer API.
    [[nodiscard]] std::size_t head() const noexcept { return head_; }
    [[nodiscard]] std::size_t tail() const noexcept { return tail_; }

    // raw slot access by index, this will allow the visualizer to draw all slots
    const T& operator[](std::size_t index) const noexcept { return data_[index]; }

  private:
    static constexpr std::size_t kBufSize = N + 1;

    std::array<T, kBufSize> data_{};
    std::size_t head_{0};
    std::size_t tail_{0};
};