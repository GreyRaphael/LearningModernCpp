# Threading and Concurrency

- [Threading and Concurrency](#threading-and-concurrency)
  - [Basic Usage](#basic-usage)
  - [thread sleep \& yield](#thread-sleep--yield)

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