# Performance

- [Performance](#performance)
  - [`std::function` overhead](#stdfunction-overhead)

## `std::function` overhead

1. `std::function` is about 10% slower than lambda
2. `std::function` as callback in slower than normal 

`std::function` flexible but comes with some overhead. 
- Type Erasure: uses type erasure to store any callable object that matches its signature. This means it abstracts away the concrete type of the callable, which introduces an additional layer of indirection when invoking the callable.
- Dynamic Memory Allocation: might allocate memory on the heap to store the callable object(like lambdas, function pointers, or functors), which can introduce latency.
- Indirection: involves an indirect call through a virtual table or a similar mechanism, preventing certain compiler optimizations like inlining.

```cpp
#include <chrono>
#include <functional>
#include <iostream>
#include <vector>

inline constexpr int myadd(int x) { return x + 1; }

int main() {
    const int iterations = 500000000;
    std::vector<int> data(iterations, 1);
    std::cout << "iterations: " << iterations << '\n';

    // Using std::function
    std::function<int(int)> func = [](int x) { return x + 1; };
    auto start_func = std::chrono::high_resolution_clock::now();
    int sum_func = 0;
    for (auto x : data) {
        sum_func += func(x);
    }
    auto end_func = std::chrono::high_resolution_clock::now();

    // Using direct lambda
    auto lambda = [](int x) { return x + 1; };
    auto start_lambda = std::chrono::high_resolution_clock::now();
    int sum_lambda = 0;
    for (auto x : data) {
        sum_lambda += lambda(x);
    }
    auto end_lambda = std::chrono::high_resolution_clock::now();

    // Using direct function
    auto start_direct = std::chrono::high_resolution_clock::now();
    int sum_direct = 0;
    for (auto x : data) {
        sum_direct += myadd(x);
    }
    auto end_direct = std::chrono::high_resolution_clock::now();

    std::cout << "std::function sum: " << sum_func << " avg Time: "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end_func - start_func).count() / static_cast<double>(iterations)
              << "us\n";
    std::cout << "Lambda sum: " << sum_lambda << " avg Time: "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end_lambda - start_lambda).count() / static_cast<double>(iterations)
              << "us\n";
    std::cout << "Direct sum: " << sum_lambda << " avg Time: "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end_direct - start_direct).count() / static_cast<double>(iterations)
              << "us\n";
}
```