# System

- [System](#system)
  - [shared library and static library](#shared-library-and-static-library)
  - [memory order](#memory-order)
    - [memory\_order\_relaxed](#memory_order_relaxed)
    - [memory\_order\_release and memory\_order\_acquire](#memory_order_release-and-memory_order_acquire)
    - [`memory_order_seq_cst`](#memory_order_seq_cst)

## shared library and static library

notes:
- dynamic library is about 5% slower than static library
- Explicit-runtime linking
  - Linux: `dlopen`
  - Windows: `LoadLibrary`

in linux usage:

```cpp
#include <iostream>
#include <dlfcn.h>

int main() {
    // Open the shared library
    void* handle = dlopen("libmylibrary.so", RTLD_LAZY);
    if (!handle) {
        std::cerr << "Cannot open library: " << dlerror() << '\n';
        return 1;
    }

    // Load the symbol (function)
    typedef void (*hello_t)();
    dlerror(); // Reset errors
    hello_t hello = (hello_t) dlsym(handle, "hello");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "Cannot load symbol 'hello': " << dlsym_error << '\n';
        dlclose(handle);
        return 1;
    }

    // Use the function
    hello();

    // Close the library
    dlclose(handle);
    return 0;
}
```

in windows usage:

```cpp
#include <iostream>
#include <windows.h>

int main() {
    // Load the DLL
    HMODULE hLib = LoadLibrary("mylibrary.dll");
    if (!hLib) {
        std::cerr << "Cannot load library: " << GetLastError() << '\n';
        return 1;
    }

    // Load the symbol (function)
    typedef void (*hello_t)();
    hello_t hello = (hello_t) GetProcAddress(hLib, "hello");
    if (!hello) {
        std::cerr << "Cannot load symbol 'hello': " << GetLastError() << '\n';
        FreeLibrary(hLib);
        return 1;
    }

    // Use the function
    hello();

    // Free the DLL
    FreeLibrary(hLib);
    return 0;
}
```

## memory order

[volatile tutorial](https://blog.csdn.net/sinat_38293503/article/details/134710547)
- before C++11, there is no memory order, just `volatile`;
- after C++11, `memory_order_seq_cst` is better

[tutorial](https://www.cooldoger.com/2020/09/what-is-memory-order-in-c-therere-six.html)

|                    | relaxed | acquire | release | acq_rel | seq_cst |
|--------------------|---------|---------|---------|---------|---------|
| load               | Y       | Y       | N       | N       | Y       |
| store              | Y       | N       | Y       | N       | Y       |
| exchange           | Y       | Y       | Y       | Y       | Y       |
| compare_exchange_* | Y       | Y       | Y       | Y       | Y       |
| fetch_*            | Y       | Y       | Y       | Y       | Y       |
| fence              | Y       | Y       | Y       | Y       | Y       |

### memory_order_relaxed

use cases:
- Suitable for scenarios where you don't need to enforce any ordering constraints between operations, but you still require atomicity.
- **simple counters**(e.g. counter in `shared_ptr`) or **flags** where the exact order of operations across threads doesn't matter.

```cpp
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

std::atomic<int> cnt = {0};

void f() {
    for (int n = 0; n < 1e8; ++n) {
        cnt.fetch_add(1, std::memory_order_relaxed); // fetch_add is atomic operation
    }
}

int main() {
    std::vector<std::thread> v;
    for (int n = 0; n < 10; ++n) {
        v.emplace_back(f);
    }
    for (auto& t : v) {
        t.join();
    }
    std::cout << "Final counter value is " << cnt << '\n';  // 1000000000
}
```

### memory_order_release and memory_order_acquire

`memory_order_release`:
1. Ensures that all memory reads and writes before the release operation are not moved after it.
2. but memory reads and writes after the release can be moved before it.

`memory_order_acquire`:
1. Ensures that all memory reads and writes after the acquire operation are not moved before it.
2. but memory reads and writes before the acquire can be moved after it.

`memory_order_acq_rel`
1. Combines the effects of both acquire and release semantics. Prevents reordering of previous memory operations after the release, Prevents reordering of subsequent memory operations before the acquire.
2. Used in read-modify-write(RMW) operations (like `fetch_add`, `fetch_sub`) when you need both to acquire and release.
3. behave like: `acquire` -> `RMW` -> `release`

example: atomic SPSC without `wait`

why use `std::memory_order_relaxed` in consumer thread
- reset `data_ready` to `false`: not need to synchronize any data with the producer, can make `flag` to `false` avoiding repeated consuming `data`
- Using `memory_order_relaxed` avoids the overhead of unnecessary memory fences or barriers, which can improve performance

```cpp
#include <atomic>
#include <chrono>
#include <format>
#include <iostream>
#include <thread>

std::atomic<bool> data_ready(false);
int data = 0;

void producer() {
    while (true) {
        // Produce data
        ++data;
        std::cout << std::format("produce data: {}\n", data);

        data_ready.store(true, std::memory_order_release);
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
}

void consumer() {
    while (true) {
        auto flag = data_ready.load(std::memory_order_acquire);
        if (flag) {
            // Consume data
            std::cout << std::format("consume data: {}\n", data);

            // Reset the data_ready flag
            data_ready.store(false, std::memory_order_relaxed);
        } else {
            std::cout << "consumer wait data\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

int main() {
    std::jthread producer_thread(producer);
    std::jthread consumer_thread(consumer);

    std::this_thread::sleep_for(std::chrono::seconds(5));
    producer_thread.detach();
    consumer_thread.detach();
    std::cout << "Main thread exiting.\n";
}
```

atomic SPSC with data_ready syncronization back to producer, `acquire-release` mode
- producer `data_ready.store(true, std::memory_order_release)` *happen before* consumer `auto flag = data_ready.load(std::memory_order_acquire)`
- consumer `data_ready.store(false, std::memory_order_release)` *happen before* producer `while (data_ready.load(std::memory_order_acquire))`

```cpp
void producer() {
    while (true) {
        // Wait until data_ready is false
        while (data_ready.load(std::memory_order_acquire)) {
            std::this_thread::yield();  // you can avoid busy waiting by sleep_for
        }

        // Produce data
        ++data;
        std::cout << std::format("produce data: {}\n", data);

        data_ready.store(true, std::memory_order_release);
        // no need explicitly wait
    }
}

void consumer() {
    while (true) {
        auto flag = data_ready.load(std::memory_order_acquire);
        if (flag) {
            // Consume data
            std::cout << std::format("consume data: {}\n", data);

            // Reset the data_ready flag with release semantics
            data_ready.store(false, std::memory_order_release);
        } else {
            std::cout << "consumer wait data\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}
```

```cpp
// Wait until data_ready is false
while (data_ready.load(std::memory_order_acquire)) {
    std::this_thread::yield();
}

// interesting, the above can be replaced by the following, not recommended
data_ready.wait(true, std::memory_order_acquire);
```

example: atomic SPSC with `wait`

```cpp
#include <atomic>
#include <chrono>
#include <format>
#include <iostream>
#include <thread>

// initial flag is false
std::atomic<bool> data_ready(false);
int data = 0;

void producer() {
    while (true) {
        // Wait until data_ready is false before producing new data
        while (data_ready.load(std::memory_order_acquire)) {
            // the while-loop is for spurious wakeup
            data_ready.wait(true, std::memory_order_acquire);
        }

        // Produce data
        ++data;

        // Signal the consumer that data is ready
        data_ready.store(true, std::memory_order_release);
        // Notify the consumer
        data_ready.notify_one();
    }
}

void consumer() {
    while (true) {
        // Wait until data_ready becomes true
        while (!data_ready.load(std::memory_order_acquire)) {
            // the while-loop is for spurious wakeup
            data_ready.wait(false, std::memory_order_acquire);
        }

        // Consume data
        std::cout << std::format("consume data: {}\n", data);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Reset the data_ready flag
        data_ready.store(false, std::memory_order_release);
        // Notify the producer
        data_ready.notify_one();
    }
}

int main() {
    std::jthread producer_thread(producer);
    std::jthread consumer_thread(consumer);

    std::this_thread::sleep_for(std::chrono::seconds(5));
    producer_thread.detach();
    consumer_thread.detach();
    std::cout << "Main thread exiting.\n";
}
```

For SPMC mode, you should choose LockFreeRingBuffer mechanism, like [atomic_queue](https://github.com/max0x7ba/atomic_queue)

SPMC with `atomic` by versioning mechanism
- producer, `consumers_left.store(NUM_CONSUMERS, std::memory_order_release);` *happen before* the `acquire` in `consumers_left.fetch_sub(1, std::memory_order_acq_rel)`
- consumer, the `release` in `consumers_left.fetch_sub(1, std::memory_order_acq_rel)` *happen before* the `consumers_left.load(std::memory_order_acquire);`
- producer, `data_version.fetch_add(1, std::memory_order_release);` *happen before* `data_version.load(std::memory_order_acquire)`

why it should use `consumers_left.fetch_sub(1, std::memory_order_acq_rel)` instead of `memory_order_relaxed`, `memory_order_release`, `memory_order_acquire`
- syncronization with producer, so it cannot be `memory_order_relaxed`
- consume data must before `consumers_left.fetch_sub`, so it cannot be `memory_order_acquire`
- notify must after `consumers_left.fetch_sub`, so it cannot be `memory_order_release`

```cpp
#include <atomic>
#include <chrono>
#include <format>
#include <iostream>
#include <thread>
#include <vector>

// increments every time the producer produces new data
std::atomic<int> data_version(0);
std::atomic<int> consumers_left(0);
int data = 0;
const int NUM_CONSUMERS = 3;  // Number of consumers

void producer() {
    while (true) {
        // Wait until all consumers have consumed the previous data
        int left = consumers_left.load(std::memory_order_acquire);
        while (left != 0) {
            consumers_left.wait(left, std::memory_order_acquire);
            left = consumers_left.load(std::memory_order_acquire);
        }

        // Produce data
        ++data;
        std::cout << std::format("Producer: produced data {}\n", data);

        // Set the number of consumers left
        consumers_left.store(NUM_CONSUMERS, std::memory_order_release);

        // Increment the data_version
        data_version.fetch_add(1, std::memory_order_release);

        // Notify all consumers
        data_version.notify_all();
    }
}

void consumer(int id) {
    // Each consumer keeps track of the last data version it has consumed
    int last_consumed_version = 0;

    while (true) {
        // Wait until data_version > last_consumed_version
        int current_version = data_version.load(std::memory_order_acquire);
        while (current_version == last_consumed_version) {
            data_version.wait(current_version, std::memory_order_acquire);
            current_version = data_version.load(std::memory_order_acquire);
        }

        // Consume data
        std::cout << std::format("Consumer {}: consumed data {}\n", id, data);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Update last_consumed_version
        last_consumed_version = current_version;

        // Decrement the consumers_left counter
        int left = consumers_left.fetch_sub(1, std::memory_order_acq_rel);
        if (left == 1) {
            // This was the last consumer
            // Notify the producer
            consumers_left.notify_one();
        }
    }
}

int main() {
    std::jthread prod_thread(producer);

    std::vector<std::thread> consumer_threads;
    for (int i = 0; i < NUM_CONSUMERS; ++i) {
        consumer_threads.emplace_back(consumer, i + 1);
    }

    for (auto& t : consumer_threads) {
        t.join();
    }
}
```

error example: when consumer2 meet `letf==1`, consumer2 notify the  producer, consumer1 which  meet `left!=1`  will continue consume the data twice, so you must use the above `versioning mechanism`

```cpp
#include <atomic>
#include <chrono>
#include <format>
#include <iostream>
#include <thread>
#include <vector>

std::atomic<bool> data_ready(false);
std::atomic<int> consumers_left(0);
int data = 0;
const int NUM_CONSUMERS = 2;  // Number of consumers

void producer() {
    while (true) {
        // Wait until data_ready is false before producing new data
        while (data_ready.load(std::memory_order_acquire)) {
            data_ready.wait(true, std::memory_order_acquire);
        }

        // Produce data
        ++data;
        std::cout << std::format("Producer: produced data {}\n", data);

        // Set the number of consumers left
        consumers_left.store(NUM_CONSUMERS, std::memory_order_release);

        // Signal the consumers that data is ready
        data_ready.store(true, std::memory_order_release);
        // Notify all consumers
        data_ready.notify_all();
    }
}

void consumer(int id) {
    while (true) {
        // Wait until data_ready becomes true
        while (!data_ready.load(std::memory_order_acquire)) {
            data_ready.wait(false, std::memory_order_acquire);
        }

        // Consume data
        std::cout << std::format("Consumer {}: consumed data {}\n", id, data);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Decrement the consumers_left counter
        int left = consumers_left.fetch_sub(1, std::memory_order_acq_rel);
        if (left == 1) {
            // This was the last consumer
            // Reset the data_ready flag
            data_ready.store(false, std::memory_order_release);
            // Notify the producer
            data_ready.notify_one();
        } else {
            std::cout << std::format("consumer{} skip, left={}\n", id, left);
        }
    }
}
```

### `memory_order_seq_cst`

`memory_order_seq_cst`: Provides a total sequential ordering of operations across all threads,  All threads see memory operations in the same order.
- [ref1](https://www.codedump.info/post/20191214-cxx11-memory-model-1/)
- [ref2](https://jamesbornholt.com/blog/memory-models/)

sequential consistency requires
1. 每个处理器的执行顺序和代码中的顺序（program order）一样。
2. 所有处理器都只能看到一个单一的操作执行顺序。

```cpp
#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>

std::atomic<int> x{0}, y{0};
int r1 = 0, r2 = 0;

void thread1() {
    x.store(1, std::memory_order_seq_cst); // A
    r1 = y.load(std::memory_order_seq_cst); // B
    std::cout << "r1: " << r1 << '\n';
}

void thread2() {
    y.store(1, std::memory_order_seq_cst); // C
    r2 = x.load(std::memory_order_seq_cst); // D
    std::cout << "r2: " << r2 << '\n';
}

int main() {
    std::jthread t1(thread1);
    std::jthread t2(thread2);

    // With sequential consistency, at least one of the loads must see the other thread's store.
    assert(!(r1 == 0 && r2 == 0));  // This assertion should always hold.
}
```

1. 一个处理器内的执行顺序必须按照程序顺序(program order). 
   - A *happen before* B
   - C *happen before* D
2. 如果出现`(r1 == 0 && r2 == 0)` 意味着
   - B *happen before* C
   - D *happen before* A
3. 最终形成 A → B → C → D → A ...，违背sequential consistency

以IM中的群聊消息作为例子说明顺序一致性的这两个要求。在这个例子中，群聊中的每个成员，相当于多核编程中的一个处理器，那么对照顺序一致性的两个要求就是：
- 每个人自己发出去的消息，必然是和ta说话的顺序一致的。即用户A在群聊中依次说了消息1和消息2，在群聊天的时候也必然是先看到消息1然后再看到消息2，这就是前面顺序一致性的第一个要求。
- 群聊中有多个用户参与聊天（多处理器），如果所有人看到的消息顺序都一样，那么就满足了前面顺序一致性的第二个要求了，但是这个顺序首先不能违背前面的第一个要求。