# Standard Library Containers, Algorithms, and Iterators

- [Standard Library Containers, Algorithms, and Iterators](#standard-library-containers-algorithms-and-iterators)
  - [`std::vector`](#stdvector)
    - [basic operation](#basic-operation)
    - [vector with object](#vector-with-object)
  - [sort](#sort)
  - [`find`](#find)
  - [Initializing a range](#initializing-a-range)
  - [custom random-access iterator](#custom-random-access-iterator)
  - [`std::any`](#stdany)
  - [`std::optional`](#stdoptional)
  - [`std::variant`](#stdvariant)
    - [`std::variant` for duck-typing polymorphism](#stdvariant-for-duck-typing-polymorphism)
    - [polymorphism by `std::variant` with concept](#polymorphism-by-stdvariant-with-concept)
  - [`std::tuple`](#stdtuple)
  - [flexible array](#flexible-array)
  - [reserve vs resize](#reserve-vs-resize)

C++ Standard Library core initially sat three main pillars: **containers**, **algorithms**,
and **iterators**

## `std::vector`

### basic operation

```cpp
#include <iostream>
#include <list>
#include <vector>

template <typename T>
void print_vect(std::vector<T> v) noexcept {
    for (auto &&item : v) {
        std::cout << item << '\t';
    }
    std::cout << '\n';
}

int main() {
    // double static_array[4] = {1, 2, 3, 4};
    double static_array[] = {1, 2, 3, 4};
    int *dynamic_array = new int[4]{11, 22, 33, 44};
    // only for staitc_array
    for (auto &&i : static_array) {
        std::cout << i << '\t';
    }
    std::cout << '\n';

    // for static_array + dynamci_array
    for (unsigned i = 0; i < 4; ++i) {
        std::cout << dynamic_array[i] << '\t';
    }
    std::cout << '\n';
    for (unsigned i = 0; i < 4; ++i) {
        std::cout << static_array[i] << '\t';
    }
    std::cout << '\n';

    // 1. from initialize-list
    std::vector<int> v10{10, 20, 30, 40};
    std::vector<int> v11 = {11, 21, 31, 41};
    std::vector v12{12, 22, 32, 42};
    std::vector v13 = {13, 23, 33, 43};
    // auto il1{5, 6, 7, 8}; // error
    auto il1 = {5, 6, 7, 8};
    std::cout << typeid(il1).name() << '\n';  // type is initialize-list

    // 2. from array
    std::vector<int> v2(static_array, static_array + 3);
    std::vector<int> v3(dynamic_array, dynamic_array + 3);
    // 3. from std::list
    std::list<int> l1{100, 200, 300};
    std::vector<int> v4(l1.begin(), l1.end());
    print_vect(v4);

    // 4. from count
    std::vector<int> v5(4, 6);
    print_vect(v5);

    // 5. insert
    std::vector<int> v6;
    v6.insert(v6.end(), static_array, static_array + 4);
    v6.insert(v6.begin(), dynamic_array, dynamic_array + 4);
    print_vect(v6);

    // 6. erase
    std::vector<int> v7{1, 2, 3, 4, 5, 6, 7, 8};
    v7.erase(v7.begin() + 1, v7.begin() + 3);
    print_vect(v7);

    // 7.clear
    v7.clear()
}
// 1       2       3       4
// 11      22      33      44
// 1       2       3       4
// St16initializer_listIiE
// 100     200     300
// 6       6       6       6
// 11      22      33      44      1       2       3       4
// 1       4       5       6       7       8
```

example: get the pointer from vector

```cpp
#include <iostream>
#include <vector>

template <typename T>
void print_vect(std::vector<T> v) noexcept {
    for (auto&& item : v) {
        std::cout << item << '\t';
    }
    std::cout << '\n';
}

void print_array(int const* const arr, int const size) {
    for (unsigned i = 0; i < size; ++i) {
        std::cout << arr[i] << ',';
    }
    std::cout << '\n';
}

int main() {
    std::vector<int> v{1, 2, 3, 4, 5};
    int* ptr = v.data();  // a pointer to the first element

    for (unsigned i = 0; i < 5; ++i) {
        *ptr += 100;
        ptr++;
    }
    print_vect(v);
    // get a pointer to the first element
    print_array(v.data(), v.size());
    print_array(&v.front(), v.size());
    print_array(&v[0], v.size());
    print_array(&*v.begin(), v.size());
}
// 101     102     103     104     105
// 101,102,103,104,105,
// 101,102,103,104,105,
// 101,102,103,104,105,
// 101,102,103,104,105,
```

### vector with object

example: ctor & destructor

```cpp
#include <iostream>

struct Item {
    int x;

    Item() : x(0) { std::cout << "default ctor\n"; }
    Item(int a) : x(a) { std::cout << "simple ctor:" << x << " at " << this << std::endl; }
    Item(const Item &rhs) : x(rhs.x) {
        std::cout << "copy ctor: " << &rhs << " => " << this << ", value=" << rhs.x << std::endl;
    }
    Item(Item &&rhs) : x(rhs.x) {
        std::cout << "move ctor: " << &rhs << " => " << this << ", value=" << rhs.x << std::endl;
    }
    ~Item() { std::cout << "destructor:" << x << " at " << this << std::endl; }
};

int main() {
    Item a{10};
    {
        Item b{std::move(a)};
    }  // 出了这个block, b失效，控制权重新回到a
    std::cout << a.x << std::endl;
}
// simple ctor:10 at 0x7fffc9534c4c
// move ctor: 0x7fffc9534c4c => 0x7fffc9534c48, value=10
// destructor:10 at 0x7fffc9534c48
// 10
// destructor:10 at 0x7fffc9534c4c
```

`push_back` vs `emplace_back`
- push_back: 要调用构造函数和move构造函数。要先构造一个临时对象，然后把临时对象move到容器最后面
- emplace_back: 直接在vector原地构造对象

```cpp
#include <iostream>
#include <vector>

struct Item {
    int x;

    Item() : x(0) { std::cout << "default ctor\n"; }
    Item(int a) : x(a) { std::cout << "simple ctor:" << x << " at " << this << std::endl; }
    Item(const Item &rhs) : x(rhs.x) {
        std::cout << "copy ctor: " << &rhs << " => " << this << ", value=" << rhs.x << std::endl;
    }
    Item(Item &&rhs) : x(rhs.x) {
        std::cout << "move ctor: " << &rhs << " => " << this << ", value=" << rhs.x << std::endl;
    }
    ~Item() { std::cout << "destructor:" << x << " at " << this << std::endl; }
};

int main()
{
   std::vector<Item> v1;
   v1.reserve(5); 
   // reserve为容器预留空间，但在空间内不真正创建元素对象
   // resize(5)和std::vector<Item> v1(5)为容器预留空间，在空间内调用default ctor创建元素对象

   Item obj{1}; // simple ctor:1
   v1.push_back(obj); // copy ctor:1
   v1.push_back(obj); // copy ctor:1
   auto cap = v1.capacity();
   std::cout << "origin capacity: " << cap << std::endl; // 5
   std::cout << std::endl;
   {
      std::cout << "call emplace_back:\n";
      v1.emplace_back(2); // simple ctor:2
      auto cap2 = v1.capacity();
      std::cout << "final capacity: " << cap2 << std::endl; // 5
   }
   {
      std::cout << "\ncall push_back:\n";
      v1.push_back(3); // 创建临时对象，move到vector，销毁临时对象
      auto cap2 = v1.capacity();
      std::cout << "final capacity: " << cap2 << std::endl; // 5
   }
   for (auto &&i : v1){ std::cout << i.x << std::endl;} // 1 1 2 3
}
// simple ctor:1 at 0x7fffeed7f76c
// copy ctor: 0x7fffeed7f76c => 0x7fffe756aeb0, value=1
// copy ctor: 0x7fffeed7f76c => 0x7fffe756aeb4, value=1
// origin capacity: 5

// call emplace_back:
// simple ctor:2 at 0x7fffe756aeb8
// final capacity: 5

// call push_back:
// simple ctor:3 at 0x7fffeed7f794
// move ctor: 0x7fffeed7f794 => 0x7fffe756aebc, value=3
// destructor:3 at 0x7fffeed7f794
// final capacity: 5
// 1
// 1
// 2
// 3
// destructor:1 at 0x7fffeed7f76c
// destructor:1 at 0x7fffe756aeb0
// destructor:1 at 0x7fffe756aeb4
// destructor:2 at 0x7fffe756aeb8
// destructor:3 at 0x7fffe756aebc
```

example: ctor with two-arguments

```cpp
#include <iostream>
#include <vector>

struct Item {
    int x;
    double y;

    Item() : x(0), y(0) { std::cout << "default ctor\n"; }
    Item(int a, double b) : x(a), y(b) { std::cout << "simple ctor:" << x << " at " << this << std::endl; }
    Item(const Item &rhs) : x(rhs.x), y(rhs.y) {
        std::cout << "copy ctor: " << &rhs << " => " << this << ", value=" << rhs.x << std::endl;
    }
    // Item &operator=(const Item &) = default; // another method
    Item &operator=(const Item &rhs) {
        std::cout << "operator=: " << &rhs << " => " << this << ", value=" << rhs.x << std::endl;
        x = rhs.x + 1;
        y = rhs.y;
        return *this;
    }
    Item(Item &&rhs) : x(rhs.x), y(rhs.y) {
        std::cout << "move ctor: " << &rhs << " => " << this << ", value=" << rhs.x << std::endl;
    }
    ~Item() {
        std::cout << "destructor:" << x << " at " << this << std::endl;
    }
};

int main() {
    std::vector<Item> v1;
    v1.reserve(5);

    // v1.push_back(1, 2.2); // error
    v1.push_back({1, 2.5});
    std::cout << std::endl;

    v1.emplace_back(11, 11.5);
    // v1.emplace_back({11, 22.5}); // error
    std::cout << std::endl;
}

// simple ctor:1 at 0x7fffe4d0ddf0
// move ctor: 0x7fffe4d0ddf0 => 0x7fffdcb69eb0, value=1
// destructor:1 at 0x7fffe4d0ddf0

// simple ctor:11 at 0x7fffdcb69ec0

// destructor:1 at 0x7fffdcb69eb0
// destructor:11 at 0x7fffdcb69ec0
```

example: `emplace`
> **operator asignment** is implicitly declared as deleted because ‘Item’ declares a **move constructor** or **move assignment operator**  
> 所以需要实现`operator assignment`，或者直接`=default`

```cpp
#include <iostream>
#include <vector>

struct Item {
    int x;
    double y;

    Item() : x(0), y(0) { std::cout << "default ctor\n"; }
    Item(int a, double b) : x(a), y(b) { std::cout << "simple ctor:" << x << " at " << this << std::endl; }
    Item(const Item &rhs) : x(rhs.x), y(rhs.y) {
        std::cout << "copy ctor: " << &rhs << " => " << this << ", value=" << rhs.x << std::endl;
    }
    // Item &operator=(const Item &) = default; // another method
    Item &operator=(const Item &rhs) {
        std::cout << "operator=: " << &rhs << " => " << this << ", value=" << rhs.x << std::endl;
        x = rhs.x + 1;
        y = rhs.y;
        return *this;
    }
    Item(Item &&rhs) : x(rhs.x), y(rhs.y) {
        std::cout << "move ctor: " << &rhs << " => " << this << ", value=" << rhs.x << std::endl;
    }
    ~Item() {
        std::cout << "destructor:" << x << " at " << this << std::endl;
    }
};

int main() {
    std::vector<Item> v1;
    v1.reserve(5);
    v1.emplace_back(10, 20.3);
    v1.emplace(v1.end(), 20, 100.2);
    v1.emplace(v1.begin(), 30, 40.5);

    for (auto &&i : v1) {
        std::cout << i.x << std::endl;
    }
}
// simple ctor:10 at 0x7fffc04f9eb0
// simple ctor:20 at 0x7fffc04f9ec0
// simple ctor:30 at 0x7fffc8667a78
// move ctor: 0x7fffc04f9ec0 => 0x7fffc04f9ed0, value=20
// operator=: 0x7fffc04f9eb0 => 0x7fffc04f9ec0, value=10
// operator=: 0x7fffc8667a78 => 0x7fffc04f9eb0, value=30
// destructor:30 at 0x7fffc8667a78
// 31
// 11
// 20
// destructor:31 at 0x7fffc04f9eb0
// destructor:11 at 0x7fffc04f9ec0
// destructor:20 at 0x7fffc04f9ed0
```

example: `std::vector` basic operation

```cpp
#include <iostream>
#include <vector>

struct Item {
    int x;
    double y;

    Item() : x(0), y(0) { std::cout << "default ctor\n"; }
    Item(int a, double b) : x(a), y(b) { std::cout << "simple ctor:" << x << " at " << this << std::endl; }
    Item(const Item &rhs) : x(rhs.x), y(rhs.y) {
        std::cout << "copy ctor: " << &rhs << " => " << this << ", value=" << rhs.x << std::endl;
    }
    // Item &operator=(const Item &) = default; // another method
    Item &operator=(const Item &rhs) {
        std::cout << "operator=: " << &rhs << " => " << this << ", value=" << rhs.x << std::endl;
        x = rhs.x + 1;
        y = rhs.y;
        return *this;
    }
    Item(Item &&rhs) : x(rhs.x), y(rhs.y) {
        std::cout << "move ctor: " << &rhs << " => " << this << ", value=" << rhs.x << std::endl;
    }
    ~Item() {
        std::cout << "destructor:" << x << " at " << this << std::endl;
    }
};

int main() {
    std::vector<Item> v1;
    v1.reserve(5);
    for (unsigned i = 0; i < 3; ++i) {
        v1.emplace_back(i, 1.1 + i);
    }

    // method1: copy all
    std::vector<Item> v2;
    v2 = v1;
    for (auto &&item : v2) {
        std::cout << item.x << std::endl;
    }
    // method2: copy all
    std::vector<Item> v3;
    v3.assign(v1.begin(), v1.end());
    for (auto &&item : v3) {
        std::cout << item.x << std::endl;
    }
    // swap
    // batch create
    std::vector<Item> v4(5, {233, 6});
    for (auto &&item : v4) {
        std::cout << item.x << std::endl;
    }
    v4.swap(v1);
    for (auto &&item : v4) {
        std::cout << item.x << std::endl;
    }
    // clear all
    v1.clear();
}
// simple ctor:0 at 0x7fffe5228eb0
// simple ctor:1 at 0x7fffe5228ec0
// simple ctor:2 at 0x7fffe5228ed0
// copy ctor: 0x7fffe5228eb0 => 0x7fffe5229320, value=0
// copy ctor: 0x7fffe5228ec0 => 0x7fffe5229330, value=1
// copy ctor: 0x7fffe5228ed0 => 0x7fffe5229340, value=2
// 0
// 1
// 2
// copy ctor: 0x7fffe5228eb0 => 0x7fffe5229360, value=0
// copy ctor: 0x7fffe5228ec0 => 0x7fffe5229370, value=1
// copy ctor: 0x7fffe5228ed0 => 0x7fffe5229380, value=2
// 0
// 1
// 2
// simple ctor:233 at 0x7fffec8ad0e0
// copy ctor: 0x7fffec8ad0e0 => 0x7fffe52293a0, value=233
// copy ctor: 0x7fffec8ad0e0 => 0x7fffe52293b0, value=233
// copy ctor: 0x7fffec8ad0e0 => 0x7fffe52293c0, value=233
// copy ctor: 0x7fffec8ad0e0 => 0x7fffe52293d0, value=233
// copy ctor: 0x7fffec8ad0e0 => 0x7fffe52293e0, value=233
// destructor:233 at 0x7fffec8ad0e0
// 233
// 233
// 233
// 233
// 233
// 0
// 1
// 2
// destructor:233 at 0x7fffe52293a0
// destructor:233 at 0x7fffe52293b0
// destructor:233 at 0x7fffe52293c0
// destructor:233 at 0x7fffe52293d0
// destructor:233 at 0x7fffe52293e0
// destructor:0 at 0x7fffe5228eb0
// destructor:1 at 0x7fffe5228ec0
// destructor:2 at 0x7fffe5228ed0
// destructor:0 at 0x7fffe5229360
// destructor:1 at 0x7fffe5229370
// destructor:2 at 0x7fffe5229380
// destructor:0 at 0x7fffe5229320
// destructor:1 at 0x7fffe5229330
// destructor:2 at 0x7fffe5229340
```

## sort

example: sort struct

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

struct Task{
    int priority;
    std::string name;
};

bool operator<(Task const & lhs, Task const & rhs){
    return lhs.priority < rhs.priority;
}

bool operator>(Task const & lhs, Task const & rhs){
    return lhs.priority > rhs.priority;
}

int main()
{

    std::vector<Task> v1{
        {10, "Task1"}, 
        {40, "Task2"}, 
        {25, "Task3"}, 
        {10, "Task4"}, 
        {80, "Task5"}, 
        {10, "Task6"}, 
    };
    std::sort(v1.begin(), v1.end());
    for(auto&& i: v1){std::cout<<i.priority<<':'<<i.name<<std::endl;}

    std::cout<<std::endl;
   //  sorting a range but keeping the order of the equal elements
    std::stable_sort(v1.begin(), v1.end());
    for(auto&& i: v1){std::cout<<i.priority<<':'<<i.name<<std::endl;}
}
```

example: sort with lambda for structs

```cpp
#include <algorithm>
#include <iostream>
#include <vector>

struct Task {
    int priority;
    std::string name;
};

int main() {
    std::vector<Task> v1{
        {10, "Task1"},
        {40, "Task2"},
        {25, "Task3"},
        {10, "Task4"},
        {80, "Task5"},
        {10, "Task6"},
    };

    auto compare = [](const auto& a, const auto& b) {
        return a.priority < b.priority;
    };
    std::sort(v1.begin(), v1.end(), compare);
    for (auto&& i : v1) {
        std::cout << i.priority << ':' << i.name << std::endl;
    }
}
```

example: sort with lambda for `tuple`s

```cpp
#include <algorithm>
#include <iostream>
#include <tuple>
#include <vector>

int main() {
    std::vector<std::tuple<int, std::string>> v1{
        {10, "Task1"},
        {40, "Task2"},
        {25, "Task3"},
        {10, "Task4"},
        {80, "Task5"},
        {10, "Task6"},
    };

    auto compare = [](const auto& a, const auto& b) {
        return std::get<0>(a) < std::get<0>(b);
    };
    std::sort(v1.begin(), v1.end(), compare);
    for (auto& [val, str] : v1) {
        std::cout << val << ':' << str << '\n';
    }
}
```

## `find`

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

int main()
{
    std::vector v{1, 3, 2, 4, 6, 19};
    auto it1=std::find(v.cbegin(), v.cend(), 7);
    if(it1!=v.cend()){
        std::cout<<"result="<<*it1
        <<", index="<< std::distance(v.cbegin(), it1)
        <<std::endl;
    }else{
        std::cout<<"no result\n";
    } // no result

    auto it2=std::find_if(v.cbegin(), v.cend(), [](int const n){return n>4;});
    if(it2!=v.cend()){
        std::cout<<"result="<<*it2
        <<", index="<< std::distance(v.cbegin(), it2)
        <<std::endl;
    }else{
        std::cout<<"no result\n";
    } // result=6, index=4
    
    auto it3=std::find_if_not(v.cbegin(), v.cend(), [](int const n){return n>4;});
    if(it3!=v.cend()){
        std::cout<<"result="<<*it3
        <<", index="<< std::distance(v.cbegin(), it3)
        <<std::endl;
    }else{
        std::cout<<"no result\n";
    } // result=1, index=0

    std::vector p{4, 7, 11};
    auto it4=std::find_first_of(v.cbegin(), v.cend(), p.cbegin(), p.cend());
    if(it4!=v.cend()){
        std::cout<<"result="<<*it4
        <<", index="<< std::distance(v.cbegin(), it4)
        <<std::endl;
    }else{
        std::cout<<"no result\n";
    } // result=4, index=3

    std::vector p2{2, 4, 6};
    auto it5=std::find_end(v.cbegin(), v.cend(), p2.cbegin(), p2.cend());
    if(it5!=v.cend()){
        std::cout<<"result="<<*it5
        <<", index="<< std::distance(v.cbegin(), it5)
        <<std::endl;
    }else{
        std::cout<<"no result\n";
    } // result=2, index=2

}
```

## Initializing a range

```cpp
#include <algorithm>
#include <iostream>
#include <list>
#include <random>
#include <vector>

template <typename T>
void print_vect(std::vector<T> v) noexcept {
    for (auto&& item : v) {
        std::cout << item << ',';
    }
    std::cout << '\n';
}
template <typename T>
void print_li(std::list<T> v) noexcept {
    for (auto&& item : v) {
        std::cout << item << ',';
    }
    std::cout << '\n';
}

int main() {
    // simple fill
    std::vector<int> v1(8);
    std::fill(v1.begin(), v1.end(), 2);
    std::fill_n(v1.begin(), 3, 42);
    print_vect(v1);  // 42,42,42,2,2,2,2,2,

    // generate uniform random
    std::random_device rd;
    std::mt19937 mt{rd()};
    std::uniform_int_distribution<> ud{1, 10};
    std::vector<int> v2(5);
    std::generate(v2.begin(), v2.end(), [&ud, &mt] { return ud(mt); });
    print_vect(v2);  // 1,4,7,5,6,

    // generate simple
    std::vector<int> v3(5);
    int i = 1;
    std::generate_n(v3.begin(), 3, [&i] { return i * i++; });
    print_vect(v3);  // 1,4,9,0,0,

    // iota
    std::vector<int> v4(5);
    std::iota(v4.begin(), v4.end(), 2);
    print_vect(v4);  // 2,3,4,5,6,

    // backinserter: container must have push_back
    std::vector<int> v5(3, 1);
    std::fill_n(std::back_inserter(v5), 3, 10);
    // std::fill_n(std::front_inserter(v1), 3, 0); // error, vector has no push_front
    print_vect(v5);  // 1,1,1,10,10,10,

    // container must have push_front
    std::list<int> li1{1, 2, 3};
    std::fill_n(std::front_inserter(li1), 3, 0);  // 0,0,0,1,2,3,
    print_li(li1);

    std::vector<int> v6(5, 2);
    std::fill_n(std::inserter(v6, v6.begin() + 2), 3, 7);
    print_vect(v6);  // 2,2,7,7,7,2,2,2,

    std::list<int> li2{1, 1, 1, 1};
    // std::fill_n(std::inserter(li2, li2.begin() + 2), 3, 0); // list没有beigin()的operator+，只能advance
    auto it = li2.begin();
    std::advance(it, 3);
    std::fill_n(std::inserter(li2, it), 3, 0);
    print_li(li2);  // 1,1,1,0,0,0,1,
}
```

## custom random-access iterator

First we show a simple example with [begin-end](examples/ch06-begin-end.cc)

We implement [range-base class](02Introductions.md#custom-range-base-class) in the previous chapter. Here we implement a [standard iterator](examples/ch06-iterator.cc) for random-access.

## `std::any`

> `std::any` can hold a single value of any type.

```cpp
#include <any>
#include <chrono>
#include <iomanip>  // std::put_time
#include <iostream>
#include <vector>

void log(std::any const& value) {
    if (value.has_value()) {
        auto const& tv = value.type();
        if (tv == typeid(int)) {
            std::cout << std::any_cast<int>(value) << '\n';
        } else if (tv == typeid(char)) {
            std::cout << std::any_cast<char>(value) << '\n';
        } else if (tv == typeid(std::string)) {
            std::cout << std::any_cast<std::string>(value) << '\n';
        } else if (tv == typeid(std::chrono::time_point<std::chrono::system_clock>)) {
            auto t = std::any_cast<std::chrono::time_point<std::chrono::system_clock>>(value);
            auto now = std::chrono::system_clock::to_time_t(t);
            std::cout << std::put_time(std::localtime(&now), "%F %T") << '\n';
        } else {
            std::cout << "unexpected value type" << '\n';
        }
    } else {
        std::cout << "(empty)" << '\n';
    }
}

int main() {
    // std::any variable, 必须使用std::any_cast才能获取原始的值
    std::any val(23);
    val = 12.0;
    val = std::string("world");
    val.reset();  // val is reset as: std::any{}
    // vector of std::any
    std::vector<std::any> v = {
        std::any{},
        42,
        'c',
        11.0,
        std::string("hello"),
        std::chrono::system_clock::now()};
    for (auto& e : v) {
        log(e);
    }
}
```

## `std::optional`

> `std::optional` is a template container for storing a value that may or may not exist.

example1: `std::optional` as return value

```cpp
#include <iostream>
#include <map>
#include <optional>

template <typename K, typename V>
std::optional<V> find(K const& key, std::map<K, V> const& m) {
    auto it = m.find(key);
    if (it != m.end()) {
        return it->second;
    } else {
        return {};  // empty optional
    }
}

int main() {
    std::map<int, std::string> dict{
        {2, "James"},
        {1, "Tom"},
        {3, "Jack"}};
    // auto val = find(3, dict);
    auto val = find(30, dict);
    if (val) {
        std::cout << *val << '\n';
    } else {
        std::cout << "find nothing" << '\n';
    }
}
```

example2: `std::optional` as arguments

```cpp
#include <iostream>
#include <optional>
#include <vector>

// std::option as parameters
std::string extract(std::string const& text, std::optional<int> start, std::optional<int> end) {
    auto s = start.value_or(0);  // if empty, then set as 0
    auto e = end.value_or(text.size());
    return text.substr(s, e - s);
}

// std::optional for struct fields
struct Book {
    std::string title;
    std::optional<std::string> subtitle;
    std::vector<std::string> authors;
    std::string publisher;
    std::string isbn;
    std::optional<int> pages;
    std::optional<int> year;
};

int main() {
    std::cout << extract("hello", {}, {}) << '\n';  // hello
    std::cout << extract("hello", 1, {}) << '\n';   // ello
    std::cout << extract("hello", {}, 4) << '\n';   // hello
}
```

example3: `std::optional`造成performance损失

solutions:
- entirely avoiding using `std::optional`
- overload `void process(bar const& arg)`

```cpp
#include <iostream>
#include <optional>

struct bar {
    bar() { std::cout << "default ctor" << '\n'; }
    ~bar() { std::cout << "dtor" << '\n'; }
    bar(bar const& b) { std::cout << "copy ctor" << '\n'; }
    bar(bar&& b) { std::cout << "move ctor" << '\n'; }
};

void process(std::optional<bar> const& arg) {
    std::cout << "in process" << '\n';
}

int main() {
    std::optional<bar> b1{bar{}};
    bar b2{};

    process(b1);  // no copy
    process(b2);  // copy construction, 因为b2是bar不是std::optional<bar>所以要copy ctor,造成性能损失
    std::cout << "out process" << '\n';
}
```

```bash
# output
default ctor
move ctor
dtor
default ctor
in process
copy ctor
in process
dtor
out process
dtor
dtor
```

## `std::variant`

> In C++17, a type-safe union is available in the form of a standard library class
template called `std::variant`.

example: `std::visit` without return for `std::variant`
> `decltype`: declare type, such as `int, int& , int&&`  
> `decay_t`: e.g. 将类型`int`, `int&`, `int&&` 退化为 `int`

```cpp
#include <iostream>
#include <variant>

int main() {
    std::variant<int, double, std::string> v1;
    v1 = 100.01;
    std::cout << v1.index() << '\n';  // 1
    v1 = 10;
    std::cout << v1.index() << '\n';  // 0

    std::visit([](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int>)
            std::cout << "int " << arg << '\n'; // int 10
        else if constexpr (std::is_same_v<T, double>)
            std::cout << "double " << arg << '\n';
        else if constexpr (std::is_same_v<T, std::string>)
            std::cout << "std::string " << arg << '\n';
    },
               v1);
}
```

basic for lambda overload since c++20

```cpp
#include <print>

template <class... Lambdas>
struct Overloaded : Lambdas... {
    using Lambdas::operator()...;
    // For each lambda type in Lambdas, this statement brings its operator() into Overloaded.
};

int main(int argc, char const *argv[]) {
    auto overloaded_lambdas = Overloaded{[](int v) { std::println("int: {}", v); },
                                         [](bool v) { std::println("bool: {}", v); },
                                         [](float v) { std::println("float: {}", v); }};

    overloaded_lambdas(30031);
    overloaded_lambdas(2.71828f);
}
```

`std::variant` with struct and `std::visit`
- method1: by lambda
- method2: by struct operator(), recommended
- method3: by functor operator()


```cpp
#include <iostream>
#include <variant>

struct Order {
    char orderid[20];
    int volume;
    double price;
};

struct Trade {
    char tradeid[20];
    char direction;
    int volume;
};

using StructVariant = std::variant<Order, Trade>;
// Function to print Order
void printOrder(const Order& order) {
    std::cout << "Order ID: " << order.orderid
              << ", Volume: " << order.volume
              << ", Price: " << order.price << std::endl;
}

// Function to print Trade
void printTrade(const Trade& trade) {
    std::cout << "Trade ID: " << trade.tradeid
              << ", Direction: " << trade.direction
              << ", Volume: " << trade.volume << std::endl;
}

// method1: by lambda
auto printVisitor1 = [](auto&& arg) {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, Order>) {
        std::cout << "Processing Order" << std::endl;
        printOrder(arg);
    } else if constexpr (std::is_same_v<T, Trade>) {
        std::cout << "Processing Trade" << std::endl;
        printTrade(arg);
    }
};

// since C++20
auto printVisitor2 = []<typename T>(T const& arg) {
    if constexpr (std::is_same_v<T, Order>) {
        std::cout << "Processing Order" << std::endl;
        printOrder(arg);
    } else if constexpr (std::is_same_v<T, Trade>) {
        std::cout << "Processing Trade" << std::endl;
        printTrade(arg);
    }
};

// method2: by struct operator()
struct Visitor {
    void operator()(Order const& o) {
        printOrder(o);
    }
    void operator()(Trade const& t) {
        printTrade(t);
    }
};

// method3: by functor operator()
template <class... Ts>
struct overload : Ts... {
    using Ts::operator()...;
};

auto MyVisitor = overload{
    [](Order const& o) { printOrder(o); },
    [](Trade const& o) { printTrade(o); },
};

int main() {
    StructVariant orderOrTrade;

    // Assigning an Order to the variant
    orderOrTrade = Order{"12345", 10, 99.99};
    std::visit(printVisitor1, orderOrTrade);
    std::visit(printVisitor2, orderOrTrade);

    // Assigning a Trade to the variant
    orderOrTrade = Trade{"54321", 'B', 20};
    std::visit(Visitor{}, orderOrTrade);

    orderOrTrade = Order{.orderid = "66666", .volume = 100, .price = 100.1};
    std::visit(MyVisitor, orderOrTrade);
}
```

### `std::variant` for duck-typing polymorphism

polymorphism by virtual function

```cpp
class Base {
   public:
    virtual ~Base() = default;

    virtual void PrintName() const {
        std::cout << "calling Bases!\n"
    }
};

class Derived : public Base {
   public:
    void PrintName() const override {
        std::cout << "calling Derived!\n"
    }
};

class ExtraDerived : public Base {
   public:
    void PrintName() const override {
        std::cout << "calling ExtraDerived!\n"
    }
};

std::unique_ptr<Base> pObject = std::make_unique<Derived>();
pObject->PrintName();
```


polymorphism(duck-typing) example by `std::variant` & `std::visit`:
> [tutorial](https://www.cppstories.com/2020/04/variant-virtual-polymorphism.html/) & [notes](https://www.cppstories.com/2018/06/variant/)

```cpp
#include <fmt/core.h>
#include <string_view>
#include <variant>

struct Derived {
    void PrintName(std::string_view txt) const {
        fmt::println("in Derived, {}", txt);
    }
};

struct ExtraDerived {
    void PrintName(std::string_view txt) const {
        fmt::println("in ExtraDerived, {}", txt);
    }
};

struct CallPrintName {
    std::string_view intro;
    void operator()(const Derived& d) { d.PrintName(intro); }
    void operator()(const ExtraDerived& ed) { ed.PrintName(intro); }
};

int main(int argc, char const* argv[]) {
    using MyType = std::variant<Derived, ExtraDerived>;
    MyType var = ExtraDerived{};
    std::visit(CallPrintName{"intro text"}, var);
}
```

complicated example with c++20 concept

```cpp
#include <concepts>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

template <typename T>
concept ILabel = requires(const T v) {
    { v.buildHtml() } -> std::convertible_to<std::string>;
};

class HtmlLabelBuilder {
   public:
    template <ILabel... Label>
    [[nodiscard]] std::string buildHtml(const std::variant<Label...>& label) {
        return std::visit(*this, label);
    }

    // private: //some friend declaration for std::visit?
    template <ILabel Label>
    std::string operator()(const Label& label) const {
        return label.buildHtml();
    }
};

class SimpleLabel {
   public:
    SimpleLabel(std::string str) : _str(std::move(str)) {}

    [[nodiscard]] std::string buildHtml() const {
        return "<p>" + _str + "</p>";
    }

   private:
    std::string _str;
};

class DateLabel {
   public:
    DateLabel(std::string dateStr) : _str(std::move(dateStr)) {}

    [[nodiscard]] std::string buildHtml() const {
        return "<p class=\"date\">Date: " + _str + "</p>";
    }

   private:
    std::string _str;
};

class IconLabel {
   public:
    IconLabel(std::string str, std::string iconSrc) : _str(std::move(str)), _iconSrc(std::move(iconSrc)) {}

    [[nodiscard]] std::string buildHtml() const {
        return "<p><img src=\"" + _iconSrc + "\"/>" + _str + "</p>";
    }

   private:
    std::string _str;
    std::string _iconSrc;
};

class HelloLabel {
   public:
    [[nodiscard]] const char* buildHtml() const {  // doesn't return std::string
        return "<p>Hello</p>";
    }
};

int main() {
    using LabelVariant = std::variant<SimpleLabel, DateLabel, IconLabel, HelloLabel>;
    std::vector<LabelVariant> vecLabels;
    vecLabels.emplace_back(HelloLabel{});
    vecLabels.emplace_back(SimpleLabel{"Hello World"});
    vecLabels.emplace_back(DateLabel{"10th August 2020"});
    vecLabels.emplace_back(IconLabel{"Error", "error.png"});

    {
        std::string finalHTML;
        auto builder = HtmlLabelBuilder{};
        for (auto& label : vecLabels)
            finalHTML += builder.buildHtml(label) + '\n';

        std::cout << finalHTML;
    }
    {
        std::string finalHTML;
        auto caller = [](ILabel auto& l) -> std::string { return l.buildHtml(); };
        for (auto& label : vecLabels)
            finalHTML += std::visit(caller, label) + '\n';

        std::cout << finalHTML;
    }
}
```

### polymorphism by `std::variant` with concept

```cpp
#include <fmt/core.h>

#include <concepts>
#include <string>
#include <variant>
#include <vector>

struct TickData {
    double open;
    double high;
    double low;
    double last;
};

// Define a concept for factor types
template <typename T>
concept TickFactorLike = requires(T t, const TickData& quote) {
    { t.factor } -> std::same_as<double&>;
    { t.name } -> std::same_as<std::string&>;
    t.update(quote);
};

struct Factor01 {
    double feature{};
    std::string name{"Factor01"};
    void update(TickData const& quote) {
        feature = quote.low;
    }
};

struct Factor02 {
    double factor{};
    std::string name{"Factor02"};
    void update(TickData const& quote) {
        factor = quote.last;
    }
};

struct Factor03 {
    double factor{};
    std::string name{"Factor03"};
    void update(TickData const& quote) {
        factor = quote.high;
    }
};

int main(int argc, char const* argv[]) {
    TickData quote{10, 20, 30, 40};

    {
        // // error case
        // using TickFactor = std::variant<Factor01, Factor02, Factor03>;  // error
        // std::vector<TickFactor> vec{Factor01{}, Factor02{}, Factor03{}};

        // // Update factors with a single visitation per factor
        // for (auto&& f : vec) {
        //     std::visit([&quote](auto& factor) { factor.update(quote); }, f);
        // }

        // // Print factors with a single visitation per factor
        // for (auto&& f : vec) {
        //     std::visit([](auto& factor) { fmt::print("name={}, factor={}\n", factor.name, factor.factor); }, f);
        // }
    }
    {
        using TickFactor = std::variant<Factor02, Factor03>;
        std::vector<TickFactor> vec{Factor02{}, Factor03{}};

        // Update factors with a single visitation per factor
        for (auto&& f : vec) {
            // std::visit([&quote](auto& factor) { factor.update(quote); }, f); // mehtod1
            std::visit([&quote](TickFactorLike auto& factor) { factor.update(quote); }, f);  // method2: with concept
        }

        // Print factors with a single visitation per factor
        for (auto&& f : vec) {
            std::visit([](auto& factor) { fmt::print("name={}, factor={}\n", factor.name, factor.factor); }, f);
        }
    }
}
```

## `std::tuple`

print tuple elements by recursive template

```cpp
#include <functional>
#include <iostream>
#include <tuple>

template <typename Tuple, typename Func, size_t Index = 0>
void tuple_for_each(const Tuple& t, const Func& f) {
    constexpr auto n = std::tuple_size_v<Tuple>;
    if constexpr (Index < n) {
        const auto& v = std::get<Index>(t);
        std::invoke(f, v);
        tuple_for_each<Tuple, Func, Index + 1>(t, f);
    }
}

int main(int argc, char const* argv[]) {
    auto tp = std::make_tuple("hello", 100, 3.14);
    tuple_for_each(tp, [](auto const& e) { std::cout << e << '\t'; });
}
```

print tuple elements by `index_sequence`

```cpp
#include <functional>
#include <iostream>
#include <tuple>

// Helper function to iterate using index_sequence
template <typename Tuple, typename Func, std::size_t... Is>
inline void tuple_for_each_impl(Tuple&& t, Func&& f, std::index_sequence<Is...>) {
    (std::invoke(f, std::get<Is>(std::forward<Tuple>(t))), ...);
}

// Main tuple_for_each function
template <typename Tuple, typename Func>
inline void tuple_for_each(Tuple&& t, Func&& f) {
    tuple_for_each_impl(
        std::forward<Tuple>(t),
        std::forward<Func>(f),
        std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tuple>>>{});
}

int main() {
    auto tp = std::make_tuple("hello", 100, 3.14);
    tuple_for_each(tp, [](const auto& e) { std::cout << e << '\t'; });
}
```

print tuple elements with index by `index_sequence`

```cpp
#include <functional>
#include <iostream>
#include <tuple>

// Helper function to iterate using index_sequence
template <typename Tuple, typename Func, std::size_t... Is>
inline void tuple_for_each_impl(Tuple&& t, Func&& f, std::index_sequence<Is...>) {
    // Pass both index and element to the lambda
    (std::invoke(f, Is, std::get<Is>(std::forward<Tuple>(t))), ...);
}

// Main tuple_for_each function
template <typename Tuple, typename Func>
inline void tuple_for_each(Tuple&& t, Func&& f) {
    tuple_for_each_impl(
        std::forward<Tuple>(t),
        std::forward<Func>(f),
        std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tuple>>>{});
}

int main() {
    auto tp = std::make_tuple("hello", 100, 3.14);

    // Lambda now accepts index and element
    tuple_for_each(tp, [](std::size_t index, const auto& element) {
        std::cout << index << "\t" << element << "\n";
    });
}
```

print tuple elements with `std::apply` & modify tuple elements with `std:apply`
> tuple is mutable in C++

```cpp
#include <functional>
#include <iostream>
#include <tuple>

// Optimized tuple_for_each using std::apply and fold expressions
template <typename Tuple, typename Func>
inline void tuple_for_each(Tuple&& t, Func&& f) {
    std::apply(
        [&](auto&&... args) {
            (std::invoke(f, std::forward<decltype(args)>(args)), ...);
        },
        std::forward<Tuple>(t));  // std::forward<Tuple>(t) preserves the value category of t
}

int main() {
    {
        auto tp = std::make_tuple("hello", 100, 3.14);
        tuple_for_each(tp, [](const auto& e) { std::cout << e << '\t'; });
        std::cout << '\n';
    }
    {
        // inplace change the tuple
        auto tp = std::make_tuple(20, 100, 3.14);
        std::get<0>(tp) *= 2;  // tuple is mutable in C++
        std::cout << std::get<0>(tp) << '\n';
        tuple_for_each(tp, [](auto& e) { return e *= 2; });
        tuple_for_each(tp, [](const auto& e) { std::cout << e << '\t'; });
    }
}
```

tuple_for_each not trigger copy

```cpp
#include <functional>
#include <iostream>
#include <tuple>

struct NoCopy {
    NoCopy() = default;
    NoCopy(const NoCopy&) { std::cout << "Copy constructor called!\n"; }
    NoCopy(NoCopy&&) noexcept = default;
    NoCopy& operator=(const NoCopy&) {
        std::cout << "Copy assignment called!\n";
        return *this;
    }
    NoCopy& operator=(NoCopy&&) noexcept = default;
};

template <typename Tuple, typename Func>
inline void tuple_for_each(Tuple&& t, Func&& f) {
    std::apply(
        [&](auto&&... args) {
            (std::invoke(f, std::forward<decltype(args)>(args)), ...);
        },
        std::forward<Tuple>(t));
}

int main() {
    auto tp = std::make_tuple(NoCopy{}, 100, 3.14);
    // pass by reference, not trigger copy
    tuple_for_each(tp, [](const auto& e) { /* std::print is omitted for brevity */ });
}
```

## flexible array

In C99, flexible array(zero-length array) member allow you to define a `struct` with an array member that doesn't have a specified size, enabling **variable-sized objects**.
> it is always placed at the end of the `struct`.

example: why it should be placed at the end of the `struct`?

```cpp
#include <cstdlib>
#include <print>

struct MyStruct1 {
    int a[0];
    int b[10];
};

struct MyStruct2 {
    int b[10];
    // size-zero array always be last field
    int a[0];
};

int main(int argc, char const* argv[]) {
    std::println("{}", sizeof(MyStruct1));  // 40
    std::println("{}", sizeof(MyStruct2));  // 40

    MyStruct1 m1;
    m1.b[0] = 100;
    // a pointered to field b
    std::println("a={}", m1.a[0]);  // a=100

    MyStruct2 m2;
    m2.b[9] = 200;
    // a pointered to field outside of the struct
    std::println("b={}", m2.a[0]);  // b=296923720
}
```

practical example of flexible array member

```cpp
#include <print>

struct MyStruct {
    size_t size;
    int a[0];  // Zero-length array for variable-sized data
};

int main(int argc, char const* argv[]) {
    // Usage
    size_t n = 10;
    auto ptr = malloc(sizeof(MyStruct) + n * sizeof(int));
    auto obj = static_cast<MyStruct*>(ptr);

    obj->size = n;
    // Now, obj->a can be used as an array of 'n' integers
    for (auto i = 0; i < n; ++i) {
        obj->a[i] = i * 100;
    }
    std::println("{}", obj->a[9]);
}
```

## reserve vs resize

- `reserve` won't change size. use `reserse` and `push_back` or `emplace_back`
- `resize` will change size

```cpp
#include <fmt/core.h>

#include <optional>
#include <vector>

int main(int argc, char const *argv[]) {
    std::optional<double> value{100};

    fmt::println("{}", sizeof(value));        // 16 bytes, bool + 7 padding + double
    std::vector<uint8_t> vec(sizeof(value));  // mustn't use .reserve, will change vec size; can use resize

    memcpy(vec.data(), &value, sizeof(value));
    fmt::println("{}", vec.size());

    for (auto &&c : vec) {
        fmt::println("{}", c);
    }
}
```