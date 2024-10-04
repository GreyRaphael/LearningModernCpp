# Robustness and Performance

- [Robustness and Performance](#robustness-and-performance)
  - [exception](#exception)
  - [`const`](#const)
  - [`dtor` and `ctor`](#dtor-and-ctor)
  - [`unique_ptr`](#unique_ptr)
  - [`shared_ptr`](#shared_ptr)
  - [custom deleter for `unique_ptr` and `shared_ptr`](#custom-deleter-for-unique_ptr-and-shared_ptr)
  - [`move` semantic](#move-semantic)
  - [operator `<=>`](#operator-)
  - [`std::move`](#stdmove)
  - [`std::function` overhead](#stdfunction-overhead)
  - [check default paramter](#check-default-paramter)
  - [`weak_ptr`](#weak_ptr)
  - [lazy evaluation](#lazy-evaluation)
    - [pipe operator](#pipe-operator)
  - [Trivially Copyable Types](#trivially-copyable-types)

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

## `dtor` and `ctor`

simple example to demo `dtor` and `ctor`

```cpp
#include <iostream>

class Foo {
   public:
    int val = 0;

    Foo() = default;                                  // default ctor
    Foo(Foo &&) = default;                            // move ctor
    Foo(const Foo &) = default;                       // copy ctor
    Foo &operator=(Foo &&) = default;                 // move assiginment
    Foo &operator=(const Foo &) = default;            // copy assignment
    ~Foo() { std::cout << "~Foo()" << val << '\n'; }  // dtor
    Foo(int value) : val(value) {}                    // custom ctor
};

int main() {
    {
        Foo f1;
        f1 = Foo{10};  // copy assignment
        std::cout << "rvalue going to die" << '\n';
        f1.val = 100;
    }
    // ~Foo()10
    // rvalue going to die
    // ~Foo()100
    {
        Foo f2;
        f2 = std::move(Foo{20});  // move assignment
        std::cout << "rvalue going to die" << '\n';
        f2.val = 200;
    }
    // ~Foo()20
    // rvalue going to die
    // ~Foo()200
}
```

## `unique_ptr`

smart pointer auto clean up memory

```cpp
#include <iostream>
#include <memory>

void* operator new(std::size_t sz) {
    std::cout << "Allocating: " << sz << '\n';
    return std::malloc(sz);
}

void operator delete(void* ptr) noexcept {
    std::cout << "Deallocating memory\n";
    std::free(ptr);
}

int main() {
    {
        auto ptr_c = std::make_unique<char>(100);
    }
    {
        auto ptr_d = new double{1.1};
    }
    {
        auto ptr_i = new int{666};
        delete ptr_i;
    }
}
```

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
        std::cout << *qi << '\n';                    // 24
        std::cout << *(qi.get()) << '\n';            // 24, .get() return raw pointer
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

## `shared_ptr`

`std::shared_ptr` example

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

    void print() { std::cout << "Derived" << '\n'; }
};

struct Apprentice;

struct Master : std::enable_shared_from_this<Master> {
    ~Master() { std::cout << "~Master" << '\n'; }

    void take_apprentice(std::shared_ptr<Apprentice> a);

   private:
    std::shared_ptr<Apprentice> apprentice;
};

struct Apprentice {
    ~Apprentice() { std::cout << "~Apprentice" << '\n'; }

    void take_master(std::weak_ptr<Master> m);

   private:
    std::weak_ptr<Master> master;
};

void Master::take_apprentice(std::shared_ptr<Apprentice> a) {
    apprentice = a;
    apprentice->take_master(weak_from_this()); // since c++17, recommended, return a weak_ptr
    // apprentice->take_master(shared_from_this()); // convert shared_ptr-> weak_ptr
}

void Apprentice::take_master(std::weak_ptr<Master> m) {
    master = m;
}

int main() {
    // simple ctor
    {
        std::shared_ptr<int> p1{new int(100)};
        std::shared_ptr<int> p2 = p1;
        std::cout << p1.use_count() << ',' << p2.use_count() << '\n';  // 2,2
        std::shared_ptr<foo> pf{new foo{1, 20.1, "world"}};
        pf->print();
    }
    // make_shared
    {
        auto p1 = std::make_shared<int>(200);
        *p1 = 200;
        std::cout << *p1 << '\n';  // 200
        auto pf = std::make_shared<foo>(2, 30.2, "grey");
        pf->print();
    }
    // custom deleter
    {
        std::shared_ptr<foo> pf1{new foo{11, 22.2, "james"}, foo_deleter()};
        std::shared_ptr<foo> pf2{
            new foo{22, 33.3, "tom"},
            [](foo* p) {std::cout << "deleting foo from lambda..." << '\n'; delete p; }};
    }
    // arrays
    {
        auto p = new int[3]{100, 200, 300};
        std::cout << p[2] << '\n';  // 300
        delete[] p;

        // <int>
        std::shared_ptr<int> pi1{
            new int[3]{1, 2, 3},
            std::default_delete<int[]>()};
        // std::cout << pi1[2] << '\n';  // error, not allowed because <int>

        // <int>
        std::shared_ptr<int> pi2{
            new int[3]{11, 22, 33},
            [](int* p) { delete[] p; }};

        std::shared_ptr<int> pi3{new int[3]{1, 2, 3}};    // bad
        std::shared_ptr<int[]> pi4{new int[3]{1, 2, 3}};  // since c++17, recommended

        auto pi5 = std::make_shared<int[]>(3);  // recommended
        for (unsigned i = 0; i < 3; ++i) {
            pi5[i] = i + 1;
        }
    }
    {
        std::shared_ptr<int[]> pa1{new int[3]{111, 222, 333}};

        for (unsigned i = 0; i < 3; ++i) {
            pa1[i] *= 10;
        }

        for (unsigned i = 0; i < 3; ++i) {
            std::cout << pa1[i] << ',';
        }
        std::cout << '\n';
    }
    // container
    {
        std::vector<std::shared_ptr<foo>> data;

        for (unsigned i = 0; i < 4; ++i) {
            data.push_back(std::make_shared<foo>(i, i * 10, std::to_string(i * 2)));
        }
        auto pf1 = std::make_shared<foo>(6, 66.6, "tim");
        auto pf2 = std::make_shared<foo>(7, 77.6, "tom");
        data.push_back(pf1);
        data.push_back(std::move(pf2));
        std::cout << pf1.use_count() << '\n';  // 2
        std::cout << pf2.use_count() << '\n';  // 0
    }
    // hierachy
    {
        std::shared_ptr<Derived> pd = std::make_shared<Derived>();
        std::shared_ptr<Base> pb = pd;
        std::cout << pd.use_count() << '\n';  // 2
        std::cout << pb.use_count() << '\n';  // 2

        auto pd2 = std::static_pointer_cast<Derived>(pb);
        pd2->print();  // Derived
    }
    // weak_ptr
    {
        auto sp1 = std::make_shared<int>(1000);
        std::cout << sp1.use_count() << '\n';  // 1

        std::weak_ptr<int> wpi = sp1;
        std::cout << sp1.use_count() << '\n';  // 1
        std::cout << wpi.use_count() << '\n';  // 1

        auto sp2 = wpi.lock();
        std::cout << sp1.use_count() << '\n';  // 2
        std::cout << sp2.use_count() << '\n';  // 2
        std::cout << wpi.use_count() << '\n';  // 2

        sp1.reset();
        std::cout << sp1.use_count() << '\n';  // 0
        std::cout << sp2.use_count() << '\n';  // 1
        std::cout << wpi.use_count() << '\n';  // 1
        // std::cout << *wpi << '\n'; // not allowed
        std::cout << *sp2 << '\n';  // 1000
    }
    // enable_shared_from_this
    {
        auto m = std::make_shared<Master>();
        auto a = std::make_shared<Apprentice>();

        m->take_apprentice(a);
    }
}
```

## custom deleter for `unique_ptr` and `shared_ptr`

[custom deleter](https://medium.com/pranayaggarwal25/custom-deleters-with-shared-ptr-and-unique-ptr-524bb7bd7262)

```cpp
{
    std::unique_ptr<int> ptr(new int(5));
}   
// unique_ptr<int> uses default_delete<int>

{
   std::unique_ptr<int[]> ptr(new int[10]);
}  
// unique_ptr<int[]> uses default_delete<int[]>

{
    std::shared_ptr<int> shared_bad(new int[10]); 
} 
// the destructor calls delete, undefined behavior as it's an array

{
    std::shared_ptr<int> shared_good(new int[10], std::default_delete<int[]> ());
} // the destructor calls delete[], ok

{
shared_ptr<int[]> shared_best(new int[10]);
}
// the destructor calls delete[], awesome!!
```

```cpp
#include <iostream>
#include <memory>

class Foo {
   public:
    int val = 0;
    Foo() = default;
    // Foo &operator=(const Foo &) = delete;
    // Foo &operator=(Foo &&) = delete;
    Foo(int value) : val(value) {}
    ~Foo() { std::cout << "~Foo()" << val << '\n'; }
};

int main() {
    {
        auto pf1 = std::make_unique<Foo>(10);  // use default_delete

        std::unique_ptr<Foo, std::default_delete<Foo>> pf2{
            new Foo{20},
            std::default_delete<Foo>()};

        // custom deleter
        // std::unique_ptr<Foo> pf3{ // error, less tempalte params
        std::unique_ptr<Foo, void (*)(Foo *)> pf3{
            new Foo{30},
            [](Foo *p) {std::cout << "deleting from lambda" << '\n'; delete p; }};
    }
    {
        std::cout << "--------------------" << '\n';
        auto pf1 = std::make_shared<Foo>(11);  // use default_delete

        std::shared_ptr<Foo> pf2{
            new Foo{22},
            std::default_delete<Foo>()};

        // custom deleter
        // std::shared_ptr<Foo, void (*)(Foo*)> pf3{ // error, too many template params
        std::shared_ptr<Foo> pf3{
            new Foo{33},
            [](Foo *p) {std::cout << "deleting from lambda" << '\n'; delete p; }};
    }
    // unique_ptr array
    {
        std::cout << "--------------------" << '\n';
        std::unique_ptr<Foo[], std::default_delete<Foo[]>> pf_arr{
            new Foo[3]{40, 50, 60},
            std::default_delete<Foo[]>()};
    }
    {
        std::cout << "--------------------" << '\n';
        auto pf_arr = std::make_unique<Foo[]>(3);
        pf_arr[1] = Foo{2000};
        std::cout << "Foo{2000} going to die" << '\n';
        pf_arr[2] = std::move(Foo{3000});
        std::cout << "Foo{3000} going to die" << '\n';
    }
    // shard_ptr array
    {
        std::cout << "--------------------" << '\n';
        std::shared_ptr<Foo[]> pf_arr{
            new Foo[3]{44, 55, 66},
            std::default_delete<Foo[]>()};
    }
    {
        std::cout << "--------------------" << '\n';
        auto pf_arr = std::make_shared<Foo[]>(3);
        pf_arr[1] = Foo{2222};
        std::cout << "Foo{2222} going to die" << '\n';
        pf_arr[2] = std::move(Foo{3333});
        std::cout << "Foo{3333} going to die" << '\n';
    }
}
```

## `move` semantic

```cpp
#include <iostream>
#include <vector>

class Buffer {
    unsigned char* ptr;
    size_t length;

   public:
    ~Buffer() { delete[] ptr; }

    Buffer() : ptr(nullptr), length(0) { std::cout << "default ctor" << '\n'; }

    explicit Buffer(size_t const size) : ptr(new unsigned char[size]{0}), length(size) {}

    Buffer(Buffer const& other) : ptr(new unsigned char[other.length]), length(other.length) {
        std::cout << "copy ctor" << '\n';
        std::copy(other.ptr, other.ptr + other.length, ptr);
    }

    Buffer& operator=(Buffer const& other) {
        std::cout << "copy assignment" << '\n';
        if (this != &other) {
            delete[] ptr;

            ptr = new unsigned char[other.length];
            length = other.length;

            std::copy(other.ptr, other.ptr + other.length, ptr);
        }

        return *this;
    }

    Buffer(Buffer&& other) {
        std::cout << "move ctor" << '\n';
        // copy
        ptr = other.ptr;
        length = other.length;

        // reset
        other.ptr = nullptr;
        other.length = 0;
    }

    // Buffer(Buffer&& other) : ptr(nullptr), length(0) {
    //     std::cout << "move ctor" << '\n';
    //     *this = std::move(other);
    // }

    Buffer& operator=(Buffer&& other) {
        std::cout << "move assignment" << '\n';
        if (this != &other) {
            // clean up existing resources
            delete[] ptr;

            // copy
            ptr = other.ptr;
            length = other.length;

            // reset
            other.ptr = nullptr;
            other.length = 0;
        }

        return *this;
    }

    size_t size() const { return length; }
    unsigned char* data() const { return ptr; }
};

int main() {
    {
        Buffer b1;                 // defalt ctor
        Buffer b2(100);            // explicit ctor
        Buffer b3(b2);             // copy ctor
        b1 = b3;                   // assigin operator
        Buffer b4(std::move(b1));  // move ctor
        b3 = std::move(b4);        // move assignment
    }
    {
        std::vector<Buffer> c;
        c.reserve(10);

        c.push_back(Buffer{100});  // move

        Buffer b{200};
        c.push_back(b);             // copy
        c.push_back(std::move(b));  // move
    }
}
```

## operator `<=>`

> three-way comparison operator `<=>`, since C++20

```cpp
#include <compare>
#include <iostream>
#include <tuple>  // std::tie

class foo {
   private:
    int val1;
    double val2;

   public:
    foo(int const i, double const d) : val1(i), val2(d) {}
    bool operator==(foo const& other) const = default;
};

class bar {
   private:
    int val1;
    double val2;

   public:
    bar(int const i, double const d) : val1(i), val2(d) {}
    auto operator<=>(bar const& other) const = default;  // since c++20
};

class custom1 {
   private:
    int val1;
    double val2;

   public:
    custom1(int const i, double const d) : val1(i), val2(d) {}
    // must implement in custom1 type
    bool operator==(custom1 const& other) const {
        return (val1 == other.val1) && (val2 == other.val2);
    }
    auto operator<=>(custom1 const& other) const {
        return std::tie(val1, val2) <=> std::tie(other.val1, other.val2);
    }
};

class custom2 {
   private:
    int val1;

   public:
    custom2(int const i) : val1(i) {}
    // must implement in custom2 type
    bool operator==(custom2 const& other) const {
        return val1 == other.val1;
    }
    auto operator<=>(custom2 const& other) const {
        return val1 <=> other.val1;
    }
};

int main() {
    {
        foo f1{1, 2};
        foo f2{1, 2};
        foo f3{2, 3};
        std::cout << (f1 == f2) << '\n';  //  true
        std::cout << (f1 == f3) << '\n';  // false
        // std::cout << (f1 < f3) << '\n';   // error
    }
    {
        bar b1{1, 2};
        bar b2{1, 2};
        bar b3{2, 1};
        std::cout << (b1 == b2) << '\n';  // true
        std::cout << (b1 == b3) << '\n';  // false
        std::cout << (b1 != b3) << '\n';  // true
        std::cout << (b1 < b3) << '\n';   // true, member-wise compare
        std::cout << (b1 > b3) << '\n';   // false, member-wise compare
        std::cout << (b1 <= b3) << '\n';  // true, member-wise compare
        std::cout << (b1 >= b3) << '\n';  // false, member-wise compare
    }
    {
        custom1 b1{1, 2};
        custom1 b2{1, 2};
        custom1 b3{2, 1};
        std::cout << (b1 == b2) << '\n';  // true
        std::cout << (b1 == b3) << '\n';  // false
        std::cout << (b1 != b3) << '\n';  // true
        std::cout << (b1 < b3) << '\n';   // true, member-wise compare
        std::cout << (b1 > b3) << '\n';   // false, member-wise compare
        std::cout << (b1 <= b3) << '\n';  // true, member-wise compare
        std::cout << (b1 >= b3) << '\n';  // false, member-wise compare
    }
    {
        custom2 b1{10};
        custom2 b2{10};
        custom2 b3{20};
        std::cout << (b1 == b2) << '\n';  // true
        std::cout << (b1 == b3) << '\n';  // false
        std::cout << (b1 != b3) << '\n';  // true
        std::cout << (b1 < b3) << '\n';   // true, member-wise compare
        std::cout << (b1 > b3) << '\n';   // false, member-wise compare
        std::cout << (b1 <= b3) << '\n';  // true, member-wise compare
        std::cout << (b1 >= b3) << '\n';  // false, member-wise compare
    }
}
```

## `std::move`

explain the move semantics

```cpp
#include <fmt/core.h>
#include <vector>

struct MyStruct0 {
    std::vector<int> vec_;

    // v is lvalue, trigger copy of argument, then move the temp object to vec_
    MyStruct0(std::vector<int> v) : vec_(std::move(v)) {}
    void display() const {
        fmt::println("in struct addr: {}", fmt::ptr(vec_.data()));
    }
};

struct MyStruct1 {
    std::vector<int> vec_;

    // v is ref, just copy ref, just then move origin data to vec_
    MyStruct1(std::vector<int>& v) : vec_(std::move(v)) {}
    void display() const {
        fmt::println("in struct addr: {}", fmt::ptr(vec_.data()));
    }
};

struct MyStruct2 {
    std::vector<int> vec_;

    // v can only bind to rvalues
    // Even though v is declared as an rvalue reference, once inside the constructor, v itself is treated as an lvalue.
    // v is a lvlaue, bound to a temporary object
    // This is because all named variables (like v) are considered lvalues, regardless of their type. that is reference collapsing
    MyStruct2(std::vector<int>&& v) : vec_(std::move(v)) {}
    void display() const {
        fmt::println("in struct addr: {}", fmt::ptr(vec_.data()));
    }
};

// in the above 3 structs, if not use `vec_(std::move(v))` but `vec_(v)`, will cause data copy

int main() {
    // In C++, when you use std::move, it casts the object to an rvalue reference (a reference to a tempory object),
    // which allows the resources to be transferred from one object to another.
    {
        // addr is different
        std::vector<int> v{1, 2, 3, 4, 5};
        fmt::println("origin addr: {}", fmt::ptr(v.data()));
        auto obj = MyStruct0(v);
        obj.display();
    }
    {
        // addr is the same
        std::vector<int> v{1, 2, 3, 4, 5};
        fmt::println("origin addr: {}", fmt::ptr(v.data()));
        auto obj = MyStruct1(v);
        obj.display();
        v[0] = 100;  // cause core dumped
        obj.display();
    }
    {
        // addr is the same
        std::vector<int> v{1, 2, 3, 4, 5};
        fmt::println("origin addr: {}", fmt::ptr(v.data()));
        // here should be std::move(), because to meet std::vector<int>&& requirement,
        // just std::move can cast object to rvalue reference
        auto obj = MyStruct2(std::move(v));
        obj.display();
        v[0] = 100;  // cause core dumped
        obj.display();
    }
}
```

## `std::function` overhead

1. `std::function` is about 10% slower than lambda
2. `std::function` as callback in slower than normal 

`std::function` flexible but comes with some overhead. 
- Type Erasure: uses type erasure to store any callable object that matches its signature. This means it abstracts away the concrete type of the callable, which introduces an additional layer of indirection when invoking the callable.
- Dynamic Memory Allocation: might allocate memory on the heap to store the callable object(like lambdas, function pointers, or functors), which can introduce latency.
- Indirection: involves an indirect call through a virtual table or a similar mechanism, preventing certain compiler optimizations like inlining.

```cpp
#include <chrono>
#include <functional>
#include <iostream>
#include <vector>

inline constexpr int myadd(int x) { return x + 1; }

int main() {
    const int iterations = 500000000;
    std::vector<int> data(iterations, 1);
    std::cout << "iterations: " << iterations << '\n';

    // Using std::function
    std::function<int(int)> func = [](int x) { return x + 1; };
    auto start_func = std::chrono::high_resolution_clock::now();
    int sum_func = 0;
    for (auto x : data) {
        sum_func += func(x);
    }
    auto end_func = std::chrono::high_resolution_clock::now();

    // Using direct lambda
    auto lambda = [](int x) { return x + 1; };
    auto start_lambda = std::chrono::high_resolution_clock::now();
    int sum_lambda = 0;
    for (auto x : data) {
        sum_lambda += lambda(x);
    }
    auto end_lambda = std::chrono::high_resolution_clock::now();

    // Using direct function
    auto start_direct = std::chrono::high_resolution_clock::now();
    int sum_direct = 0;
    for (auto x : data) {
        sum_direct += myadd(x);
    }
    auto end_direct = std::chrono::high_resolution_clock::now();

    std::cout << "std::function sum: " << sum_func << " avg Time: "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end_func - start_func).count() / static_cast<double>(iterations)
              << "us\n";
    std::cout << "Lambda sum: " << sum_lambda << " avg Time: "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end_lambda - start_lambda).count() / static_cast<double>(iterations)
              << "us\n";
    std::cout << "Direct sum: " << sum_lambda << " avg Time: "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end_direct - start_direct).count() / static_cast<double>(iterations)
              << "us\n";
}
```

## check default paramter

use default_type

```cpp
#include <iostream>

// Define a default marker type
struct default_type {};

// Helper to detect if a parameter is provided
template <typename T>
constexpr bool is_parameter_provided_v = !std::is_same_v<T, default_type>;

// Primary foo function template with two parameters
template <typename T1 = default_type, typename T2 = default_type>
constexpr void foo(T1 x = T1{}, T2 y = T2{}) {
    if constexpr (is_parameter_provided_v<T1> && is_parameter_provided_v<T2>) {
        std::cout << "Both parameters provided: " << x << ", " << y << '\n';
    } else if constexpr (is_parameter_provided_v<T1> && !is_parameter_provided_v<T2>) {
        std::cout << "Only first parameter provided: " << x << '\n';
    } else if constexpr (!is_parameter_provided_v<T1> && is_parameter_provided_v<T2>) {
        std::cout << "Only second parameter provided: " << y << '\n';
    } else {
        std::cout << "Default parameters used\n";
    }
}

int main() {
    foo(42, 3.14);              // Outputs: Both parameters provided: 42, 3.14
    foo(42);                    // Outputs: Only first parameter provided: 42
    foo(default_type{}, 3.14);  // Only second parameter provided: 3.14
    foo();                      // Outputs: Default parameters used
}
```

use constexpr and default value

```cpp
constexpr int foo(int x = 10, double y = 3.14) {
    return x + static_cast<int>(y);
}

int main() {
    foo();           // Case 1
    foo(11);         // Case 2
    foo(100, 31.4);  // Case 3
}
```

## `weak_ptr`

`std::weak_ptr` provides a way to reference an object managed by `std::shared_ptr` without participating in the ownership of that object. It allows you to observe or access the object without affecting its lifetime. If all `std::shared_ptr` instances owning the object are destroyed, the object itself is destroyed, and the `std::weak_ptr` becomes expired.

Why Use `std::weak_ptr`?
- Avoiding Circular References: In data structures like graphs or trees where nodes may reference each other, using only `std::shared_ptr` can create cycles that prevent reference counts from reaching zero, thereby causing memory leaks. `std::weak_ptr` breaks these cycles by not contributing to the reference count.
- Conditional Access: Sometimes, you want to access an object only if it still exists, without ensuring its lifetime. `std::weak_ptr` allows you to check if the object is still alive before using it.

basic example

```cpp
#include <memory>
#include <iostream>

struct MyObject {
    MyObject() { std::cout << "MyObject created.\n"; }
    ~MyObject() { std::cout << "MyObject destroyed.\n"; }
};

int main() {
    std::weak_ptr<MyObject> wp;
    {
        std::shared_ptr<MyObject> sp = std::make_shared<MyObject>();
        wp = sp; // wp now references the object but doesn't own it
        std::cout << "sp is alive.\n";
    } // sp goes out of scope, object is destroyed
    if (auto locked = wp.lock()) { // Attempt to get shared_ptr
        std::cout << "Object is still alive.\n";
    } else {
        std::cout << "Object has been destroyed.\n";
    }
}
```

practical example: break circular reference

```cpp
#include <memory>
#include <iostream>

struct Child; // Forward declaration

struct Parent {
    std::shared_ptr<Child> child;
    Parent() { std::cout << "Parent created.\n"; }
    ~Parent() { std::cout << "Parent destroyed.\n"; }
};

struct Child {
    std::weak_ptr<Parent> parent; // Use weak_ptr to prevent cycle
    Child() { std::cout << "Child created.\n"; }
    ~Child() { std::cout << "Child destroyed.\n"; }
};

int main() {
    {
        std::shared_ptr<Parent> p = std::make_shared<Parent>();
        std::shared_ptr<Child> c = std::make_shared<Child>();
        p->child = c;
        c->parent = p; // No cycle, weak_ptr does not increase reference count
    }
    std::cout << "End of scope.\n";
}
```

practical example: **Observer Pattern**

```cpp
#include <iostream>
#include <memory>

struct Subject;

struct Observer {
    std::weak_ptr<Subject> subject;

    void observe() {
        if (auto s = subject.lock()) {
            std::cout << "Observing subject.\n";
            // Interact with the subject
        } else {
            std::cout << "Subject no longer exists.\n";
        }
    }
};

struct Subject : std::enable_shared_from_this<Subject> {
    void attach(std::shared_ptr<Observer> obs) {
        obs->subject = shared_from_this();  // share this to obs, so need std::enable_shared_from_this
    }

    ~Subject() { std::cout << "Subject destroyed.\n"; }
};

int main() {
    auto subject = std::make_shared<Subject>();
    auto observer1 = std::make_shared<Observer>();
    auto observer2 = std::make_shared<Observer>();

    subject->attach(observer1);
    subject->attach(observer2);

    observer1->observe();
    observer2->observe();

    subject.reset();  // Destroy the subject

    observer1->observe();
    observer2->observe();
}
```

```cpp
#include <format>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// Observer Interface
class Observer {
   public:
    virtual ~Observer() = default;
    virtual void update(int) = 0;
};

// Subject Class
class Subject {
    std::vector<std::weak_ptr<Observer>> observers;

   public:
    // Attach an observer
    void attach(const std::shared_ptr<Observer>& observer) {
        observers.emplace_back(observer);
    }

    // Detach expired observers using C++20's erase_if
    void detachExpired() {
        std::erase_if(observers, [](const std::weak_ptr<Observer>& wp) { return wp.expired(); });
    }

    // Notify all observers
    void notify(int value) {
        std::cout << std::format("notify all {}\n", value);
        detachExpired();  // Clean up expired observers first

        for (auto&& observer : observers) {
            if (auto obs = observer.lock()) {
                obs->update(value);
            }
        }
    }
};

// Concrete Observer
class ConcreteObserver : public Observer, public std::enable_shared_from_this<ConcreteObserver> {
    std::string name_;

   public:
    ConcreteObserver(std::string name) : name_(std::move(name)) {}
    // update method called by the subject
    void update(int value) override {
        std::cout << std::format("observer {} received {}\n", name_, value);
    }

    void subscribe(std::shared_ptr<Subject> subject) {
        subject->attach(shared_from_this());
    }
};

int main() {
    // Create a subject & observers
    auto subject = std::make_shared<Subject>();
    auto observer1 = std::make_shared<ConcreteObserver>("A");
    auto observer2 = std::make_shared<ConcreteObserver>("B");
    auto observer3 = std::make_shared<ConcreteObserver>("C");

    // Subscribe observers to the subject
    observer1->subscribe(subject);
    observer2->subscribe(subject);
    observer3->subscribe(subject);

    // 1st notify
    subject->notify(100);

    // Destroy observer1
    observer1.reset();
    // 2nd notify
    subject->notify(200);
}
```

## lazy evaluation

### pipe operator

```cpp
#include <algorithm>
#include <print>

template <typename T>
struct ContainsProxy {
    const T& value_;
};

template <typename Range, typename T>
auto operator|(const Range& r, const ContainsProxy<T>& proxy) {
    const auto& v = proxy.value_;
    return std::find(r.begin(), r.end(), v) != r.end();
}

template <typename T>
auto contains(const T& v) { return ContainsProxy<T>{v}; }

int main(int argc, char const* argv[]) {
    const auto r = {-5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5};
    auto val = r | contains(5);
    std::println("{}", val);
}
```

## Trivially Copyable Types

> A **trivially copyable type** is a type that can be copied bitwise (e.g., using `memcpy`) without invoking any special copy constructors, destructors, or other member functions.

features:
- **Trivial Constructors and Destructors**. default constructors, copy constructors, move constructors, copy assignment operators, and move assignment operators that are all trivial. A trivial constructor or destructor does nothing beyond what the compiler *automatically generates* (e.g., no resource allocation or deallocation).
- **No Virtual Functions or Base Classes**. The type does not have virtual functions or virtual base classes, ensuring that there are no hidden pointers or additional data structures that complicate the memory layout.
- **No Non-Trivial Members**. All *non-static* data members must themselves be trivially copyable. If a member is not trivially copyable, the containing type also isn't.

Key Point: *Static* members are not considered part of the instance's memory layout. They exist independently of any object instances and are shared across all instances of the class or struct.

Why It Matters:
- Performance: Bitwise copying is generally faster than invoking copy constructors, especially for large arrays or performance-critical applications.
- Interoperability: Facilitates interaction with C APIs or hardware where data structures need to have a specific memory layout.
- Serialization: Simplifies the process of serializing and deserializing data.