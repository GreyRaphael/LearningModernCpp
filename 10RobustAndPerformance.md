# Robustness and Performance

- [Robustness and Performance](#robustness-and-performance)
  - [exception](#exception)
  - [`const`](#const)
  - [`unique_ptr`](#unique_ptr)

## exception

custom exception

```cpp
#include <iostream>
#include <sstream>

class advanced_error : public std::runtime_error {
    int error_code;

    std::string make_message(int const e) {
        std::stringstream ss;
        ss << "error with code " << e;
        return ss.str();
    }

   public:
    advanced_error(int const e) : std::runtime_error(make_message(e).c_str()), error_code(e) {
    }
};

int main() {
    {
        try {
            throw advanced_error(110);
        } catch (advanced_error const& e) {
            std::cout << e.what() << '\n';
        }
    }
}
```

> `noexcept` 编译期完成声明和检查工作.`noexcept` 主要是解决的问题是减少运行时开销. 运行时开销指的是, 编译器需要为代码生成一些额外的代码用来包裹原始代码，当出现异常时可以抛出一些相关的堆栈stack unwinding错误信息, 这里面包含，错误位置, 错误原因, 调用顺序和层级路径等信息.当使用`noexcept`声明一个函数不会抛出异常候, 编译器就不会去生成这些额外的代码, 直接的减小的生成文件的大小, 间接的优化了程序运行效率.
1. 当`noexcept` 是标识符时, 它的作用是在函数后面声明一个函数是否会抛出异常.
2. 当`noexcept` 是函数时, 它的作用是检查一个函数是否会抛出异常.

where `noexcept` recommendation
- destructor
- move constructor
- move assignment

## `const`

`T const` == `const T`

> Placing the const keyword after the type is more natural because it is consistent with the reading direction, from right to left. 

example: `const` memeber function with `mutable`

```cpp
#include <algorithm>  // std::find
#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <vector>

class computation {
    double compute_value(double const input) const {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        return input + 100;
    }

    // mutable用于突破compute() const中的const
    mutable std::map<double, double> cache;

   public:
    double compute(double const input) const {
        auto it = cache.find(input);
        if (it != cache.end()) return it->second;

        auto result = compute_value(input);
        cache[input] = result;

        return result;
    }
};

template <typename T>
class container {
    std::vector<T> data;
    mutable std::mutex mt;

   public:
    void add(T const& value) {
        std::lock_guard<std::mutex> lock(mt);
        data.push_back(value);
    }

    bool contains(T const& value) const {
        std::lock_guard<std::mutex> lock(mt);
        return std::find(std::begin(data), std::end(data), value) != std::end(data);
    }
};

int main() {
    {
        computation const c;
        std::cout << c.compute(6) << '\n';  // 106
        // 如果已经存在，直接访问cache
        std::cout << c.compute(6) << '\n';  // 106
    }
    {
        container<int> c;
        c.add(100);
        c.add(200);
        std::cout << c.contains(100) << '\n';  // 1
    }
}
```

## `unique_ptr`

simple example

```cpp
#include <iostream>
#include <memory>
#include <vector>

class foo {
    double b;
    std::string c;

   public:
    int a;
    foo(int const a = 10, double const b = 10.1, std::string const& c = "hello") : a(a), b(b), c(c) {}

    void print() const {
        std::cout << '(' << a << ',' << b << ',' << c << ')' << '\n';
    }
};

struct foo_deleter {
    void operator()(foo* pf) const {
        std::cout << "deleting foo..." << '\n';
        delete pf;
    }
};

struct Base {
    virtual ~Base() {
        std::cout << "~Base()" << '\n';
    }
};

struct Derived : public Base {
    virtual ~Derived() {
        std::cout << "~Derived()" << '\n';
    }
};

int main(int, char**) {
    // unique_ptr with std::move
    {
        auto pi = std::make_unique<int>(24);
        auto qi = std::move(pi);
        std::cout << (pi.get() == nullptr) << '\n';  // true
        std::cout << *(qi.get()) << '\n';            // 24
    }
    // dereff unique_ptr
    {
        auto pf1 = std::make_unique<foo>();
        pf1->print();
        (*pf1).print();  // equivalent to above

        auto pf2 = std::make_unique<foo>(2, 3.2, "world");
        pf2->print();
        std::cout << pf2->a << '\n';
    }
    // unique_ptr with std::move
    {
        std::vector<std::unique_ptr<foo>> data;

        for (unsigned i = 0; i < 5; ++i) {
            data.push_back(std::make_unique<foo>(i, i * 10.0, std::to_string(i * 2)));
        }
        auto pf = std::make_unique<foo>(23, 23.0, "grey");
        data.push_back(std::move(pf));
        for (auto const& p : data) p->print();
    }
    // custom unique_ptr deleter
    {
        std::unique_ptr<foo, foo_deleter> pf(new foo(42, 42.0, "42"), foo_deleter());
    }
    // unique_ptr with heirachy
    {
        std::unique_ptr<Derived> pd = std::make_unique<Derived>();
        std::unique_ptr<Base> pb = std::move(pd);
    }
}
```