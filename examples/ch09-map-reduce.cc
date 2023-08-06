#include <algorithm>  // std::transform
#include <chrono>
#include <execution>  // std::execution::par
#include <iostream>
#include <numeric>  // std::reduce
#include <vector>

template <typename Time = std::chrono::microseconds,
          typename Clock = std::chrono::high_resolution_clock>
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
    std::vector<unsigned char> v(5e9, 1);
    // // compare sequential, parallel map & reduce
    {
        auto v0 = v;
        auto s0 = 0LL;

        auto t01 = perf_timer<>::duration([&] { std::transform(
                                                    std::begin(v0), std::end(v0),
                                                    std::begin(v0),
                                                    [](unsigned char const i) { return i + i; }); });
        auto t02 = perf_timer<>::duration([&] { s0 = std::reduce(
                                                    std::begin(v0), std::end(v0),
                                                    0LL,
                                                    std::plus<>()); });
        std::cout << "   default sum:" << s0
                  << ", map cost: " << std::chrono::duration<double, std::milli>(t01)
                  << ", reduce cost: " << std::chrono::duration<double, std::milli>(t02)
                  << ", map+reduce cost: " << std::chrono::duration<double, std::milli>(t01 + t02) << '\n';
    }
    {
        auto v1 = v;
        auto s1 = 0LL;

        auto t11 = perf_timer<>::duration([&] { std::transform(
                                                    std::execution::seq,
                                                    std::begin(v1), std::end(v1),
                                                    std::begin(v1),
                                                    [](unsigned char const i) { return i + i; }); });
        auto t12 = perf_timer<>::duration([&] { s1 = std::reduce(
                                                    std::execution::seq,
                                                    std::begin(v1), std::end(v1),
                                                    0LL,
                                                    std::plus<>()); });
        std::cout << "sequential sum:" << s1
                  << ", map cost: " << std::chrono::duration<double, std::milli>(t11)
                  << ", reduce cost: " << std::chrono::duration<double, std::milli>(t12)
                  << ", map+reduce cost: " << std::chrono::duration<double, std::milli>(t11 + t12) << '\n';
    }
    {
        auto v2 = v;
        auto s2 = 0LL;
        auto t21 = perf_timer<>::duration([&] { std::transform(
                                                    std::execution::par,
                                                    std::begin(v2), std::end(v2),
                                                    std::begin(v2),
                                                    [](unsigned char const i) { return i + i; }); });
        auto t22 = perf_timer<>::duration([&] { s2 = std::reduce(
                                                    std::execution::par,
                                                    std::begin(v2), std::end(v2),
                                                    0LL,
                                                    std::plus<>()); });
        std::cout << "  parallel sum:" << s2
                  << ", map cost: " << std::chrono::duration<double, std::milli>(t21)
                  << ", reduce cost: " << std::chrono::duration<double, std::milli>(t22)
                  << ", map+reduce cost: " << std::chrono::duration<double, std::milli>(t21 + t22) << '\n';
    }
    {
        auto v3 = v;
        auto s3 = 0LL;
        auto t31 = perf_timer<>::duration([&] { std::transform(
                                                    std::execution::unseq,
                                                    std::begin(v3), std::end(v3),
                                                    std::begin(v3),
                                                    [](unsigned char const i) { return i + i; }); });
        auto t32 = perf_timer<>::duration([&] { s3 = std::reduce(
                                                    std::execution::unseq,
                                                    std::begin(v3), std::end(v3),
                                                    0LL,
                                                    std::plus<>()); });
        std::cout << "unsequence sum:" << s3
                  << ", map cost: " << std::chrono::duration<double, std::milli>(t31)
                  << ", reduce cost: " << std::chrono::duration<double, std::milli>(t32)
                  << ", map+reduce cost: " << std::chrono::duration<double, std::milli>(t31 + t32) << '\n';
    }
    {
        auto v4 = v;
        auto s4 = 0LL;
        auto t41 = perf_timer<>::duration([&] { std::transform(
                                                    std::execution::par_unseq,
                                                    std::begin(v4), std::end(v4),
                                                    std::begin(v4),
                                                    [](unsigned char const i) { return i + i; }); });
        auto t42 = perf_timer<>::duration([&] { s4 = std::reduce(
                                                    std::execution::par_unseq,
                                                    std::begin(v4), std::end(v4),
                                                    0LL,
                                                    std::plus<>()); });
        std::cout << " par+unseq sum:" << s4
                  << ", map cost: " << std::chrono::duration<double, std::milli>(t41)
                  << ", reduce cost: " << std::chrono::duration<double, std::milli>(t42)
                  << ", map+reduce cost: " << std::chrono::duration<double, std::milli>(t41 + t42) << '\n';
    }
    // use stl map - reduce sequential& parallel
    {
        auto v5 = v;
        auto s5 = 0LL;

        auto t5 = perf_timer<>::duration([&] { s5 = std::transform_reduce(
                                                   std::begin(v5), std::end(v5),
                                                   0LL,
                                                   std::plus<>(),
                                                   [](unsigned char const i) { return i + i; }); });

        std::cout << "   default sum:" << s5
                  << ", map+reduce cost: " << std::chrono::duration<double, std::milli>(t5) << '\n';
    }
    {
        auto v6 = v;
        auto s6 = 0LL;

        auto t6 = perf_timer<>::duration([&] { s6 = std::transform_reduce(
                                                   std::execution::seq,
                                                   std::begin(v6), std::end(v6),
                                                   0LL,
                                                   std::plus<>(),
                                                   [](unsigned char const i) { return i + i; }); });
        std::cout << "sequential sum:" << s6
                  << ", map+reduce cost: " << std::chrono::duration<double, std::milli>(t6) << '\n';
    }
    {
        auto v7 = v;
        auto s7 = 0LL;

        auto t7 = perf_timer<>::duration([&] { s7 = std::transform_reduce(
                                                   std::execution::par,
                                                   std::begin(v7), std::end(v7),
                                                   0LL,
                                                   std::plus<>(),
                                                   [](unsigned char const i) { return i + i; }); });
        std::cout << "  parallel sum:" << s7
                  << ", map+reduce cost: " << std::chrono::duration<double, std::milli>(t7) << '\n';
    }
    {
        auto v8 = v;
        auto s8 = 0LL;

        auto t8 = perf_timer<>::duration([&] { s8 = std::transform_reduce(
                                                   std::execution::unseq,
                                                   std::begin(v8), std::end(v8),
                                                   0LL,
                                                   std::plus<>(),
                                                   [](unsigned char const i) { return i + i; }); });
        std::cout << "unsequence sum:" << s8
                  << ", map+reduce cost: " << std::chrono::duration<double, std::milli>(t8) << '\n';
    }
    {
        auto v9 = v;
        auto s9 = 0LL;

        auto t9 = perf_timer<>::duration([&] { s9 = std::transform_reduce(
                                                   std::execution::par_unseq,
                                                   std::begin(v9), std::end(v9),
                                                   0LL,
                                                   std::plus<>(),
                                                   [](unsigned char const i) { return i + i; }); });

        std::cout << " par+unseq sum:" << s9
                  << ", map+reduce cost: " << std::chrono::duration<double, std::milli>(t9) << '\n';
    }
}