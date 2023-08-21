# C++20 Concepts

- [C++20 Concepts](#c20-concepts)
  - [Simple constraint by Concepts](#simple-constraint-by-concepts)

> A concept is a named set of constraints and a constraint is a requirement for a template argument. 

> These are used to select the appropriate function overloads and template specializations.

## Simple constraint by Concepts

simple example

```cpp
#include <iostream>

namespace v1 {
// without concepts
template <typename T, typename = typename std::enable_if_t<std::is_arithmetic_v<T>, T>>
T myadd(T x, T y) {
    return x + y;
}
}  // namespace v1

namespace v2 {
// with concepts
template <typename T>
concept Numerical = std::is_arithmetic_v<T>;

template <Numerical T>
T myadd(T x, T y) {
    return x + y;
}
}  // namespace v2

int main() {
    {
        // without concepts
        auto v1 = v1::myadd(1, 2);
        auto v2 = v1::myadd(11.1, 22.2);
        // auto v3 = myadd(std::string{"hello"}, std::string{"world"});  // error
    }
    {
        // with concepts
        auto v1 = v2::myadd(1, 2);
        auto v2 = v2::myadd(11.1, 22.2);
        // auto v3 = myadd(std::string{"hello"}, std::string{"world"});  // error
    }
}
```

concept conjection: `&&`

```cpp
#include <iostream>

struct IComparableToInt {
    virtual bool CompareTo(int const o) = 0;
};

struct IConvertibleToInt {
    virtual int ConvertTo() = 0;
};

template <typename T>
struct SmartNumericalValue : public IComparableToInt, IConvertibleToInt {
    T value;

    SmartNumericalValue(T v) : value(v) {}

    bool CompareTo(int const o) override { return static_cast<int>(value) == o; }
    int ConvertTo() override { return static_cast<int>(value); }
};

template <typename T>
struct DullNumericalValue : public IConvertibleToInt {
    T value;

    DullNumericalValue(T v) : value(v) {}

    int ConvertTo() override { return static_cast<int>(value); }
};

template <class T>
concept IntComparable = std::is_base_of_v<IComparableToInt, T>;

template <class T>
concept IntConvertible = std::is_base_of_v<IConvertibleToInt, T>;

template <class T>
concept IntComparableAndConvertible = IntComparable<T> && IntConvertible<T>;

template <IntComparableAndConvertible T>
void print(T o) {
    std::cout << o.value << '\n';
}

int main() {
    SmartNumericalValue v1{100.2};
    DullNumericalValue v2{200.3};
    print(v1);  // 100.2
    // print(v2);// error, v2 isn't IntComparableAndConvertible
}
```

concept disjunction: `||`

```cpp
template <typename T>
concept Numerical = std::integral<T> || std::floating_point<T>;
```