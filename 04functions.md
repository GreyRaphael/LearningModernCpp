# Functions

- [Functions](#functions)
  - [default \& delete function](#default--delete-function)
  - [Lambda](#lambda)
    - [basic usage](#basic-usage)

## default & delete function

> delete只能放在declaration的地方，不能放在definition的地方；default都行

> [default vs delete](https://www.geeksforgeeks.org/explicitly-defaulted-deleted-functions-c-11/)
- default: 使用compliler默认提供的special member functions(default constructor, destructor, copy constructor,move constructor, copy assignment operator, move assignment operator)
- delete: 不使用某member function

## Lambda

> lambdas are basically syntactic sugar for unnamed function objects, which are classes that implement the call operator

### basic usage

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>

struct foo
{
   int id;
   std::string name;

   auto print()
   {
      return [i = id, n = name]()
      { std::cout << i << ' ' << n << std::endl; };
   }
};

int main()
{
   std::vector<int> v1{1, 2, 3, -3, 5, -1};
   auto positives = std::count_if(
       std::begin(v1),
       std::end(v1),
       [](int const i){ return i > 0; }
   );
   std::cout << positives << std::endl; // 4

   auto negetives = std::count_if(
      std::begin(v1),
      std::end(v1),
      [](auto const i){return i<0;} // auto, since c++14
   );
   std::cout<<negetives<<std::endl; // 2 

   // for pointer
   auto ptr=std::make_unique<int>(28);
   auto func=[lptr=std::move(ptr)](){return ++*lptr;}; // std::move in capture list, since c++14
   std::cout<<*ptr<<std::endl; // error, Segmentation fault
   std::cout<<func()<<std::endl;// 29

   foo obj{10, "grey"};
   auto func=obj.print();
   func(); // 10 grey
}
```

```cpp
#include <iostream>
#include <algorithm>
#include <numeric>

int main()
{
   std::vector<int> v1{1, 2, 3, 4, 5};
   std::vector<std::string> v2{"hello", "world", "china", "today"};

   auto lsum=[](auto const a, auto const b){return a+b;};

   auto s1=std::accumulate(
      std::begin(v1),
      std::end(v1),
      0,
      lsum
   );
   std::cout<<s1<<std::endl; // 15

   auto s2=std::accumulate(
      std::begin(v2),
      std::end(v2),
      std::string(""),
      lsum
   );
   std::cout<<s2<<std::endl; // helloworldchinatoday
}
```

