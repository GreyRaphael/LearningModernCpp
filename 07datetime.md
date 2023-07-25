# Modern C++ datetime

- [Modern C++ datetime](#modern-c-datetime)
  - [chrono](#chrono)

## chrono

simple usage of `<chrono>`

```cpp
#include <chrono>
#include <iostream>

int main() {
    using namespace std::chrono_literals;

    // auto total_seconds = 12345s; // simple way
    std::chrono::seconds total_seconds{12345};
    
    // low precision-> high precision
    std::chrono::milliseconds total_milliseconds{total_seconds};
    std::cout << total_milliseconds.count() << '\n';  // 12345000

    // high precision-> low precision
    auto hours = std::chrono::duration_cast<std::chrono::hours>(total_seconds);
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(total_seconds % 1h);
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(total_seconds % 1min);
    // std::cout << hours << ':' << minutes << ':' << seconds << '\n'; // 3h:25min:45s
    std::cout << hours.count() << ':' << minutes.count() << ':' << seconds.count() << '\n';  // 3:25:45

    // time compare
    auto d1 = 1h + 23min + 45s;
    auto d2 = 3h + 12min + 50s;
    std::cout << (d1 < d2) << '\n';  // 1
}
```