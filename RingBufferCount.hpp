#pragma once

#include <array>
#include <cstddef>
#include <optional>

template <typename T, std::size_t N> class RingBufferCount {
  public:
    static constexpr std::size_t capacity() noexcept { return N; }

    [[nodiscard]] std::size_t size() const noexcept { return count_; }

    [[nodiscard]] bool empty() const noexcept { return count_ == 0; }

    [[nodiscard]] bool full() const noexcept { return count_ == N; }

    bool push(const T& value) noexcept {
        bool overwrote = false;

        if (count_ == N) {
            tail_ = (tail_ + 1) % N;
        } else {
            ++count_;
        }

        data_[head_] = value;
        head_ = (head_ + 1) % N;
        return overwrote;
    }

    std::optional<T> pop() noexcept {
        if (count_ == 0) {
            return std::nullopt;
        }

        T value = data_[tail_];
        tail_ = (tail_ + 1) % N;
        --count_;

        return value;
    }

    bool try_push(const T& value) noexcept {
        if (count_ == N) {
            return false;
        }

        data_[head_] = value;
        head_ = (head_ + 1) & N;
        ++count_;
    }

    std::optional<T> peek() const noexcept {
        if (count_ == 0) {
            return std::nullopt;
        }
        return data_[tail_];
    }

    void clear() noexcept {
        head_ = 0;
        tail_ = 0;
        count_ = 0;
    }

    // These are visualization helpers meant to expose internal state for ImGui
    // Not part of the ring buffer API.
    [[nodiscard]] std::size_t head() const noexcept { return head_; }
    [[nodiscard]] std::size_t tail() const noexcept { return tail_; }

    // raw slot access by index, this will allow the visualizer to draw all slots
    const T& operator[](std::size_t index) const noexcept { return data_[index]; }

  private:
    std::array<T, N> data_{};
    std::size_t head_{0};
    std::size_t tail_{0};
    std::size_t count_{0};
};
