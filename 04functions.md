# Functions

- [Functions](#functions)
  - [default \& delete function](#default--delete-function)
  - [Lambda](#lambda)
    - [basic usage](#basic-usage)
    - [lambda with template](#lambda-with-template)
  - [variadic function arguments](#variadic-function-arguments)

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

### lambda with template

example: make sure that two parameters of the lambda have the same type.

```cpp
#include <iostream>
#include <vector>

int main()
{
   auto func1 = []<typename T>(std::vector<T> const &vec)
   {
      std::cout << std::size(vec) << std::endl;
   };

   std::vector<int> v1{1, 2, 3, 4, 5};
   func1(v1); // 5
   // func1(10); // cannot resolve type int

   auto func2 = []<typename T>(T x, T y){ return x + y; }; // since c++20
   std::cout << func2(10, 20) << std::endl; // 30
   // std::cout << func2(1.0, 20) << std::endl;// error, must be the same type
   std::cout << func2(1.0, 2.0) << std::endl; // 3.0
}
```

example: when you need to know the type of a parameter so that you can create instances of that type or invoke static members of it.

```cpp
// before c++20
#include <iostream>

struct foo
{
   foo(int x){std::cout << "call foo-"<< x << std::endl;}
   static void static_func(){std::cout << "message" << std::endl;}
};

int main()
{
   auto func=[](auto x){
      using T=std::decay_t<decltype(x)>;
      T other{20};
      T::static_func();
   };

   foo obj1{10};
   func(obj1);
}
```

```cpp
// since c++20
#include <iostream>

struct foo
{
   foo(int x){std::cout << "call foo-"<< x << std::endl;}
   static void static_func(){std::cout << "message" << std::endl;}
};

int main()
{
   auto func=[]<typename T>(T x){
      T other{20};
      T::static_func();
   };

   foo obj1{10};
   func(obj1);
}
```

example: perfect forwarding

[reference collapsing rules](http://thbecker.net/articles/rvalue_references/section_08.html):
- A& & becomes A&: reference to A& is A&
- A& && becomes A&: 
- A&& & becomes A&: 
- A&& && becomes A&&: 

```cpp
#include <iostream>

void myFunction(int &&myNum)
{
   std::cout << "rvalue reference overload: myFunction(int&&): " << myNum << std::endl;
}
void myFunction(int &myNum)
{
   std::cout << "lvalue reference overload: myFunction(int&): " << myNum << std::endl;
}

auto l1 = [](auto &&myNum)
{ myFunction(std::forward<decltype(myNum)>(myNum)); };

// since c++20
auto l2 = []<typename T>(T &&myNum)
{ myFunction(std::forward<T>(myNum)); };

int main()
{
   int someNum{121};          // lvalue
   int &anotherNum = someNum; // lvalue reference

   l1(someNum);               // calls lvalue reference overload
   l1(anotherNum);            // calls lvalue reference overload
   l1(2);                     // calls rvalue reference overload
   l1(std::move(anotherNum)); // calls rvalue reference overload

   l2(someNum);               // calls lvalue reference overload
   l2(anotherNum);            // calls lvalue reference overload
   l2(2);                     // calls rvalue reference overload
   l2(std::move(anotherNum)); // calls rvalue reference overload
}
```

```cpp
#include <iostream>

template<typename... T>
void foo(T&&... args){
    std::cout<<sizeof...(args)<<std::endl;
}

auto func1=[](auto&& ...args){
    return foo(std::forward<decltype(args)>(args)...);
};

// since c++20
auto func2=[]<typename... T>(T&&... args){
    return foo(std::forward<T>(args)...);
};

int main()
{
    foo(1, 2.1, "hello");//3
    func1(1, 2, 2.1, "world", "today"); //5
    func2(1, 2.1, "world", "today"); //4
}
```

## variadic function arguments

```cpp
#include <iostream>

template <typename T>
T add(T value){return value;}

template <typename T, typename... Ts>
// T add(T first, Ts... rest){
auto add(T first, Ts... rest){
   return first+add(rest...);
}

int main()
{
   std::cout << add(1, 2, 3) << std::endl;
   using namespace std::string_literals;
   std::cout << add("hello"s, " "s, "world"s) << std::endl; // 当没有char类型的时候，可以使用 T add
   std::cout << add("hello"s, ' ', "world"s) << std::endl; // 当有char类型的时候，应该使用auto add
}
```

```cpp
#include <iostream>

template <typename... T>
auto make_even_tuple(T... args){
   static_assert(sizeof...(args) % 2 ==0, "expected an even number of arguments");
   std::tuple<T...> t{args...};
   return t;
}

int main()
{
   auto t1=make_even_tuple(1, 2.2, 3, "hello");
//    auto t1=make_even_tuple(1, 2.2, "hello"); // error

   auto [a, b, c, d]=t1;
   std::cout<<std::get<1>(t1)<<std::endl;// 2.2
   std::cout<<d<<std::endl; // hello
}
```

example: fold expression to simplify variadict function template

```cpp
#include <iostream>

template <typename... Ts>
auto add(Ts... args){
   return (args + ...);
}

int main()
{
   std::cout << add(1, 2, 3) << std::endl;
   using namespace std::string_literals;
   std::cout << add("hello"s, ' ', "world"s) << std::endl; 
}
```

example: custom wrapper for fold expression

```cpp
#include <iostream>

template <typename... Ts>
constexpr auto min(Ts... args){
   return (args < ...); // warning, unsafe use of type 'bool' in operation
}

int main()
{
    // return is bool
   std::cout << min(1, 2, 3) << std::endl; // false
   std::cout << min(3, 2, 1) << std::endl; // false
}
```

```cpp
#include <iostream>

template<typename T>
struct wrapper{
    T const & value;
};

template<typename T>
constexpr auto operator<(wrapper<T> const & lhs, wrapper<T> const & rhs){
    return wrapper<T>{lhs.value<rhs.value ? lhs.value : rhs.value};
}

template<typename... Ts>
constexpr auto min(Ts&&... args){
    return (wrapper<Ts>(args) < ...).value;
}

int main()
{
    // return is int
   std::cout << min(2, 3, 4) << std::endl; // 2
   std::cout << min(4, 3, 2) << std::endl; // 2
}
```