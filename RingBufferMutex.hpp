#pragma once

#include <array>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <optional>

template <typename T, std::size_t N> class RingBufferMutex {
  public:
    static constexpr std::size_t capacity() noexcept { return N; }

    std::size_t size() const noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        return count_;
    }

    bool empty() const noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        return count_ == 0;
    }

    bool full() const noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        return count_ == N;
    }

    void push(const T& value) {
        std::unique_lock<std::mutex> lock(mutex_);

        notFull_.wait(lock, [this]() { return count_ < N; });

        data_[head_] = value;
        head_ = (head_ + 1) % N;
        ++count_;

        lock.unlock();
        notEmpty_.notify_one();
    }

    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock(mutex_);

        notEmpty_.wait(lock, [this]() { return count_ > 0; });

        T value = data_[tail_];
        tail_ = (tail_ + 1) % N;
        --count_;

        lock.unlock();
        notFull_.notify_one();

        return value;
    }

    // this is a non-blocking version of pop for imgui, to make sure UI doesn't stall
    // imgui will use this and not pop()
    std::optional<T> try_pop() {
        std::unique_lock<std::mutex> lock(mutex_);

        if (count_ == 0) {
            return std::nullopt;
        }

        T value = data_[tail_];
        tail_ = (tail_ + 1) % N;
        --count_;

        lock.unlock();
        notFull_.notify_one();

        return value;
    }

    // non blocking push
    bool try_push(const T& value) {
        std::unique_lock<std::mutex> lock(mutex_);

        if (count_ == N) {
            return false;
        }

        data_[head_] = value;
        head_ = (head_ + 1) % N;
        ++count_;

        lock.unlock();
        notEmpty_.notify_one();

        return true;
    }

    std::optional<T> peek() const {
        std::unique_lock<std::mutex> lock(mutex_);

        if (count_ == 0) {
            return std::nullopt;
        }
        return data_[tail_];
    }

    void clear() {
        std::unique_lock<std::mutex> lock(mutex_);

        head_ = 0;
        tail_ = 0;
        count_ = 0;

        lock.unlock();
        notFull_.notify_all();
    }

    // These are visualization helpers meant to expose internal state for ImGui
    // Not part of the ring buffer API.
    [[nodiscard]] std::size_t head() const {
        std::unique_lock<std::mutex> lock(mutex_);
        return head_;
    }

    [[nodiscard]] std::size_t tail() const {
        std::unique_lock<std::mutex> lock(mutex_);
        return tail_;
    }

    const T& operator[](std::size_t index) const {
        std::unique_lock<std::mutex> lock(mutex_);
        return data_[index];
    }

  private:
    std::array<T, N> data_{};
    std::size_t head_{0};
    std::size_t tail_{0};
    std::size_t count_{0};

    mutable std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::condition_variable notFull_;
};