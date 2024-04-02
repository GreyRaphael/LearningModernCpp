# Modern C++ datetime

- [Modern C++ datetime](#modern-c-datetime)
  - [chrono](#chrono)
    - [`strftime` \& `strptime`](#strftime--strptime)
    - [Measure exection time](#measure-exection-time)

## chrono

simple usage of `<chrono>`

```cpp
#include <chrono>
#include <iostream>

int main() {
    using namespace std::chrono_literals;
    std::cout << (12345s).count() << '\n';  // 12345

    // auto total_seconds = 12345s;  // simple way
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

    std::chrono::duration<double, std::ratio<1>> seconds1{20};
    std::chrono::duration<double, std::ratio<10>> seconds2{20};
    std::cout << std::chrono::duration_cast<std::chrono::seconds>(seconds1) << '\n';  // 20s
    std::cout << std::chrono::duration_cast<std::chrono::seconds>(seconds2) << '\n';  // 200s
}
```

### `strftime` & `strptime`

in C++17, `now` to string

```cpp
#include <chrono>
#include <iomanip>
#include <iostream>

int main() {
    // 获取当前时间点
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

    // 将时间点转换为time_t类型
    std::time_t current_time = std::chrono::system_clock::to_time_t(now);

    // 获取毫秒和微秒部分
    auto duration = now.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration) % 1000;
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration) % 1000;

    // 将time_t类型格式化为字符串
    std::tm* time_info = std::localtime(&current_time);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", time_info);

    // 构造时间字符串
    std::stringstream time_string_stream;
    time_string_stream << buffer << '.' << std::setw(3) << std::setfill('0') << milliseconds.count() << std::setw(3) << std::setfill('0') << microseconds.count();
    // std::string time_string = time_string_stream.str();
    auto time_string = time_string_stream.view(); // since c++20

    // 打印结果
    std::cout << time_string << std::endl;
}
```

in C++17, `time_point` to string, string to `time_point`

```cpp
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

std::tm strptime(const std::string& datetime, const std::string& format) {
    std::istringstream ss(datetime);
    std::tm tm = {};

    ss >> std::get_time(&tm, format.c_str());

    if (ss.fail()) {
        // Handle parsing error here
        // For simplicity, just throwing an exception
        throw std::runtime_error("Failed to parse datetime");
    }

    return tm;
}

std::chrono::system_clock::time_point convertToTimePoint(const std::string& datetime, const std::string& format) {
    std::tm tm = strptime(datetime, format);

    std::time_t t = std::mktime(&tm);
    std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(t);

    return tp;
}

int main() {
    std::string datetime = "2023-07-26 12:30:15";
    std::string format = "%Y-%m-%d %H:%M:%S";

    try {
        std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
        // std::chrono::system_clock::time_point tp = convertToTimePoint(datetime, format);

        // Print the converted time point
        std::time_t tt = std::chrono::system_clock::to_time_t(tp);
        std::cout << std::put_time(std::localtime(&tt), "%Y-%m-%d %H:%M:%S") << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}
```

### Measure exection time

simple example

```cpp
#include <chrono>
#include <iostream>

void task(int const N = 100000000) {
    for (int i = 0; i < N; ++i);
}

int main() {
    auto start_tp = std::chrono::high_resolution_clock::now();
    task();
    auto diff = std::chrono::high_resolution_clock::now() - start_tp;
    std::cout << std::chrono::duration<double, std::milli>(diff).count() << "milliseconds\n";  // 469.718milliseconds
    std::cout << std::chrono::duration<double, std::nano>(diff) << "\n";                       // 4.69718e+08ns
}
```

wrap into a struct

```cpp
#include <chrono>
#include <functional>  // std::invoke
#include <iostream>

void task(int const N = 100000000) {
    for (int i = 0; i < N; ++i)
        ;
}

struct perf_timer {
    template <typename F, typename... Args>
    static std::chrono::microseconds duration(F&& f, Args... args) {
        auto start = std::chrono::high_resolution_clock::now();
        std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    }
};

int main() {
    auto start_tp = std::chrono::high_resolution_clock::now();
    auto d = perf_timer::duration(task, 100000000);
    std::cout << d << '\n';                                             // 485352µs
    std::cout << std::chrono::duration<double, std::milli>(d) << '\n';  // 485.352ms
}
```

wrap with template

```cpp
#include <chrono>
#include <functional>  // std::invoke
#include <iostream>

void task(int const N = 100000000) {
    for (int i = 0; i < N; ++i);
}

// easy to change duration units & clock type
template <typename Time = std::chrono::microseconds, typename Clock = std::chrono::high_resolution_clock>
struct perf_timer {
    template <typename F, typename... Args>
    static Time duration(F&& f, Args... args) {
        auto start = Clock::now();
        std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
        auto end = Clock::now();
        return std::chrono::duration_cast<Time>(end - start);
    }
};

int main() {
    auto start_tp = std::chrono::high_resolution_clock::now();
    auto d = perf_timer<>::duration(task, 100000000);
    std::cout << d << '\n';                                             // 485352µs
    std::cout << std::chrono::duration<double, std::milli>(d) << '\n';  // 485.352ms
}
```