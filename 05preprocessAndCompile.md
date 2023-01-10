# Preprocessing and Compilation

- [Preprocessing and Compilation](#preprocessing-and-compilation)
  - [Conditional Compilaton](#conditional-compilaton)

## Conditional Compilaton

```cpp
#ifndef I2SWAP_CUSTOMCONVERTER_H
#define I2SWAP_CUSTOMCONVERTER_H

#include <string>
#include <cstring>

#ifdef _WIN32
    #include "windows.h"
#elif __linux__
    #include "iconv.h"
#endif
```

