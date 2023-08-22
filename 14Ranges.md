# C++20 Ranges

- [C++20 Ranges](#c20-ranges)

simple usage of `ranges`

```cpp
#include <iostream>
#include <ranges>

int main() {
    // [1, 10)
    for (auto i : std::ranges::views::iota(1, 10)) {
        std::cout << i << ' ';
    }
    std::cout << '\n';
}
```

ranges with pipeline `|`

```cpp
#include <cmath>
#include <iostream>
#include <ranges>

bool is_prime(int const n) {
    if (n != 2) {
        if (n < 2 || n % 2 == 0) return false;
        auto root = std::sqrt(n);
        for (int i = 3; i <= root; i += 2) {
            if (n % i == 0) return false;
        }
    }
    return true;
}

int main() {
    namespace rv = std::ranges::views;
    for (auto i : rv::iota(1, 10) | rv::filter(is_prime)) {
        std::cout << i << ' ';
    }  // 2 3 5 7
    std::cout << '\n';

    auto v = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    // for (auto i : rv::all(v) | rv::filter(is_prime)) { // method1
    for (auto i : v | rv::filter(is_prime)) {
        std::cout << i << ' ';
    }  // 2 3 5 7
    std::cout << '\n';

    for (auto i : rv::iota(1, 10) | rv::filter(is_prime) | rv::transform([](int const n) { return n + 1; })) {
        std::cout << i << ' ';
    }  // 3 4 6 8
    std::cout << '\n';

    // take(): take the first-N elements
    // drop(): drop the first-N elements
    for (auto i : rv::iota(1, 10) | rv::reverse | rv::filter(is_prime) | rv::take(3) | rv::reverse | rv::drop(1)) {
        std::cout << i << ' ';
    }  // 5 7
    std::cout << '\n';
}
```

ranges with algorithms

```cpp
#include <algorithm>  // max,sort,reverse,copy, count_if...
#include <iostream>
#include <iterator>  // std::ostream_iterator
#include <ranges>
#include <vector>

int main() {
    namespace rg = std::ranges;
    // algorithms
    std::vector<int> v{5, 2, 7, 1, 4, 2, 9, 5};
    std::cout << rg::max(v) << '\n';  // 9

    rg::sort(v);
    for (auto& e : v) std::cout << e << ' ';  // 1 2 2 4 5 5 7 9
    std::cout << '\n';

    rg::reverse(v);
    rg::copy(v, std::ostream_iterator<int>(std::cout, ","));  // 9,7,5,5,4,2,2,1,
    std::cout << '\n';

    auto n = rg::count_if(v, [](int const n) { return n % 2 == 0; });
    std::cout << n << '\n';  // 3
}
```


C++17 use [ranges](https://github.com/ericniebler/range-v3)

```cpp
#ifdef USE_RANGES_V3
// use in C++17
#include "range/v3/view.hpp"
#include "range/v3/algorithm/sort.hpp"
#include "range/v3/algorithm/copy.hpp"
#include "range/v3/algorithm/reverse.hpp"
#include "range/v3/algorithm/count_if.hpp"
#include "range/v3/algorithm/max.hpp"

namespace rv = ranges::views;
namespace rg = ranges;

template<class T>
struct ranges_ostream_iterator : std::ostream_iterator<T>
{
   using std::ostream_iterator<T>::ostream_iterator;
   ranges_ostream_iterator() : std::ostream_iterator<T>{ std::cout } {}
   ranges_ostream_iterator& operator++() { std::ostream_iterator<T>::operator++(); return *this; }
   ranges_ostream_iterator& operator++(int) { return operator++(); }
   using difference_type = int;
   int operator-(ranges_ostream_iterator const&) { return 0; }
};

#define OSTREAM_ITERATOR ranges_ostream_iterator

#else
// use in C++20

#include <ranges>
namespace rv = std::ranges::views;
namespace rg = std::ranges;

#define OSTREAM_ITERATOR std::ostream_iterator

#endif
```

