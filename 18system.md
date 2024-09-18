# System

- [System](#system)
  - [shared library and static library](#shared-library-and-static-library)
  - [memory order](#memory-order)

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

