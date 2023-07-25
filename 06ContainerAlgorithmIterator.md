# Standard Library Containers, Algorithms, and Iterators

- [Standard Library Containers, Algorithms, and Iterators](#standard-library-containers-algorithms-and-iterators)
  - [`std::vector`](#stdvector)
    - [basic operation](#basic-operation)
    - [vector with object](#vector-with-object)
  - [sort](#sort)
  - [`find`](#find)
  - [Initializing a range](#initializing-a-range)
  - [custom random-access iterator](#custom-random-access-iterator)

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
