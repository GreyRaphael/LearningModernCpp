# Threading and Concurrency

- [Threading and Concurrency](#threading-and-concurrency)
  - [Basic Usage](#basic-usage)
  - [thread sleep \& yield](#thread-sleep--yield)
  - [`mutex`](#mutex)
  - [Handling exceptions from thread functions](#handling-exceptions-from-thread-functions)

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
        std::vector<std::thread> thead_vec;
        for (int i = 0; i < 5; ++i) {
            thead_vec.emplace_back(thread_func_1);
            // thead_vec.emplace_back(thread_func_2);
            // thead_vec.emplace_back(thread_func_3);
        }

        for (auto& t : thead_vec)
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