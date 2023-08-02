#include <chrono>
#include <iostream>
#include <mutex>  // std::unique_lock
#include <shared_mutex>
#include <syncstream>
#include <thread>
#include <vector>

class ThreadSafeCounter {
   public:
    ThreadSafeCounter() = default;

    // Multiple threads/readers can read the counter's value at the same time.
    unsigned int get() const {
        std::shared_lock lock(mutex_);
        std::cout << std::this_thread::get_id() << " read " << value_ << '\n';
        return value_;
    }

    // Only one thread/writer can increment/write the counter's value.
    void increment() {
        std::unique_lock lock(mutex_);
        ++value_;
        std::cout << std::this_thread::get_id() << " write\n";
    }

    // Only one thread/writer can reset/write the counter's value.
    void reset() {
        std::unique_lock lock(mutex_);
        value_ = 0;
    }

   private:
    mutable std::shared_mutex mutex_;
    unsigned int value_{};
};

int main() {
    ThreadSafeCounter counter;

    auto increment_and_print = [&counter]() {
        for (int i{}; i < 10; ++i) {
            counter.increment();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            counter.get();
        }
    };

    std::vector<std::thread> thread_vec;
    for (unsigned i = 0; i < 3; ++i) {
        thread_vec.emplace_back(increment_and_print);
    }

    for (auto& t : thread_vec) {
        t.join();
    }
}