# Threading and Concurrency

- [Threading and Concurrency](#threading-and-concurrency)
  - [Basic concepts](#basic-concepts)
  - [Basic Usage](#basic-usage)
  - [thread sleep \& yield](#thread-sleep--yield)
  - [`mutex`](#mutex)
  - [`std::shared_mutex`](#stdshared_mutex)
  - [Handling exceptions from thread functions](#handling-exceptions-from-thread-functions)
  - [`std::condition_variable`](#stdcondition_variable)
  - [Using promises and futures to return values from threads](#using-promises-and-futures-to-return-values-from-threads)
  - [`std::async` with `std::future`](#stdasync-with-stdfuture)
  - [`std::atomic`](#stdatomic)
  - [Implementing parallel `map` and `reduce`](#implementing-parallel-map-and-reduce)
    - [with STL standard algorithms](#with-stl-standard-algorithms)
    - [with custom parallel algorithms by `std::thread`](#with-custom-parallel-algorithms-by-stdthread)
    - [with custom parallel algorithms by `std::async`](#with-custom-parallel-algorithms-by-stdasync)
  - [`std::jthread`](#stdjthread)
  - [latch, semaphore, barrier](#latch-semaphore-barrier)
  - [thread local variable](#thread-local-variable)
  - [lock\_guard vs scoped\_lock vs unique\_lock](#lock_guard-vs-scoped_lock-vs-unique_lock)
  - [custom mutex by atomic](#custom-mutex-by-atomic)
  - [atomic shared\_ptr](#atomic-shared_ptr)
  - [multithread with shared\_ptr](#multithread-with-shared_ptr)
  - [atomic\_ref](#atomic_ref)
  - [parallel policy](#parallel-policy)

## Basic concepts

线程私有数据
- 局部变量
- 函数的参数
- Thread Local Storage

线程共享数据(进程所有)
- 全局变量
- heap data
- 函数里的static变量
- 程序代码
- 打开的文件

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

## `std::shared_mutex`

allows multiple threads to simultaneously read (shared access) or exclusively write (unique access) to a shared resource.

Use std::shared_mutex when:
- **Read-Heavy Workloads**: read operations are frequent and write operations are infrequent
- **Shared Data Structures**: Multiple threads need to read shared data structures concurrently without causing data racing.
- **Performance Optimization**: You want to improve performance by allowing concurrent reads while still ensuring exclusive writes.

[cppreference example](examples/ch09-shared-mutex.cc)
- If one thread has acquired the *exclusive lock*, no other threads can acquire the lock (including the shared).
- If one thread has acquired the *shared lock*, no other thread can acquire the *exclusive lock*, but can acquire the *shared lock*.

practical example

```cpp
#include <algorithm>
#include <format>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <vector>

class SharedData {
    mutable std::shared_mutex mutex_;
    std::vector<int> data_;

   public:
    void add(int value) {
        // write operation should be exclusive, must use unique_lock
        std::unique_lock<std::shared_mutex> lock(mutex_);
        data_.push_back(value);
    }

    bool contains(int value) const {
        // read operation could be concurrent, can use shared_lock
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return std::find(data_.begin(), data_.end(), value) != data_.end();
    }

    void print() const {
        // read operation could be concurrent, can use shared_lock
        std::shared_lock<std::shared_mutex> lock(mutex_);
        for (const auto& val : data_) {
            std::cout << val << " ";
        }
        std::cout << "\n";
    }
};

int main() {
    SharedData sharedData;

    // Writer thread
    std::jthread writer([&sharedData] {
        for (int i = 0; i < 10; ++i) {
            sharedData.add(i);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    // Reader threads
    std::vector<std::jthread> readers;
    for (int i = 0; i < 3; ++i) {
        readers.emplace_back([&sharedData, i] {
            for (int j = 0; j < 10; ++j) {
                if (sharedData.contains(j)) {
                    std::cout << std::format("Reader {} found {}\n", i, j);
                } else {
                    std::cout << std::format("Reader {} didn't find {}\n", i, j);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    sharedData.print();
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

### with STL standard algorithms

[example1](examples/ch09-map-reduce.cc): with STL standard algorithms, differ in [policy](https://en.cppreference.com/w/cpp/algorithm/execution_policy_tag)
- `std::execution::seq`
- `std::execution::par`
- `std::execution::unseq`
- `std::execution::par_unseq`

为了使用`std::execution::par`, Debian需要如下配置。MSVC不需要任何配置，也不需要修改**CMakeLists.txt**
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

### with custom parallel algorithms by `std::thread`

[example2](examples/ch09-thread-mapreduce.cc): with custom parallel algorithms by `std::thread`

```bash
   default sum:10000000000, map cost: 42791.2ms, reduce cost: 30494.6ms, map+reduce cost: 73285.9ms
  parallel sum:10000000000, map cost: 5520.65ms, reduce cost: 3663.02ms, map+reduce cost: 9183.66ms
```

### with custom parallel algorithms by `std::async`

[example3](examples/ch09-async-mapreduce.cc): with custom parallel algorithms by `std::aysnc`

```bash
   default sum:10000000000, map cost: 41210ms, reduce cost: 35599ms, map+reduce cost: 76809ms
  parallel sum:10000000000, map cost: 5613.03ms, reduce cost: 2478.61ms, map+reduce cost: 8091.65ms
```

## `std::jthread`

> `std::jthread`: since C++20,  joinable thread, NO NEED explicitly invoke the `join()` method to wait for the thread to finish
execution

```cpp
#include <iostream>
#include <thread>

void thread_func1(int i) {
    while (i-- > 0) {
        std::cout << i << '\n';
    }
}

int main() {
    std::jthread t(thread_func1, 10);
}
```

example: cancel the execution of thread by `std::stock_source`

```cpp
#include <chrono>
#include <iostream>
#include <thread>

void thread_func2(std::stop_token st, int& i) {
    while (!st.stop_requested() && i < 100) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        i++;
    }
}

int main() {
    // example1: cancel the execution of a thread
    {
        int a = 0;
        std::jthread t(thread_func2, std::ref(a));
        std::this_thread::sleep_for(std::chrono::seconds(1));
        t.request_stop();
        std::cout << a << '\n';  // 4
    }
    // example2: cancel the execution of multi-threads
    {
        int a = 0;
        int b = 10;

        std::stop_source ss;
        std::jthread t1(thread_func2, ss.get_token(), std::ref(a));
        std::jthread t2(thread_func2, ss.get_token(), std::ref(b));

        std::this_thread::sleep_for(std::chrono::seconds(1));

        ss.request_stop();  // stop all with stop_source

        std::cout << a << ' ' << b << '\n';  // 4 14
    }
    // example3: cancel the execution of a thread with callback
    {
        int a = 0;

        std::stop_source ss;
        std::stop_token st = ss.get_token();
        std::stop_callback cb(st, [&] { std::cout << a << "invoke the callback\n"; });

        std::jthread t(thread_func2, st, std::ref(a));

        std::this_thread::sleep_for(std::chrono::seconds(1));

        ss.request_stop();

        std::cout << a << '\n';  // prints "4invoke the callback" and 4
    }
}
```

## latch, semaphore, barrier

> `std::latch`: Use `std::latch` when you need threads to wait until a counter, decreased
by other threads, reaches zero.

```cpp
#include <iostream>
#include <latch>
#include <thread>
#include <vector>

void process(std::vector<int> const& data) {
    for (auto const e : data) std::cout << e << ' ';
    std::cout << '\n';
}

int main() {
    // latch
    int const NUM = 4;
    std::latch work_done(NUM);
    std::vector<int> data(NUM);
    std::vector<std::jthread> threads;  // or std::thread
    for (int i = 1; i <= NUM; ++i) {
        threads.push_back(std::jthread([&data, i, &work_done] {
            std::this_thread::sleep_for(std::chrono::seconds(1));  // simulate work

            data[i] = i * 2;  // create data

            work_done.count_down();  // decrement counter
        }));
    }
    work_done.wait();  // wait for all jobs to finish(work_done count down to 0)
    process(data);     // process data from all jobs
}
```

> `std::semaphore`: Use `std::counting_semaphore<N>` when you want to restrict a number of N threadsto access a shared resource

```cpp
#include <iostream>
#include <semaphore>
#include <thread>
#include <vector>

void process(std::vector<int> const& data) {
    for (auto const e : data) std::cout << e << ' ';
    std::cout << '\n';
}

int main() {
    // semaphore
    int const NUM = 4;
    std::vector<int> data;
    std::vector<std::jthread> threads;  // or std::thread
    std::counting_semaphore sem{1};

    for (int i = 1; i <= NUM; ++i) {
        threads.push_back(std::jthread([&data, i, &sem] {
            for (int k = 1; k < 3; ++k) {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));  // simulate work
                int value = i * k;                                            // create data

                sem.acquire();  // acquire the semaphore

                data.push_back(value);  // write to the shared resource
                std::cout << "===>(" << i << ',' << k << "):";
                process(data);  // print data

                sem.release();  // release the semaphore
            }
        }));
    }

    for (auto& t : threads) t.join();
    process(data);  // process data from all jobs
}
```

> `std::barrier`: Threads arrive at the barrier, decrease the internal counter, and block. 

```cpp
#include <barrier>
#include <iostream>
#include <thread>
#include <vector>

void process(std::vector<int> const& data) {
    for (auto const e : data) std::cout << e << ' ';
    std::cout << '\n';
}

int main() {
    // barrier
    int const NUM = 4;
    std::vector<int> data(NUM);
    // callback is optional
    auto callback = [&data] { process(data); }; // invoke at arrive_and_wait()
    std::barrier barr(NUM, callback);

    std::vector<std::thread> threads;
    for (int i = 0; i < NUM; i++) {
        threads.emplace_back([&data, &barr](int const i) {
            barr.arrive_and_wait();
            std::cout << "thread-" << i << " starts working" << '\n';

            std::this_thread::sleep_for(std::chrono::seconds(1));
            data[i] = i * 10;
            std::cout << "thread-" << i << " finish working" << '\n';

            barr.arrive_and_wait();  // Workers synchronize here
            std::cout << "thread-" << i << " done" << '\n';
        },
                             i);
    }
    for (auto& t : threads) t.join();
    process(data);
}
```

complicated example of `std::barrier`

```cpp
#include <barrier>
#include <iostream>
#include <thread>
#include <vector>

void process(std::vector<int> const& data) {
    for (auto const e : data) std::cout << e << ' ';
    std::cout << '\n';
}

int main() {
    int const NUM = 4;
    std::vector<int> data(NUM);
    int cycle = 1;

    std::stop_source st;

    std::barrier work_done(
        NUM,
        [&data, &cycle, &st]() {
            process(data);
            cycle++;
            if (cycle == 10)
                st.request_stop();
        });

    std::vector<std::jthread> threads;
    for (int i = 0; i < NUM; ++i) {  // Change loop initialization from 1 to 0
        threads.emplace_back(
            [&cycle, &work_done, &data](std::stop_token st, int i) {  // Add &data as a parameter capture
                while (!st.stop_requested()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));

                    data[i] = i * cycle;
                    work_done.arrive_and_wait();
                }
            },
            st.get_token(),  // Pass stop_token explicitly
            i                // Pass i explicitly
        );
    }

    for (auto& t : threads) t.join();
}
```

## thread local variable

`thread_local` define variables that are unique to each thread. This means every thread has its own separate instance of the variable, ensuring thread safety without the need for explicit synchronization mechanisms like mutexes.
- `thread_local` can be applied to *global variables*, *namespace-scope variables*, and *static local variables within functions*.
- Variables with `thread_local` storage duration are initialized when a thread starts and destroyed when the thread ends.

simple example

```cpp
#include <format>
#include <iostream>
#include <thread>

// thread_local for global variable
thread_local int global_value = 100;

void increment_and_print(int id) {
    global_value += id;
    std::cout << std::format("thread-{} value={}\n", id, global_value);
}

int main() {
    std::jthread t1(increment_and_print, 10);
    std::jthread t2(increment_and_print, 20);
}
```

```cpp
#include <format>
#include <iostream>
#include <thread>

void increment_and_print(int id) {
    // thread_local for static local variable
    thread_local static int func_counter = 1000;
    func_counter += id;
    std::cout << std::format("thread-{} value={}\n", id, func_counter);
}

int main() {
    std::jthread t1(increment_and_print, 10);
    std::jthread t2(increment_and_print, 20);
}
```

```cpp
#include <format>
#include <iostream>
#include <thread>

namespace sp1 {
// thread_local for namespace-scope variable
thread_local int nsp_counter = 200;

void increment_and_print(int id) {
    nsp_counter += id;
    std::cout << std::format("thread-{} value={}\n", id, nsp_counter);
}
}  // namespace sp1

int main() {
    std::jthread t1(sp1::increment_and_print, 10);
    std::jthread t2(sp1::increment_and_print, 20);
}
```

practical example: Per-Thread Logging

```cpp
#include <format>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

struct Logger {
    Logger() { std::cout << "logger init\n"; }
    ~Logger() { std::cout << "logger destroy\n"; }
    void log(const std::string& message) {
        std::ostringstream oss;
        oss << std::this_thread::get_id();
        std::cout << std::format("Thread-{}:{}\n", oss.str(), message);
    }
};

thread_local Logger thread_logger;

void log_messages() {
    thread_logger.log("Starting thread ");
    thread_logger.log("Ending thread ");
}

int main() {
    std::vector<std::jthread> threads;
    for (size_t i = 0; i < 3; ++i) {
        threads.emplace_back(log_messages);
    }
}
```

Best Practices and Considerations
- Minimize Usage: Overuse of thread_local can lead to increased memory consumption, especially if many threads are created.
- Frequent creation and destruction of threads with thread_local variables can impact performance due to repeated initialization and destruction.
- Understand when to use `static` (shared across threads) versus `thread_local` (unique per thread) storage.

## lock_guard vs scoped_lock vs unique_lock

Best Practices:
- Prefer `std::lock_guard` for simple, single-mutex scenarios.
- Use `std::unique_lock` when you need advanced locking features.
- Utilize `std::scoped_lock` when dealing with multiple mutexes to avoid deadlocks effortlessly.

| Feature                | `std::lock_guard`                         | `std::unique_lock`                                   | `std::scoped_lock`                               |
|------------------------|-------------------------------------------|-----------------------------------------------------|--------------------------------------------------|
| **Mutexes Managed**    | Single mutex                              | Single mutex                                        | Multiple mutexes                                 |
| **Flexibility**        | Minimal (lock on construction)            | High (defer, try, manual lock/unlock)               | Minimal (locks all provided mutexes at once)     |
| **Movable/Copyable**   | Neither                                   | Movable but not copyable                            | Neither                                           |
| **Locking Strategies** | Immediate lock                            | Immediate, deferred, try-lock, adopt-lock            | Simultaneous locking of multiple mutexes         |
| **Use Cases**          | Simple scope-based locking                | Complex synchronization, condition variables        | Locking multiple mutexes safely and easily       |
| **Overhead**           | Lowest                                     | Slightly higher due to added flexibility            | Comparable to `std::lock_guard` for multiple locks|

```cpp
// basic usage
#include <cassert>
#include <iostream>
#include <mutex>
#include <thread>

auto counter = 0;  // Counter will be protected by counter_mutex
auto counter_mutex = std::mutex{};

void increment_counter1(int n) {
    for (int i = 0; i < n; ++i) {
        std::lock_guard<std::mutex> lock{counter_mutex};
        ++counter;
    }
}
void increment_counter2(int n) {
    for (int i = 0; i < n; ++i) {
        std::unique_lock<std::mutex> lock{counter_mutex};
        ++counter;
    }
}

void increment_counter3(int n) {
    for (int i = 0; i < n; ++i) {
        std::scoped_lock<std::mutex> lock{counter_mutex};
        ++counter;
    }
}

int main() {
    constexpr auto n = int{10'000'000};
    {
        auto t1 = std::jthread{increment_counter1, n};
        auto t2 = std::jthread{increment_counter1, n};
    }
    std::cout << counter << '\n';
    assert(counter == (n * 2));

    {
        auto t1 = std::jthread{increment_counter2, n};
        auto t2 = std::jthread{increment_counter2, n};
    }
    std::cout << counter << '\n';
    assert(counter == (n * 4));

    {
        auto t1 = std::jthread{increment_counter3, n};
        auto t2 = std::jthread{increment_counter3, n};
    }
    std::cout << counter << '\n';
    assert(counter == (n * 6));
}
```

example: multiple mutex

```cpp
struct Account {
    Account() {}
    int balance_{0};
    std::mutex m_{};
};

// by unique_lock
void transfer_money(Account& from, Account& to, int amount) {
    auto lock1 = std::unique_lock<std::mutex>{from.m_, std::defer_lock};
    auto lock2 = std::unique_lock<std::mutex>{to.m_, std::defer_lock};
    // Lock both unique_locks at the same time
    std::lock(lock1, lock2);
    from.balance_ -= amount;
    to.balance_ += amount;
}

// recomended, by scoped_lock
void transfer_money2(Account& from, Account& to, int amount) {
    // Lock both mutexes simultaneously using std::scoped_lock
    std::scoped_lock lock(from.m_, to.m_);

    // Perform the transfer
    from.balance_ -= amount;
    to.balance_ += amount;
}
```

## custom mutex by atomic

How It Works:
1. Lock Acquisition (`lock` method): 
   1. A thread calling `lock()` will attempt to set the `is_locked_` flag.
   2. If the flag was previously `false` (unlocked), `test_and_set()` sets it to `true` and the thread acquires the lock. the `lock()` method will return immediately.
   3. If the flag was already `true` (locked by another thread), the thread enters a busy-wait loop, repeatedly checking the flag until it becomes false.
   4. Once the flag is cleared, the thread tries to set it again to acquire the lock.
2. Lock Release (`unlock` method): 
   1. The thread holding the lock calls `unlock()`, which clears the `is_locked_` flag.
   2. Clearing the flag signals other waiting threads that the lock is now available.

[some ref](https://timur.audio/using-locks-in-real-time-audio-processing-safely)

```cpp
#include <atomic>
#include <iostream>
#include <thread>

class SimpleMutex {
    std::atomic_flag is_locked_{};  // Cleared by default
   public:
    auto lock() noexcept {
        while (is_locked_.test_and_set(std::memory_order_acquire)) {  // Attempt to set the flag
            while (is_locked_.test(std::memory_order_relaxed));       // Busy-wait (spin) until the flag is cleared
        }
    }

    auto unlock() noexcept {
        is_locked_.clear();  // Release the lock by clearing the flag
    }
};

SimpleMutex mutex;
int shared_resource = 0;

void increment() {
    for (int i = 0; i < 1000000; ++i) {
        mutex.lock();
        ++shared_resource;
        mutex.unlock();
    }
}

int main() {
    std::thread t1(increment);
    std::thread t2(increment);

    t1.join();
    t2.join();

    std::cout << "Final value: " << shared_resource << std::endl;
}
```

## atomic shared_ptr

`std::atomic<std::shared_ptr<T>>` is introduced in C++20. 
> The `std::atomic<std::shared_ptr<T>>` ensures that operations on the shared_ptr **itself** (like *loading* and *storing* pointers) are atomic. 

```cpp
#include <atomic>
#include <memory>
#include <print>
#include <thread>

struct MyData {
    int value;
    MyData(int v) : value(v) {}
};

int main() {
    std::atomic<std::shared_ptr<MyData>> atomicPtr{std::make_shared<MyData>(42)};
    std::println("ptr is lock-free: {}", atomicPtr.is_lock_free());  // false

    // Thread 1 - reads the value
    std::jthread reader([&atomicPtr] {
        while (true) {
            std::shared_ptr<MyData> localPtr = atomicPtr.load(std::memory_order_acquire);
            if (localPtr) {
                std::println("reader read {}", localPtr->value);
            } else {
                std::println("reader read nullptr");
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });

    // Thread 2 - updates the value
    std::jthread writer([&atomicPtr] {
        for (int i = 0; i < 10; ++i) {
            auto newPtr = std::make_shared<MyData>(i);
            atomicPtr.store(newPtr, std::memory_order_release);
            // auto oldPtr = atomicPtr.exchange(std::make_shared<MyData>(i), std::memory_order_release);

            std::println("writer write {}", newPtr->value);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    });
}
```

The `is_lock_free()` method returning `false` indicates that the atomic operations on `atomicPtr` are implemented using locks internally rather than being lock-free. Despite not being lock-free, these atomic operations still provide the necessary synchronization to prevent data races on the pointer itself.

While the atomicity of `std::atomic<std::shared_ptr<T>>` ensures that the pointer itself is managed safely, it doesn't protect the *pointed-to object* from concurrent access.

protect the `pointed-to object` by atomic

```cpp
#include <atomic>
#include <chrono>
#include <memory>
#include <print>
#include <thread>

struct MyData {
    std::atomic<int> value;  // Make the member atomic
    MyData(int v) : value(v) {}
};

int main() {
    // Initialize atomicPtr with a shared_ptr to MyData
    std::atomic<std::shared_ptr<MyData>> atomicPtr{std::make_shared<MyData>(42)};
    std::println("ptr is lock-free: {}", atomicPtr.is_lock_free());  // false

    // Thread 1 - reads the value
    std::jthread reader([&atomicPtr] {
        while (true) {
            std::shared_ptr<MyData> localPtr = atomicPtr.load(std::memory_order_acquire);
            if (localPtr) {
                int currentValue = localPtr->value.load(std::memory_order_acquire);
                std::println("reader read {}", currentValue);
            } else {
                std::println("reader read nullptr");
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });

    // Thread 2 - updates the value
    std::jthread writer([&atomicPtr] {
        for (int i = 0; i < 10; ++i) {
            std::shared_ptr<MyData> localPtr = atomicPtr.load(std::memory_order_acquire);
            if (localPtr) {
                localPtr->value.store(i + 1, std::memory_order_release);
                std::println("writer modified value to {}", i + 1);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    });
}
```

## multithread with shared_ptr

- shared_ptr should be captured by value
- shared_ptr caputred by reference if risky

```cpp
#include <memory>
#include <print>
#include <thread>

#include "spsc.hpp"

int main(int argc, char const *argv[]) {
    auto ptr = std::make_shared<lockfree::SPSC<int, 16>>();

    std::jthread producer{[ptr] {
        for (auto i = 0; i < 10; ++i) {
            while (!ptr->push(i)) {
                std::println("full, cannot push");
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }};

    std::jthread consumer{[ptr] {
        while (true) {
            std::optional<int> value;
            while (!(value = ptr->pop())) {
                std::println("empty, cannot pop");
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            std::println("consumer got {}", value.value());
        }
    }};
}
```

## atomic_ref

It's not possible to write `std::atomic<T&>`, but we can use `std::atomic_ref<T>` since C++20
> perform atomic operations on existing non-atomic objects without changing its original type to an atomic type by by creating an atomic reference to them. 

```cpp
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

struct ExpensiveToCopy {
    int counter{};
};

void count(ExpensiveToCopy& exp) {
    std::vector<std::thread> v;
    std::atomic_ref<int> counter{exp.counter};

    for (int n = 0; n < 10; ++n) {
        v.emplace_back([&counter] {
            for (int i = 0; i < 1000; ++i) {
                ++counter;
            }
        });
    }

    for (auto& t : v) t.join();
}

int main() {
    ExpensiveToCopy exp;
    count(exp);
    std::cout << "exp.counter: " << exp.counter << '\n';
}
```

why not define atomic in the struct?
- Each access to the counter is synchronized, and synchronization is not for free. On the contrary, using a `std::atomic_ref<int>` counter lets you explicitly control when you need atomic access to the counter. Most of the time, you may only want to read the value of the counter. Consequently, defining it as an atomic is pessimization.
- not modify the existing struct

```cpp
struct ExpensiveToCopy {
    std::atomic<int> counter{};
};
```

## parallel policy

`std::accumulate` vs `std::reduce`
> when the provided algorithm is commutative, `std::reduce` is preferred.

```cpp
#include <execution>
#include <functional>
#include <numeric>
#include <print>
#include <vector>

int main(int argc, char const *argv[]) {
    std::vector<int> v{1, 2, 3, 4}; // sum & product is commutative
    {
        // std::accumulate cannot be parallelized
        auto sum = std::accumulate(v.begin(), v.end(), 0, std::plus<int>{});
        std::println("sum={}", sum);
        auto product = std::accumulate(v.begin(), v.end(), 1, std::multiplies<int>{});
        std::println("product={}", product);
    }
    {
        // std::reduce can be parallelized, but the order of vector not matter
        auto sum = std::reduce(std::execution::par, v.begin(), v.end(), 0, std::plus<int>{});
        std::println("sum={}", sum);
        auto product = std::reduce(std::execution::par, v.begin(), v.end(), 1, std::multiplies<int>{});
        std::println("product={}", product);
    }

    std::vector<std::string> v2{"A", "B", "C", "D"}; // string concat is not commutative
    {
        // std::accumulate cannot be parallelized
        auto concat = std::accumulate(v2.begin(), v2.end(), std::string{}, std::plus<std::string>{});
        std::println("concat={}", concat); // ABCD
    }
    {
        // std::reduce can be parallelized, but the order of vector not matter
        auto concat = std::reduce(std::execution::par, v2.begin(), v2.end(), std::string{}, std::plus<std::string>{});
        std::println("concat={}", concat); // maybe ABCD, ACDB, ....
    }
}
```

there is the same problem in `std::for_each` like `std::accumulate`
> By only accessing vector elements via the unique index i, we avoid introducing data races when mutating the strings in the vector, so we can use `std::execution::par`

```cpp
#include <execution>
#include <print>
#include <ranges>
#include <vector>

int main(int argc, char const *argv[]) {
    auto v = std::vector<std::string>{"A", "B", "C"};
    auto r = std::views::iota(size_t{0}, v.size());
    std::for_each(std::execution::par, r.begin(), r.end(), [&v](size_t i) {
        v[i] += std::to_string(i + 1);
    });

    for (auto &&e : v) {
        std::println("{}", e);
    }
}
```