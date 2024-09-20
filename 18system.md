# System

- [System](#system)
  - [shared library and static library](#shared-library-and-static-library)
  - [memory order](#memory-order)
    - [memory\_order\_relaxed](#memory_order_relaxed)
    - [memory\_order\_release and memory\_order\_acquire](#memory_order_release-and-memory_order_acquire)

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
2. It prevents prior reads/writes in the current thread from being reordered after the release operation.

`memory_order_acquire`:
1. Ensures that all memory reads and writes after the acquire operation are not moved before it.
2. It prevents subsequent reads/writes in the current thread from being reordered before the acquire operation.

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

For SPMC mode, you should choose LockFreeRingBuffer mechanism