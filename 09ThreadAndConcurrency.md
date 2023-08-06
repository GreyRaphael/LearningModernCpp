# Threading and Concurrency

- [Threading and Concurrency](#threading-and-concurrency)
  - [Basic Usage](#basic-usage)
  - [thread sleep \& yield](#thread-sleep--yield)
  - [`mutex`](#mutex)
  - [one writer \& multi-readers](#one-writer--multi-readers)
  - [Handling exceptions from thread functions](#handling-exceptions-from-thread-functions)
  - [`std::condition_variable`](#stdcondition_variable)
  - [Using promises and futures to return values from threads](#using-promises-and-futures-to-return-values-from-threads)
  - [`std::async` with `std::future`](#stdasync-with-stdfuture)
  - [`std::atomic`](#stdatomic)
  - [Implementing parallel `map` and `reduce`](#implementing-parallel-map-and-reduce)

## Basic Usage

```cpp
#include <iostream>
#include <thread>

void func1() {
    // std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "thread func without params" << '\n';
}

void func2(int const i, double const d, std::string const s) {
    std::cout << i << ", " << d << ", " << s << '\n';
}

void func3(int& i) {
    i *= 2;
}

int main() {
    // thread func without params
    {
        std::thread t(func1);
        t.join();
    }
    {
        std::thread t([]() { std::cout << "thread lambda func without params" << '\n'; });
        t.join();
    }
    // thread func with params
    {
        std::thread t(func2, 10, 20.1, "hello");
        t.join();
    }
    {
        std::thread t(
            [](int const i, double const d, std::string const s) { std::cout << i << ", " << d << ", " << s << '\n'; },
            11, 22.2, "world");
        t.join();
    }
    // thead func change params
    {
        int a = 100;
        std::thread t(func3, std::ref(a));
        t.join();
        std::cout << a << '\n';  // 200
    }
    {
        int b = 1000;
        std::thread t(
            [](int& i) { i *= 2; },
            std::ref(b));
        t.join();
        std::cout << b << '\n';  // 2000
    }
}
```

## thread sleep & yield

- `sleep_for()`: sleep for a duration
- `sleep_until()`: sleep until a time point
- `yield()`: hints the scheduler to activate another thread explicitly


```cpp
#include <chrono>
#include <iomanip>  // std::put_time
#include <iostream>
#include <thread>

inline void print_time() {
    auto now = std::chrono::system_clock::now();
    auto stime = std::chrono::system_clock::to_time_t(now);
    auto ltime = std::localtime(&stime);

    std::cout << std::put_time(ltime, "%c") << '\n';
}

void func4() {
    print_time();
    std::this_thread::sleep_for(std::chrono::seconds(3));
    print_time();
}

void func5() {
    print_time();
    auto tp = std::chrono::system_clock::now() + std::chrono::seconds(3);
    std::this_thread::sleep_until(tp);
    print_time();
}

void func6(std::chrono::seconds timeout) {
    auto now = std::chrono::system_clock::now();
    auto then = now + timeout;

    size_t count = 0;
    while (std::chrono::system_clock::now() < then) {
        std::this_thread::yield();
        ++count;
    }
    // 加yield: count=1311416
    // 不加yield: count=48890582
    std::cout << "count=" << count << '\n';
}

int main() {
    // sleep_for
    {
        std::thread t(func4);
        t.join();
    }
    // sleep_until
    {
        std::thread t(func5);
        t.join();
    }
    // yield
    {
        std::thread t(func6, std::chrono::seconds(3));
        t.join();
    }
}
```

## `mutex`

example: single mutex

```cpp
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

std::mutex g_mutex;

void thread_func_1() {
    std::lock_guard<std::mutex> lock(g_mutex);
    std::cout << "running thread " << std::this_thread::get_id() << '\n';
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "done in thread " << std::this_thread::get_id() << '\n';
}  // g_mutex is released here

void thread_func_2() {
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        std::cout << "running thread " << std::this_thread::get_id() << '\n';
    }  // g_mutex is released here

    std::this_thread::yield();
    std::this_thread::sleep_for(std::chrono::seconds(3));

    {
        std::lock_guard<std::mutex> lock(g_mutex);
        std::cout << "done in thread " << std::this_thread::get_id() << '\n';
    }  // g_mutex is released here
}

void thread_func_3() {
    g_mutex.lock();
    std::cout << "running thread " << std::this_thread::get_id() << '\n';
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "done in thread " << std::this_thread::get_id() << '\n';
    g_mutex.unlock();  // g_mutex is released here
}

int main() {
    {
        std::vector<std::thread> thread_vec;
        for (int i = 0; i < 5; ++i) {
            thread_vec.emplace_back(thread_func_1);
            // thread_vec.emplace_back(thread_func_2);
            // thread_vec.emplace_back(thread_func_3);
        }

        for (auto& t : thread_vec)
            t.join();
    }
}
```

example: multiple mutexes

> `std::scope_lock`:  is a wrapper for multiple mutexes implemented in an RAII manner.(since c++17)

```cpp
#include <algorithm>  // std::erase
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

template <typename T>
struct container {
    std::mutex mtx;
    std::vector<T> data;
};

template <typename T>
void move_between(container<T>& c1, container<T>& c2, T const value) {
    std::scoped_lock lock(c1.mtx, c2.mtx);  // since c++17

    std::erase(c1.data, value);  // since c++20
    c2.data.push_back(value);
}

template <typename T>
void print_container(container<T> const& c) {
    for (auto const& e : c.data) std::cout << e << ' ';
    std::cout << '\n';
}

int main() {
    {
        container<int> c1;
        c1.data = {1, 2, 3, 4, 5, 3};

        container<int> c2;
        c2.data = {4, 5, 6, 7, 8, 6};

        print_container(c1); // 1 2 3 4 5 3 
        print_container(c2); // 4 5 6 7 8 6 

        std::thread t1(move_between<int>, std::ref(c1), std::ref(c2), 3);
        std::thread t2(move_between<int>, std::ref(c2), std::ref(c1), 6);

        t1.join();
        t2.join();

        print_container(c1); // 1 2 4 5 6 
        print_container(c2); // 4 5 7 8 3 
    }
}
```

## one writer & multi-readers

`std::shared_mutex`: used in scenarios when multiple readers can access the same resource at the same time without causing data races, while only one writer it allowed to do so. 

[official example](examples/ch09-shared-mutex.cc): <https://en.cppreference.com/w/cpp/thread/shared_mutex>

```cpp
#include <chrono>
#include <iostream>
#include <mutex>
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
        return value_;
    }

    // Only one thread/writer can increment/write the counter's value.
    void increment() {
        std::unique_lock lock(mutex_);
        ++value_;
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

    auto data_increment = [&counter]() {
        for (int i{}; i < 10; ++i) {
            counter.increment();
            std::osyncstream(std::cout) << std::this_thread::get_id() << " write\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        }
    };
    auto data_print = [&counter]() {
        int i = 0;
        while (i < 30) {
            std::osyncstream(std::cout) << std::this_thread::get_id() << " read " << counter.get() << '\n';
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            ++i;
        }
    };

    std::vector<std::thread> thread_vec;
    thread_vec.emplace_back(data_increment);
    for (unsigned i = 0; i < 3; ++i) {
        thread_vec.emplace_back(data_print);
    }

    for (auto& t : thread_vec) {
        t.join();
    }
}
```

## Handling exceptions from thread functions

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

std::mutex g_exception_mtx;
std::vector<std::exception_ptr> g_exceptions;
std::mutex g_data_mtx;
size_t G_TOTAL = 0;
const size_t N = 30;
const size_t M = N * 0.6;

void func1() {
    for (unsigned i = 0; i < N; ++i) {
        g_data_mtx.lock();
        ++G_TOTAL;
        g_data_mtx.unlock();
        std::cout << std::this_thread::get_id() << ':' << i << ',' << G_TOTAL << '\n';
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        if (i == M) {
            throw std::runtime_error("exception 1");
        }
    }
}

void func2() {
    for (unsigned i = 0; i < N; ++i) {
        g_data_mtx.lock();
        ++G_TOTAL;
        g_data_mtx.unlock();
        std::cout << std::this_thread::get_id() << ':' << i << ',' << G_TOTAL << '\n';
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        if (i == M) {
            throw std::runtime_error("exception 2");
        }
    }
}

void thread_func1() {
    try {
        func1();
    } catch (...) {
        std::lock_guard<std::mutex> lock(g_exception_mtx);
        g_exceptions.push_back(std::current_exception());
    }
}

void thread_func2() {
    try {
        func2();
    } catch (...) {
        std::lock_guard<std::mutex> lock(g_exception_mtx);
        g_exceptions.push_back(std::current_exception());
    }
}

int main() {
    g_exceptions.clear();

    std::thread t1(thread_func1);
    std::thread t2(thread_func2);
    t1.join();
    t2.join();

    for (auto const& e : g_exceptions) {
        try {
            if (e != nullptr)
                std::rethrow_exception(e);
        } catch (std::exception const& ex) {
            std::cout << ex.what() << '\n';
        }
    }
    std::cout << "G_TOTAL=" << G_TOTAL << '\n';
}
```

## `std::condition_variable`

example: simple usage of `std::conditional_variable`

```cpp
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <vector>

int main() {
    std::condition_variable cv;
    std::mutex cv_mutex;
    std::mutex io_mutex;
    int data{};

    auto func1 = [&](size_t timeout) {
        // simulate long running operation
        {
            std::this_thread::sleep_for(std::chrono::seconds(timeout));
        }
        // produce
        {
            std::unique_lock lock(cv_mutex);
            ++data;
        }
        // print msg
        {
            std::lock_guard l(io_mutex);
            std::cout << "produced " << data << '\n';
        }
        // notify
        cv.notify_one();
        // cv.notify_all();
    };
    auto func2 = [&]() {
        // wait for notification
        {
            std::unique_lock lock(cv_mutex);
            cv.wait(lock);
        }
        // consume
        {
            std::lock_guard l(io_mutex);
            std::cout << "consume " << data << '\n';
        }
    };

    const size_t N = 3;
    std::vector<std::thread> producers_vec;
    for (unsigned i = 0; i < N; ++i) {
        producers_vec.emplace_back(func1, i);
    }
    std::vector<std::thread> consumers_vec;
    for (unsigned i = 0; i < N; ++i) {
        consumers_vec.emplace_back(func2);
    }
    for (auto& t : producers_vec) t.join();
    for (auto& t : consumers_vec) t.join();
}
```

example: `std::conditional_variabbe` usage in producer-consumer model

```cpp
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <random>
#include <thread>
#include <vector>

std::mutex g_mutex;
std::condition_variable g_queuecheck;
std::queue<int> g_buffer;
std::atomic<bool> g_done(false);

void producer(int id, std::mt19937& gen, std::uniform_int_distribution<int>& dist) {
    for (int i = 0; i < 5; ++i) {
        // Simulate work
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // Generate data
        int value = id * 100 + dist(gen);
        std::cout << "id:" << id << " [produced]: " << value << '\n';

        {
            std::lock_guard<std::mutex> locker(g_mutex);
            g_buffer.push(value);
            g_queuecheck.notify_one();
        }
    }
}

void consumer() {
    while (true) {
        std::unique_lock<std::mutex> locker(g_mutex);
        if (g_buffer.empty() && !g_done) {
            g_queuecheck.wait_for(locker, std::chrono::seconds(3));
        } else if (g_buffer.empty() && g_done) {
            break;
        } else {
            std::cout << "[consumed]: " << g_buffer.front() << '\n';
            g_buffer.pop();
        }
    }
}

int main() {
    std::random_device rd{};
    auto generator = std::mt19937{rd()};
    auto distribution = std::uniform_int_distribution<>{1, 5};  // [1, 5]

    std::thread consumer1(consumer);

    std::vector<std::thread> producers;
    for (int i = 1; i < 6; i++) {
        producers.emplace_back(producer, i, std::ref(generator), std::ref(distribution));
    }
    for (auto& t : producers) t.join();

    g_done = true;

    consumer1.join();
}
```

## Using promises and futures to return values from threads

> `promise-future` pair is basically a communication channel that enables a thread
to communicate a value or exception with another thread through a shared state.

example: `promise-future` for value

```cpp
#include <chrono>
#include <future>
#include <iostream>

void produce_value(std::promise<int>& p) {
    // simulate long running operation
    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << "produce value " << '\n';
    p.set_value(100);
}

void consume_value(std::future<int>& f) {
    auto val = f.get();
    std::cout << "consume: " << val << '\n';
}

int main() {
    std::promise<int> p;
    std::thread t1(produce_value, std::ref(p));

    std::future<int> f = p.get_future();
    std::thread t2(consume_value, std::ref(f));

    t1.join();
    t2.join();
}
```

example: `promise-future` for exception

```cpp
#include <chrono>
#include <future>
#include <iostream>

void produce_value(std::promise<int>& p) {
    // simulate long running operation
    std::this_thread::sleep_for(std::chrono::seconds(3));

    try {
        throw std::runtime_error("an error has occurred!");
    } catch (...) {
        p.set_exception(std::current_exception());
    }
}

void consume_value(std::future<int>& f) {
    try {
        f.get();
    } catch (const std::exception& e) {
        std::cout << "msg=" << e.what() << '\n';  // an error has occurred!
    }
}

int main() {
    std::promise<int> p;
    std::thread t1(produce_value, std::ref(p));

    std::future<int> f = p.get_future();
    std::thread t2(consume_value, std::ref(f));

    t1.join();
    t2.join();
}
```

## `std::async` with `std::future`

> `std::thread` is low-level operations, `std::async` is high-level operations

std::async() is a variadic function template that has two overloads: one that specifies a launch policy as the first argument and another that does not. The other arguments to std::async() are the function to execute and its arguments, if any.

```cpp
#include <chrono>
#include <future>
#include <iostream>

void do_something1() {
    // simulate long running operation
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "operation 1 done" << '\n';
}
void do_something2() {
    // simulate long running operation
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "operation 2 done" << '\n';
}
int compute_value1() {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    return 100;
}
int compute_value2() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return 200;
}
int compute_value3(int i) {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    return 100 + i;
}
int compute_value4(int i) {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return 200 * i;
}

int main() {
    {
        auto f = std::async(std::launch::async, do_something1);
        do_something2();
        f.wait();
        std::cout << "all done!" << '\n';
    }
    {
        auto f = std::async(std::launch::async, compute_value1);
        auto val = compute_value2();
        val += f.get();
        std::cout << "finale result=" << val << '\n';
    }
    {
        auto f = std::async(std::launch::async, compute_value3, 10);
        auto val = compute_value4(20);
        val += f.get();
        std::cout << "finale result=" << val << '\n';
    }
    {
        auto f = std::async(std::launch::async, do_something1);
        int count = 0;
        while (true) {
            auto status = f.wait_for(std::chrono::milliseconds(500));
            if (status == std::future_status::ready) {
                break;
            }
            std::cout << "waiting " << (++count) * 500 << "ms\n";
        }
        std::cout << "done!" << '\n';
    }
}
```

## `std::atomic`

example: counter, [other examples](examples/ch09-atomic.cc)

```cpp
void test_atomic() {
    // int counter = 0; // result is error
    std::atomic<int> counter{0}; // result is correct

    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&counter]() {
            for (int i = 0; i < 1e8; ++i)
                ++counter;
        });
    }

    for (auto& t : threads) t.join();

    std::cout << counter << '\n';
}
```

## Implementing parallel `map` and `reduce`

[example1](examples/ch09-map-reduce.cc): with STL standard algorithms, differ in [policy](https://en.cppreference.com/w/cpp/algorithm/execution_policy_tag)
- `std::execution::seq`
- `std::execution::par`
- `std::execution::unseq`
- `std::execution::par_unseq`

为了使用`std::execution::par`, Debian需要
- `sudo apt install libtbb-dev`
- `g++ main.cpp --std=c++17 -ltbb && ./a.out` or change **CMakeLists.txt**

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.0.0)
project(proj1 VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(TBB REQUIRED)

add_executable(proj1 main.cpp)

target_link_libraries(proj1 PRIVATE TBB::tbb)
```

```bash
   default sum:10000000000, map cost: 38516.8ms, reduce cost: 25805.9ms, map+reduce cost: 64322.8ms
sequential sum:10000000000, map cost: 43771.2ms, reduce cost: 34368.3ms, map+reduce cost: 78139.5ms
  parallel sum:10000000000, map cost: 6021.23ms, reduce cost: 4736.1ms, map+reduce cost: 10757.3ms
unsequence sum:10000000000, map cost: 23439.7ms, reduce cost: 38640.6ms, map+reduce cost: 62080.4ms
 par+unseq sum:10000000000, map cost: 3096.2ms, reduce cost: 5242.59ms, map+reduce cost: 8338.78ms

   default sum:10000000000, map+reduce cost: 30342.7ms
sequential sum:10000000000, map+reduce cost: 31153.7ms
  parallel sum:10000000000, map+reduce cost: 4350.87ms
unsequence sum:10000000000, map+reduce cost: 33458.6ms
 par+unseq sum:10000000000, map+reduce cost: 4838.21ms
```