# Threading and Concurrency

- [Threading and Concurrency](#threading-and-concurrency)
  - [Basic Usage](#basic-usage)

## Basic Usage

```cpp
#include <chrono>
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
        std::cout << b << '\n';
    }
}
```