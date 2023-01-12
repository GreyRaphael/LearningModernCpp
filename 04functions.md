# Functions

- [Functions](#functions)
  - [default \& delete function](#default--delete-function)
  - [Lambda](#lambda)
    - [basic usage](#basic-usage)
    - [lambda with template](#lambda-with-template)
  - [variadic function arguments](#variadic-function-arguments)
  - [high-order functions](#high-order-functions)
    - [mapf](#mapf)
    - [foldl and foldr](#foldl-and-foldr)
    - [high-order functions combination](#high-order-functions-combination)
  - [`std::invoke`](#stdinvoke)

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

## high-order functions

- map: 对容器里面每一个元素调用一下func
- fold: applies a combining function to the elements of the range to produce a single result.

### mapf

```cpp
#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <queue>

// range
template <typename F, typename R>
R mapf(F&& func, R range){
    std::transform(
        std::begin(range), std::end(range),
        std::begin(range),
        std::forward<F>(func)
    );
    return range;
}

// map
template <typename F, typename T, typename U>
auto mapf(F&& func, std::map<T,U> const & m){
    std::map<T,U> r; // new map
    for(auto&& i: m){r.insert(func(i));}
    return r;
}

// queue
template <typename F, typename T>
auto mapf(F&& func, std::queue<T> q){
    std::queue<T> r; // new queue
    while(!q.empty()){
        r.push(func(q.front()));
        q.pop(); // return is void
    }
    return r;
}

int main()
{
    // range
    std::vector<int> v1{1, -2, 3, -4};
    auto lfunc1=[](int const i){ return std::abs(i);}; // lambda
    auto lfunc2=[](int const i){ return i*i;}; 
    auto result1=mapf(lfunc1, v1);
    auto result2=mapf(lfunc2, v1);
    for(auto&& i:result1){std::cout<<i<<',';} // 1,2,3,4,
    for(auto&& i:v1){std::cout<<i<<',';} // 1,-2,3,-4
    for(auto&& i:result2){std::cout<<i<<',';} // 1,4,9,16,
    for(auto&& i:v1){std::cout<<i<<',';} // 1,-2,3,-4

    // map
    std::map<std::string, int> dict{{"one", 1}, {"two", 2}, {"three", 3}};
    auto lfunc3=[](auto const kv){
        return std::make_pair(
            mapf(std::toupper, kv.first), 
            kv.second
        );
    };
    auto result3=mapf(lfunc3, dict);
    for(auto&& kv:result3){std::cout<<kv.first<<':'<<kv.second<<' ';} // ONE:1 THREE:3 TWO:2 

    // queue
    std::queue<int> q({11, 22, 33, 44}); // 也可以q.push逐个加入
    auto lfunc4=[](auto const i){return i>30? 2:1;};
    auto result4=mapf(lfunc4, q);
    while(!result4.empty()){
        std::cout<<result4.front()<<' ';
        result4.pop();
    } // 1 1 2 2
}
```

example: specify type

```cpp
#include <iostream>
#include <algorithm>
#include <vector>

template <typename F, typename R>
R mapf(F&& func, R range){
    std::transform(
        std::begin(range), std::end(range),
        std::begin(range),
        std::forward<F>(func)
    );
    return range;
}

template<class T = double>
struct fround
{
    typename std::enable_if_t<std::is_floating_point_v<T>, T>
    // function object
    operator()(const T& value) const
    {
        return std::round(value);
    }
};

int main()
{
    auto lfunc=[](auto const i){return std::round(i);};
    std::vector<double> v1{1.1, 3.3, 5.5, 6.6};

    auto result1=mapf(lfunc, v1);
    for(auto&& i:result1){std::cout<<i<<' ';}

    auto result2=mapf(fround<>(), v1);
    for(auto&& i:result2){std::cout<<i<<' ';}

    // std::vector<float> v2{1.1, 3.3, 5.5, 6.6};
    // auto result3=mapf(fround<>(), v2); // error, conversion from 'double' to 'float', possible loss of data
}
```

### foldl and foldr

```cpp
#include <iostream>
#include <numeric>
#include <vector>
#include <map>
#include <queue>

// range, fold from left to right
template <typename F, typename R, typename T>
constexpr T foldl(F&& func, R&& range, T first){ // R&&是为了后面的chars[], R&也行
    return std::accumulate(
        std::begin(range), std::end(range),
        std::move(first),
        std::forward<F>(func)
    );
}

// range, fold from right to left
template <typename F, typename R, typename T>
constexpr T foldr(F&& func, R&& range, T first){
    return std::accumulate(
        std::rbegin(range), std::rend(range),
        std::move(first),
        std::forward<F>(func)
    );
}

template <typename F, typename T>
constexpr T foldl(F&& func, std::queue<T> q, T first){
    while (!q.empty()){
        first=func(first, q.front());
        q.pop();
    }
    return first;
}

int main()
{
    auto lfunc1=[](auto const a, auto const b){
        std::cout<<"a="<<a<<std::endl;
        return a+b;
    };

    std::vector<int> v1{1, -2, 3, -4};
    auto result1=foldl(lfunc1, v1, 0);
    std::cout<<result1<<std::endl;// -2

    std::vector<int> v2{1, -2, 3, -4};
    auto result2=foldl(std::plus<>(), v1, 0);
    std::cout<<result2<<std::endl;// -2
    
    std::vector<std::string> v3{"hello", "world", "grey", "what"};
    auto result3=foldl(lfunc1, v3, std::string(""));
    std::cout<<result3<<std::endl;// helloworldgreywhat

    std::vector<std::string> v4{"hello", "world", "grey", "what"};
    auto result4=foldr(lfunc1, v4, std::string("xxx"));
    std::cout<<result4<<std::endl;// xxxwhatgreyworldhello

    char chars[]{'h','e', 'l', 'l', 'o'};
    auto str1=foldl(lfunc1, chars, std::string(""));
    std::cout<<str1<<std::endl; // hello

    std::map<std::string, int> dict{{"one", 1}, {"two", 2}, {"three", 3}};
    auto lfunc2=[](int const sum, std::pair<std::string, int> const kv){
        std::cout<<"sum="<<sum<<std::endl;
        return sum+kv.second;
    };
    auto count=foldl(lfunc2, dict, 0);
    std::cout<<count<<std::endl; // 6

    // queue
    std::queue<int> q({11, 22, 33, 44});
    auto result5=foldl(lfunc1, q, 0);
    std::cout<<result5<<std::endl;// 110
}
```

example: cascade `mapf`, `foldf`

```cpp
#include <iostream>
#include <numeric>
#include <algorithm>
#include <vector>

template <typename F, typename R>
R mapf(F&& func, R range){
    std::transform(
        std::begin(range), std::end(range),
        std::begin(range),
        std::forward<F>(func)
    );
    return range;
}

template <typename F, typename R, typename T>
constexpr T foldl(F&& func, R&& range, T first){
    return std::accumulate(
        std::begin(range), std::end(range),
        std::move(first),
        std::forward<F>(func)
    );
}

int main()
{
    std::vector v1{1, 2, 3, -1, -2, -3};
    auto lfunc1=[](auto const i){ return i*i;};
    auto lfunc2=[](auto const i){ return std::abs(i);};

    auto result=foldl(
        std::plus<>(), 
        mapf(lfunc1, mapf(lfunc2, v1)
        ),
        0
    );
    std::cout<<result<<std::endl; // 28
}
```

example: variadic `fold`

```cpp
#include <iostream>

template <typename F, typename T1, typename T2>
auto foldl(F&& func, T1 v1, T2 v2){
    return func(v1, v2);
}

template <typename F, typename T, typename... Ts>
auto foldl(F&& func, T head, Ts... rest){
    return func(head, foldl(std::forward<F>(func), rest...));
}

int main()
{
    auto result1=foldl(std::plus<>(), 1, 2, 3, 4);
    std::cout<<result1<<std::endl; // 10
    
    auto result2=foldl(std::plus<>(), 1.1, 2, 3, 4);
    std::cout<<result2<<std::endl; // 10.1
   
    // auto result3=foldl(std::plus<>(), 1);
    // std::cout<<result3<<std::endl; // error, too few arguments
}
```

### high-order functions combination

example: combine 2 functions

```cpp
#include <iostream>
#include <string>

template <typename F, typename G>
auto compose(F&& f, G&& g){
    return [&](auto x){return f(g(x));};
}

int main()
{
    auto v=compose(
        [](int const n){return std::to_string(n);},
        [](int const n){return n*n;}
    );
    std::cout<<v(3)<<std::endl; // 9
}
```

example: combine functions recursively

```cpp
#include <iostream>
#include <string>

template <typename F, typename G>
auto compose(F&& f, G&& g){
    return [&](auto x){return f(g(x));};
}

template <typename F, typename... R>
auto compose(F&& f, R&& ... r){
    auto func=[&](auto x){return f(compose(r...)(x));};
    return func;
}

int main()
{
    auto v=compose(
        [](int const n){return std::to_string(n);},
        [](int const n){return n*n;},
        [](int const n){return n+n;},
        [](int const n){return std::abs(n);}
    );
    std::cout<<v(-3)<<std::endl; // 36
}
```

example: lambda combinations

```cpp
#include <iostream>
#include <numeric>
#include <algorithm>
#include <vector>

template <typename F, typename R>
R mapf(F&& func, R range){
    std::transform(
        std::begin(range), std::end(range),
        std::begin(range),
        std::forward<F>(func)
    );
    return range;
}

template <typename F, typename R, typename T>
constexpr T foldl(F&& func, R&& range, T first){
    return std::accumulate(
        std::begin(range), std::end(range),
        std::move(first),
        std::forward<F>(func)
    );
}

template <typename F, typename G>
auto compose(F&& f, G&& g){
    return [&](auto x){return f(g(x));};
}

template <typename F, typename... R>
auto compose(F&& f, R&& ... r){
    auto func=[&](auto x){return f(compose(r...)(x));};
    return func;
}

int main()
{
    auto v=compose(
        [](auto const v){return foldl(std::plus<>(), v, 0);},
        [](auto const v){return mapf(
            [](auto const i){ return i*i;}, v);}, // lambda in lambda
        [](auto const v){return mapf(
            [](auto const i){ return std::abs(i);}, v);}
    );
    std::vector v1{1, 2, 3, -1, -2, -3};
    std::cout<<v(v1)<<std::endl; // 28
}
```

example: lambda combinations with `operator*`

```cpp
#include <iostream>
#include <numeric>
#include <algorithm>
#include <vector>
#include <string>

template <typename F, typename R>
R mapf(F&& func, R range){
    std::transform(
        std::begin(range), std::end(range),
        std::begin(range),
        std::forward<F>(func)
    );
    return range;
}

template <typename F, typename R, typename T>
constexpr T foldl(F&& func, R&& range, T first){
    return std::accumulate(
        std::begin(range), std::end(range),
        std::move(first),
        std::forward<F>(func)
    );
}

template <typename F, typename G>
auto compose(F&& f, G&& g){
    return [&](auto x){return f(g(x));};
}

template <typename F, typename... R>
auto compose(F&& f, R&& ... r){
    auto func=[&](auto x){return f(compose(r...)(x));};
    return func;
}

// new operator
template <typename F, typename G>
auto operator*(F&& f, G&& g){
    return compose(std::forward<F>(f), std::forward<G>(g));
}

template <typename F, typename... R>
auto operator*(F&& f, R&&... r){
    return operator*(std::forward<F>(f), r...);
}

int main()
{
    auto v=[](auto const v){return foldl(std::plus<>(), v, 0);}
            *
           [](auto const v){return mapf(
                [](auto const i){ return i*i;}, v);}
            *
           [](auto const v){return mapf(
            [](auto const i){ return std::abs(i);}, v);};
    std::vector v1{1, 2, 3, -1, -2, -3};
    std::cout<<v(v1)<<std::endl; // 28

    auto n= [](auto const n){return std::to_string(n);} *
            [](auto const n){return n+n;} *
            [](auto const n){return std::abs(n);};
    std::cout<<n(-3)<<std::endl;// 6
}
```

## `std::invoke`

>  useful in template metaprogramming for implementing various library functions

examle: call function methods

```cpp
#include <iostream>
#include <functional>

int add(int const a, int const b) { return a + b; }

struct foo
{
   int x = 0;
   void increment_by(int const n){x += n;}
   void msg(){std::cout << "x=" << x << std::endl;}
};

int main()
{
    // direct call
    auto a1=add(1, 2);
    
    // by pointer
    int (*padd1)(int const, int const)=&add;
    auto a2=padd1(2, 3);
    // auto pointer
    auto padd2=&add;
    auto a3=padd2(3, 4);

    foo f;
    f.increment_by(100);
    auto x1=f.x; // 100

    // by pointer
    void (foo::*pfunc1)(int const) = &foo::increment_by;
    (f.*pfunc1)(10);
    std::cout<<f.x<<std::endl; // 110

    // auto pointer
    auto pfunc2=&foo::increment_by;
    (f.*pfunc2)(3);
    std::cout<<f.x<<std::endl; // 113
}
```

example: call function by `std::invoke`

```cpp
#include <iostream>
#include <functional>

int add(int const a, int const b) { return a + b; }

struct foo
{
   int x = 0;
   void increment_by(int const n){x += n;}
   void msg(){std::cout << "x=" << x << std::endl;}
};

int main()
{
   std::cout << std::invoke(add, 1, 2) << std::endl;
   std::cout << std::invoke(&add, 1, 2) << std::endl;

   int (*fadd)(int const, int const) = &add;
   std::cout << std::invoke(fadd, 1, 2) << std::endl;

   foo f1;
   std::invoke(&foo::increment_by, f1, 10); 
   f1.msg(); // x=10

   foo f2;
   auto x2 = std::invoke(&foo::x, f2);
   f2.msg(); // x=0
   std::cout<<x2<<std::endl;// 0

   // function obj
   foo f3;
   auto x3=std::invoke(std::plus<>(), std::invoke(&foo::x, f3), 11);
   std::cout<<x3<<std::endl;

   // lambda
   auto lfunc=[](auto const a, auto const b){return a+b;};
   auto x4=std::invoke(lfunc, 2, 4);// 6
}
```