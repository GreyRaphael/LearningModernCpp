#include <algorithm>  // std::transform
#include <chrono>
#include <functional>  // std::invoke
#include <iostream>
#include <numeric>  // std::reduce
#include <thread>
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

template <typename Iter, typename F>
void parallel_map(Iter begin, Iter end, F f) {
    auto size = std::distance(begin, end);

    if (size <= 1e4)
        std::transform(begin, end, begin, std::forward<F>(f));
    else {
        auto core_nums = std::thread::hardware_concurrency();
        auto part = size / core_nums;
        auto last = begin;

        std::vector<std::thread> threads;
        for (unsigned i = 0; i < core_nums; ++i) {
            if (i == core_nums - 1)
                last = end;
            else
                std::advance(last, part);

            threads.emplace_back([=, &f] { std::transform(begin, last, begin, std::forward<F>(f)); });

            begin = last;
        }

        for (auto& t : threads) t.join();
    }
}

template <typename Iter, typename R, typename F>
auto parallel_reduce(Iter begin, Iter end, R init, F op) {
    auto size = std::distance(begin, end);

    if (size <= 1e4)
        return std::reduce(begin, end, init, std::forward<F>(op));
    else {
        auto core_nums = std::thread::hardware_concurrency();
        auto part = size / core_nums;
        auto last = begin;

        std::vector<std::thread> threads;
        std::vector<R> values(core_nums);
        for (unsigned i = 0; i < core_nums; ++i) {
            if (i == core_nums - 1)
                last = end;
            else
                std::advance(last, part);

            threads.emplace_back([=, &op](R& result) { result = std::reduce(begin, last, R{}, std::forward<F>(op)); }, std::ref(values[i]));

            begin = last;
        }

        for (auto& t : threads) t.join();

        return std::reduce(std::begin(values), std::end(values), init, std::forward<F>(op));
    }
}

int main() {
    std::vector<unsigned char> v(5e9, 1);

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

        auto t11 = perf_timer<>::duration([&] { parallel_map(
                                                    std::begin(v1), std::end(v1),
                                                    [](unsigned char const i) { return i + i; }); });
        auto t12 = perf_timer<>::duration([&] { s1 = parallel_reduce(
                                                    std::begin(v1), std::end(v1),
                                                    0LL,
                                                    std::plus<>()); });
        std::cout << "  parallel sum:" << s1
                  << ", map cost: " << std::chrono::duration<double, std::milli>(t11)
                  << ", reduce cost: " << std::chrono::duration<double, std::milli>(t12)
                  << ", map+reduce cost: " << std::chrono::duration<double, std::milli>(t11 + t12) << '\n';
    }
}