# C++20 Concepts

- [C++20 Concepts](#c20-concepts)
  - [Simple constraint by Concepts](#simple-constraint-by-concepts)
  - [Concepts `requires` expr](#concepts-requires-expr)

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

## Concepts `requires` expr

example: parameter type `T` of function `myadd()` must be `Addable`, which means `operator+` must be overloaded for the `T` template argument

```cpp
#include <iostream>

template <typename T>
concept Addable = requires(T x, T y) { x + y; };

template <Addable T>
T myadd(T x, T y) {
    return x + y;
}

class Foo {
    int val_;

   public:
    Foo(int const v) : val_(v) {
    }
};

class Bar {
   private:
    int val_;

   public:
    Bar(int const v) : val_(v) {}
    Bar operator+(Bar const& other) {
        Bar temp{0};
        temp.val_ = other.val_ + val_;
        return temp;
    }
    void print() {
        std::cout << val_ << '\n';
    }
};

int main() {
    auto v1 = myadd(1, 2);
    auto v2 = myadd(10.1, 20.2);
    auto v3 = myadd(std::string{"hello"}, std::string{"world"});
    std::cout << v3 << '\n';  // helloworld
    // auto v4 = myadd(Foo{2}, Foo{3}); // error
    auto v5 = myadd(Bar{10}, Bar{20});
    v5.print();  // 30
}
```

example: paramter type `T` of function `do_wrap()` must be `Wrapable`, which means `T` must be `Numerical`

```cpp
#include <iostream>

template <typename T>
concept Numerical = std::is_arithmetic_v<T>;

template <Numerical T>
struct Num {
    T value;
};

template <Numerical T>
Num<T> wrap(T value) {
    return {value};
}

template <Numerical T>
T unwrap(Num<T> s) {
    return s.value;
}

template <typename T>
concept Wrapable = requires(T x) { wrap(x); };

template <Wrapable T>
auto do_wrap(T x) {
    return wrap(x);
}

int main() {
    auto v1 = do_wrap(1);
    auto v2 = do_wrap(10.1);
    // auto v3 = do_wrap(std::string{"hello"});// not arithmetic_v
}
```

example: paramter type `T` of function `pass_container` must be `Container`
- `typename T::value_type;` states that `T` must have a nested type called `value_type`  
- `typename T::iterator;` states that `T` must have a nested type called `iterator`
- `begin(x);` checks if the expression `begin(x)` is valid
- `end(x);` checks if the expression `end(x)` is valid

Overall, the `Container` concept ensures that a type `T` satisfies the requirements necessary to be used as a container. It checks if `T` has the nested types `value_type` and `iterator`, and if it has valid `begin()` and `end()` member functions.

```cpp
#include <array>
#include <deque>
#include <iostream>
#include <vector>

template <typename T>
concept Container = requires(T x) {
    typename T::value_type;
    typename T::iterator;
    begin(x);
    end(x);
};

template <Container T>
void pass_container(T const& c) {
    for (auto const& x : c) std::cout << x << '\t';
    std::cout << '\n';
}

int main() {
    int arr[4]{4, 3, 2, 1};
    // pass_container(arr); // error, not Container

    std::array<int, 4> a1{10, 20, 30, 40};
    pass_container(a1);

    std::vector<double> v1{1, 2, 3, 4, 5.1};
    pass_container(v1);

    std::deque<int> d1{11, 22, 33, 44};
    pass_container(d1);
}
```