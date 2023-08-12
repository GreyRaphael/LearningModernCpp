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