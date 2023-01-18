# Standard Library Containers, Algorithms, and Iterators

- [Standard Library Containers, Algorithms, and Iterators](#standard-library-containers-algorithms-and-iterators)
  - [`std::vector`](#stdvector)
  - [sort](#sort)

C++ Standard Library core initially sat three main pillars: **containers**, **algorithms**,
and **iterators**

## `std::vector`

example: ctor & destructor

```cpp
#include <iostream>

struct Item
{
   int x;

   Item() : x(0) { std::cout << "default ctor\n"; }
   Item(int a) : x(a) { std::cout << "simple ctor:"<< x << " at " << &(*this) << std::endl; }
   Item(const Item &rhs) : x(rhs.x) { std::cout << "copy ctor:"<< x << " at " << &(*this) << std::endl; }
   Item(Item &&rhs) : x(rhs.x) { std::cout << "move ctor:"<< x << " at " << &(*this) << std::endl; }
   ~Item() { std::cout << "destructor:"<< x << " at " << &(*this) << std::endl; }
};

int main()
{
    Item a{10};
    {
        Item b{std::move(a)};
    } // 出了这个block, b失效，控制权重新回到a
    std::cout<<a.x<<std::endl;
} 
```

```bash
# output
simple ctor:10 at 0x7ffeb2ff8ab0
move ctor:10 at 0x7ffeb2ff8ab4
destructor:10 at 0x7ffeb2ff8ab4
10
destructor:10 at 0x7ffeb2ff8ab0
```

push_back, emplace_back
- push_back: 要调用构造函数和move构造函数。要先构造一个临时对象，然后把临时对象move到容器最后面
- emplace_back: 直接在vector原地构造对象


```cpp
#include <iostream>
#include <vector>

struct Item
{
   int x;

   Item() : x(0) { std::cout << "default ctor\n"; }
   Item(int a) : x(a) { std::cout << "simple ctor:"<< x << " at " << &(*this) << std::endl; }
   Item(const Item &rhs) : x(rhs.x) { std::cout << "copy ctor:"<< x << " at " << &(*this) << std::endl; }
   Item(Item &&rhs) : x(rhs.x) { std::cout << "move ctor:"<< x << " at " << &(*this) << std::endl; }
   ~Item() { std::cout << "destructor:"<< x << " at " << &(*this) << std::endl; }
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
```

```bash
# output
simple ctor:1 at 0x7ffe897c534c
copy ctor:1 at 0x1bd2eb0
copy ctor:1 at 0x1bd2eb4
origin capacity: 5

call emplace_back:
simple ctor:2 at 0x1bd2eb8
final capacity: 5

call push_back:
simple ctor:3 at 0x7ffe897c5374
move ctor:3 at 0x1bd2ebc
destructor:3 at 0x7ffe897c5374
final capacity: 5
1
1
2
3
destructor:1 at 0x7ffe897c534c
destructor:1 at 0x1bd2eb0
destructor:1 at 0x1bd2eb4
destructor:2 at 0x1bd2eb8
destructor:3 at 0x1bd2ebc
```

example: ctor with two-arguments

```cpp
#include <iostream>
#include <vector>

struct Item
{
   int x;
   double y;

   Item() : x(0),y(0) { std::cout << "default ctor\n"; }
   Item(int a, double b) : x(a), y(b) { std::cout << "simple ctor:"<< x << " at " << &(*this) << std::endl; }
   Item(const Item &rhs) : x(rhs.x), y(rhs.y) { std::cout << "copy ctor:"<< x << " at " << &(*this) << std::endl; }
   Item(Item &&rhs) : x(rhs.x), y(rhs.y) { std::cout << "move ctor:"<< x << " at " << &(*this) << std::endl; }
   ~Item() { std::cout << "destructor:"<< x << " at " << &(*this) << std::endl; }
};

int main()
{
   std::vector<Item> v1;
   v1.reserve(5); 

    // v1.push_back(1, 2.2); // error
    v1.push_back({1, 2.5});
    std::cout<<std::endl;
    
    v1.emplace_back(11, 11.5);
    // v1.emplace_back({11, 22.5}); // error
    std::cout<<std::endl;
}

// simple ctor:1 at 0000001CDDEFFA38
// move ctor:1 at 00000158F02627F0
// destructor:1 at 0000001CDDEFFA38

// simple ctor:11 at 00000158F02627F8

// destructor:1 at 00000158F02627F0
// destructor:11 at 00000158F02627F8
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