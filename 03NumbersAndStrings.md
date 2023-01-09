# Numbers and Strings

- [Numbers and Strings](#numbers-and-strings)
  - [built-in literals](#built-in-literals)
  - [custom literals](#custom-literals)
    - [raw user-defined literals](#raw-user-defined-literals)
  - [raw string literals](#raw-string-literals)
    - [custom string with many functions](#custom-string-with-many-functions)
  - [string\_view](#string_view)
  - [std::format](#stdformat)
    - [basic usage](#basic-usage)
    - [std::format with user-defined types](#stdformat-with-user-defined-types)
      - [user-defined formatter with single value](#user-defined-formatter-with-single-value)
      - [user-defined formatter with multiple values](#user-defined-formatter-with-multiple-values)

## built-in literals

```cpp
#include<iostream>
#include<chrono>
#include<string>

int main(){
    auto a{42u}; // unsigned int
    auto b{42l}; // unsigned long
    auto c{42.4f}; // float

    using namespace std::chrono_literals;
    auto my_timer{2h+42min+15s+1ms}; // std::chrono::duration<long long>
    std::cout<<my_timer.count()<<std::endl; // 9735001

    using namespace std::string_literals;
    auto str1="你好"s;// std::string
    std::cout<<str1.length()<<std::endl; // 6
    std::cout<<str1.size()<<std::endl; // 6
}
```

## custom literals

```cpp
namespace units{
    // unit types
    enum class unit{us, ms, s};

    // quantity class
    template <unit U>
    class quantity{
        const double amount;
    public:
        constexpr explicit quantity(double const a):amount(a){}
        explicit operator double() const { return amount; } // 为了static_cast能够成功
    };

    // template operator overload
    template <unit U>
    constexpr quantity<U> operator+ (quantity<U> const &q1, quantity<U> const &q2){
        return quantity<U>(static_cast<double>(q1) + static_cast<double>(q2));
    }

    // unit literals
    namespace unit_literals{
        // for us
        constexpr quantity<unit::us> operator "" _us(long double const amount)
        {
            return quantity<unit::us> {static_cast<double>(amount)};
        }
        constexpr quantity<unit::us> operator "" _us(unsigned long long const amount)
        {
            return quantity<unit::us> {static_cast<double>(amount)};
        }
        // for ms
        constexpr quantity<unit::ms> operator "" _ms(long double const amount)
        {
            return quantity<unit::ms> {static_cast<double>(amount)};
        }
        // for s
        constexpr quantity<unit::s> operator "" _s(unsigned long long const amount)
        {
            return quantity<unit::s> {static_cast<double>(amount)};
        }
    }
}


int main(){
    using namespace units::unit_literals;
    auto q1{1_us};
    auto q2{2_us};
    auto q3{3.0_ms};
    auto q4{q1+q2};
    
    // auto q5{q1+q3}; // error
    // auto q6{6_ms}; // error, ms must be double
    // auto q7{7.0_s}; // error, s must be integer
}
```

### raw user-defined literals

version1: 无法解决`2010_b8`中2的约束，比较直接的实现

```cpp
#include <iostream>

namespace binary
{
    using byte8 = unsigned char;
    namespace binary_literals
    {
        namespace binary_literals_internals
        {
            template <typename T, typename... Ts>
            // 原理: 8*t0+4*t1+2*t2+1*t3
            byte8 func(T t0, Ts... t)
            {
                auto const sz = sizeof...(t);
                if constexpr (sz == 0) // c++17
                {
                    return t0 - '0';
                }
                else
                {
                    auto const value = (1 << sz) * (t0 - '0');
                    std::cout << "t0=" << t0 << ',' << value << std::endl;
                    return value + func(t...);
                }
            }
        }

        template <char... bits>
        constexpr byte8 operator""_b8()
        {
            static_assert(
                sizeof...(bits) <= 8,
                "binary literal b8 must be up to 8 digits long");
            // binary_literals_internals得放在前面
            return binary_literals_internals::func(bits...); 
        }
    }
}

int main()
{
    // using namespace binary;
    // using namespace binary_literals;
    using namespace binary::binary_literals;

    auto b1 = 1010_b8;
    std::cout << (int)b1 << std::endl; // 10

    auto b2 = 2010_b8;
    std::cout << (int)b2 << std::endl; // 18
}
```

```cpp
// modification1:
// 原理: 8*t0+4*t1+2*t2+1*t3
template <typename... Ts> // 因为每一个digit都是char, 所以去掉T
byte8 func(char t0, Ts... t)
{
    if (t0!='0' &&  t0!='1')
    {
        throw std::invalid_argument("digit must be 0 or 1");
    }
    
    auto const sz = sizeof...(t);
    if constexpr (sz == 0) // c++17
    {
        return t0 - '0';
    }
    else
    {
        auto const value = (1 << sz) * (t0 - '0');
        std::cout << "t0=" << t0 << ',' << value << std::endl;
        return value + func(t...);
    }
}
```

```cpp
// modification2:
// 原理: 位运算
template <typename... Ts>
byte8 func(char t0, Ts... t)
{
    if (t0 != '0' && t0 != '1')
    {
        throw std::invalid_argument("digit must be 0 or 1");
    }

    auto const sz = sizeof...(t);
    if constexpr (sz == 0)
    {
        return t0-'0';
    }
    else
    {
        if (t0 == '0')
        {
            return func(t...);
        }
        else // t0=='1'
        {
            return (1 << sz) | func(t...);
        }
    }
}
```

```cpp
// return type is T
template <typename T, typename... Ts>
T func(int x, Ts... xs)
{
    if constexpr (sizeof...(xs)==0){
        return static_cast<T>(x);
    }
    else{
        return static_cast<T>(x)+func<T>(xs...);
    }
}
```

```cpp
// modification3: 返回值设置T
// 原理: 位运算
template <typename T, typename... Ts>
T func(char t0, Ts... t)
{
    if (t0 != '0' && t0 != '1')
    {
        throw std::invalid_argument("digit must be 0 or 1");
    }

    auto const sz = sizeof...(t);
    if constexpr (sz == 0)
    {
        return t0-'0';
    }
    else
    {
        if (t0 == '0')
        {
            return func<T>(t...);
        }
        else // t0=='1'
        {
            return (1 << sz) | func<T>(t...);
        }
    }
}
```

```cpp
// modification4: declaration, definition分开
// 原理: 位运算
template <typename T>
T func();// declaration

template <typename T, typename... Ts>
T func(char t0, Ts... t)
{
    if (t0 != '0' && t0 != '1')
    {
        throw std::invalid_argument("digit must be 0 or 1");
    }

    if (t0 == '0')
    {
        return func<T>(t...);
    }
    else // t0=='1'
    {
        return (1 << sizeof...(t)) | func<T>(t...);
    }
}

template<typename T>
T func(){return 0;} //definition, 结束递归调用的最后一个
```

> raw user-defined literals by **function**

```cpp
#include <iostream>

namespace binary
{
    using byte8 = unsigned char;
    using byte16 = unsigned short;
    using byte32 = unsigned int;

    namespace binary_literals
    {
        namespace binary_literals_internals
        {
            // 原理: 位运算
            template<typename T>
            T func(){return 0;} //直接definition结束递归调用

            template <typename T, typename... Ts>
            T func(char t0, Ts... t)
            {
                if (t0 != '0' && t0 != '1')
                {
                    throw std::invalid_argument("digit must be 0 or 1");
                }

                if (t0 == '0')
                {
                    return func<T>(t...);
                }
                else // t0=='1'
                {
                    return (1 << sizeof...(t)) | func<T>(t...);
                }
            }
        }

        template <char... bits>
        constexpr byte8 operator""_b8()
        {
            static_assert(
                sizeof...(bits) <= 8,
                "binary literal b8 must be up to 8 digits long");
            return binary_literals_internals::func<byte8>(bits...);
        }

        template <char... bits>
        constexpr byte16 operator""_b16()
        {
            static_assert(
                sizeof...(bits) <= 16,
                "binary literal b16 must be up to 16 digits long");
            return binary_literals_internals::func<byte16>(bits...);
        }

        template <char... bits>
        constexpr byte32 operator""_b32()
        {
            static_assert(
                sizeof...(bits) <= 32,
                "binary literal b32 must be up to 32 digits long");
            return binary_literals_internals::func<byte32>(bits...);
        }
    }
}

int main()
{
    using namespace binary::binary_literals;

    auto b1 = 1010_b8;
    std::cout << (int)b1 << std::endl; // 10

    auto b2 = 1_b8;
    std::cout << (int)b2 << std::endl; // 1

    // auto b3 = 2010_b8;
    // std::cout << (int)b3 << std::endl; // error

    auto b4 = 101010101010_b16;
    std::cout << b4 << std::endl; //2730

    auto b5 = 10101010101010101010101_b32;
    std::cout << b5 << std::endl; //5592405
}
```

> raw user-defined literals by **struct**

```cpp
#include <iostream>

namespace binary
{
    using byte8 = unsigned char;
    using byte16 = unsigned short;
    using byte32 = unsigned int;

    namespace binary_literals
    {
        namespace binary_literals_internals
        {
            template <typename CharT, char... bits>
            struct binary_struct; // declaration, 必须有

            template <typename CharT>
            struct binary_struct<CharT> // empty argument
            {
                static constexpr CharT value{0}; // 递归结束条件，采用了静态成员变量
            };

            template <typename CharT, char... bits>
            struct binary_struct<CharT, '0', bits...> // specification, 0, x, x, x...
            {
                static constexpr CharT value{binary_struct<CharT, bits...>::value};
            };

            template <typename CharT, char... bits>
            struct binary_struct<CharT, '1', bits...> // specification, 1, x, x, x...
            {
                static constexpr CharT value{static_cast<CharT>(1 << sizeof...(bits)) | binary_struct<CharT, bits...>::value};
            };
        }

        template <char... bits>
        constexpr byte8 operator""_b8()
        {
            static_assert(sizeof...(bits) <= 8, "binary literal b8 must be up to 8 digits long");
            return binary_literals_internals::binary_struct<byte8, bits...>::value;
        }

        template <char... bits>
        constexpr byte16 operator""_b16()
        {
            static_assert(sizeof...(bits) <= 16, "binary literal b16 must be up to 16 digits long");
            return binary_literals_internals::binary_struct<byte16, bits...>::value;
        }

        template <char... bits>
        constexpr byte32 operator""_b32()
        {
            static_assert(sizeof...(bits) <= 32, "binary literal b32 must be up to 32 digits long");
            return binary_literals_internals::binary_struct<byte32, bits...>::value;
        }
    }
}

int main()
{
    using namespace binary;
    using namespace binary_literals;

    auto b1 = 1010_b8;
    // auto b2 = 101010101010_b16;
    // auto b3 = 10101010101010101010101_b32;
    std::cout << b1 << std::endl;
    // std::cout << b2 << std::endl;
    // std::cout << b3 << std::endl;
}
```

## raw string literals

```cpp
#include<iostream>

int main(){
    using namespace std::string_literals;

    auto filename{ R"(C:\Users\Grey\你好\)"s };
    auto s1{ u8R"(C:\Users\Grey\你好\)"s }; // utf8 raw string
    std::cout<<filename<<std::endl;
    std::cout<<s1<<std::endl;
}
```

### custom string with many functions

```cpp
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>

namespace string_library
{
    template <typename CharT>
    using tstring = std::basic_string<CharT, std::char_traits<CharT>, std::allocator<CharT>>;

    template <typename CharT>
    using tstringstream = std::basic_stringstream<CharT, std::char_traits<CharT>, std::allocator<CharT>>;

    template <typename CharT>
    inline tstring<CharT> to_upper(tstring<CharT> text)
    {
        std::transform(std::begin(text), std::end(text), std::begin(text), toupper);
        return text;
    }

    template <typename CharT>
    inline tstring<CharT> to_lower(tstring<CharT> text)
    {
        std::transform(std::begin(text), std::end(text), std::begin(text), tolower);
        return text;
    }

    template <typename CharT>
    inline tstring<CharT> reverse(tstring<CharT> text)
    {
        std::reverse(std::begin(text), std::end(text));
        return text;
    }

    template <typename CharT>
    inline tstring<CharT> trim(tstring<CharT> const &text)
    {
        auto first{text.find_first_not_of(' ')};
        auto last{text.find_last_not_of(' ')};
        return text.substr(first, (last - first + 1));
    }

    template <typename CharT>
    inline tstring<CharT> trimleft(tstring<CharT> const &text)
    {
        auto first{text.find_first_not_of(' ')};
        return text.substr(first, text.size() - first);
    }

    template <typename CharT>
    inline tstring<CharT> trimright(tstring<CharT> const &text)
    {
        auto last{text.find_last_not_of(' ')};
        return text.substr(0, last + 1);
    }

    template <typename CharT>
    inline tstring<CharT> trim(tstring<CharT> const &text, tstring<CharT> const &chars)
    {
        auto first{text.find_first_not_of(chars)};
        auto last{text.find_last_not_of(chars)};
        return text.substr(first, (last - first + 1));
    }

    template <typename CharT>
    inline tstring<CharT> trimleft(tstring<CharT> const &text, tstring<CharT> const &chars)
    {
        auto first{text.find_first_not_of(chars)};
        return text.substr(first, text.size() - first);
    }

    template <typename CharT>
    inline tstring<CharT> trimright(tstring<CharT> const &text, tstring<CharT> const &chars)
    {
        auto last{text.find_last_not_of(chars)};
        return text.substr(0, last + 1);
    }

    template <typename CharT>
    inline tstring<CharT> remove(tstring<CharT> text, CharT const ch)
    {
        auto start = std::remove_if(std::begin(text), std::end(text), [=](CharT const c)
                                    { return c == ch; });
        text.erase(start, std::end(text));
        return text;
    }

    template <typename CharT>
    inline std::vector<tstring<CharT>> split(tstring<CharT> text, CharT const delimiter)
    {
        auto sstr = tstringstream<CharT>{text};
        auto tokens = std::vector<tstring<CharT>>{};
        auto token = tstring<CharT>{};
        while (std::getline(sstr, token, delimiter))
        {
            if (!token.empty())
                tokens.push_back(token);
        }
        return tokens;
    }

    // mutable version
    namespace mutable_version
    {
        template <typename CharT>
        inline void to_upper(tstring<CharT> &text)
        {
            std::transform(std::begin(text), std::end(text), std::begin(text), toupper);
        }

        template <typename CharT>
        inline void to_lower(tstring<CharT> &text)
        {
            std::transform(std::begin(text), std::end(text), std::begin(text), tolower);
        }

        template <typename CharT>
        inline void reverse(tstring<CharT> &text)
        {
            std::reverse(std::begin(text), std::end(text));
        }

        template <typename CharT>
        inline void trim(tstring<CharT> &text)
        {
            auto first{text.find_first_not_of(' ')};
            auto last{text.find_last_not_of(' ')};
            text = text.substr(first, (last - first + 1));
        }

        template <typename CharT>
        inline void trimleft(tstring<CharT> &text)
        {
            auto first{text.find_first_not_of(' ')};
            text = text.substr(first, text.size() - first);
        }

        template <typename CharT>
        inline void trimright(tstring<CharT> &text)
        {
            auto last{text.find_last_not_of(' ')};
            text = text.substr(0, last + 1);
        }
    }
    // ----------
}

int main()
{
    using namespace std::string_literals;

    auto str1 = "hello, world"s;
    auto v1 = string_library::split(str1, ',');
    for (auto &&i : v1)
    {
        std::cout << i << ';';
    } // hello; world;

    auto str2 = "hello, grey"s;
    string_library::mutable_version::to_upper(str2);
    std::cout << str2 << std::endl; // HELLO, GREY

    // CharT是为了支持std::string, std::wstring, std::u16string, and std::u32string
    auto str3 = u8"hello, 你好"s;
    auto v2 = string_library::split(str3, ',');
    for (auto &&i : v2)
    {
        std::cout << i << ';';
    } // hello; 你好;
}
```

## string_view

cstring, std::string, std::string_view
- cstring 性能高，不方便操作
- std::string, 每个std::string都独自拥有一份字符串的拷贝
- std::string_view 只是记录了自己对应的字符串的指针和偏移位置

```cpp
typedef basic_string<char> string
typedef basic_string_view<char> string_view
```

```cpp
// std::string constructor
string(); //default ctor
string (const string& str); // copy ctor
string (const string& str, size_t pos, size_t len = npos); // substring
string (const char* s); // from c-str
string (const char* s, size_t n); // from buffer
string (size_t n, char c); // fill
template <class InputIterator>  string  (InputIterator first, InputIterator last); // range
string (initializer_list<char> il); //initializer_list
string (string&& str) noexcept; // move

// std::string_view constructor
string_view() noexcept; // default
string_view(const string_view& other) noexcept;
string_view(const char* s); // from c-str
string_view(const char* s, size_t n); // from buffer
```

```cpp
// std::string操作更容易
#include<iostream>
#include<cstring>
#include<string>

int main(){
    char cstr1[]{'h', 'e', 'l', 'l', 'o', '\0'};
    char cstr2[6];
    strcpy(cstr2, cstr1);
    std::cout<<cstr2<<std::endl;
    
    std::string str1{"world"};
    std::string str2{str1};
    std::cout<<str2<<std::endl;
}
```

```cpp
// std::string_view ctor
#include<iostream>
#include<string>
#include<string_view>

int main(){
    const char* cstr{"hello,world"};
    std::string_view sv1{cstr};
    std::string_view sv2{cstr, 5};
    std::string_view sv3{cstr+6, 5};
    std::cout<<sv1<<'|'<<sv2<<std::endl; // hello,world|hello
    std::cout<<sv1<<'|'<<sv3<<std::endl; // hello,world|world

    std::string str{"hello,world"};
    std::string_view sv4{str.c_str()};
    std::string_view sv5{str.c_str(), 5};
    std::string_view sv6{str.c_str()+6, 5};
    std::string_view sv7{str}; // std::string数据交给std::string_view
    std::cout<<sv4<<'|'<<sv5<<std::endl; // hello,world|hello
    std::cout<<sv4<<'|'<<sv6<<std::endl; // hello,world|world
    std::cout<<sv7<<std::endl; // hello,world
}
```

```cpp
// std::string const &, std::string_view性能比较
#include <iostream>
#include <chrono>
#include <string>
#include <string_view>

class Timer
{
    std::string title;
    std::chrono::high_resolution_clock::time_point m_start, m_stop;

public:
    Timer(const std::string &name) : title{name}
    {
        m_start = std::chrono::high_resolution_clock::now();
    }
    ~Timer() { stop(); }
    void stop()
    {
        m_stop = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(m_stop - m_start);
        std::cout << title << ':' << ms.count() << "ms\n";
    }
};

void func1(std::string const &str1) { str1.find_first_of('i'); } // heap反复创建销毁std::string
void func2(std::string_view sv1) { sv1.find_first_of('i'); } // std::string_view采用值传递不用ref传递

int main()
{
    {
        Timer timer("std::string");
        std::string name{"XiaoMing"};
        for (int i = 0; i < 1e7; i++)
        {
            auto firstName = name.substr(0, 4); // Xiao
            auto lastName = name.substr(4, 4);  // Ming
            func1(firstName);
            func1(lastName);
        }
    } // std::string:368ms
    // 仅仅func1与func2不同
    {
        Timer timer("std::string_view");
        std::string name{"XiaoMing"};
        for (int i = 0; i < 1e7; i++)
        {
            auto firstName = name.substr(0, 4);
            auto lastName = name.substr(4, 4); 
            func2(firstName);// std::string_view能够高效替代std::string const &
            func2(lastName);
        }
    } // std::string:158ms
}
```

```cpp
// std::string创建销毁耗时
auto longStr= std::string(1000, 'a'); // 1000个a组成的字符串
auto longCStr=longStr.c_str();
{
    Timer timer("std::string");
    for (int i = 0; i < 1e7; i++)
    {
        std::string name{longStr}; // 反复创建销毁std::string对象
    }
} // std::string:585ms
{
    Timer timer("std::string_view");
    for (int i = 0; i < 1e7; i++)
    {
        std::string_view name{longCStr};
    }
} // std::string:0ms
```

```cpp
// substr会创建临时std::string对象
{
    Timer timer("std::string");
    std::string name{"XiaoMing"};
    for (int i = 0; i < 1e7; i++)
    {
        // substr过程中反复创建销毁std::string
        auto firstName = name.substr(0, 4); // std::string
        auto lastName = name.substr(4, 4); 
    }
} // std::string:110ms
{
    Timer timer("std::string_view");
    std::string_view name{"XiaoMing"}; // 仅仅初始变量不同
    for (int i = 0; i < 1e7; i++)
    {
        auto firstName = name.substr(0, 4); // std::string_view
        auto lastName = name.substr(4, 4);  
    }
} // std::string:0ms
```

```cpp
// std::string无法使用被销毁的对象
#include <iostream>
#include <string>
#include <string_view>

std::string_view GetStringView()
{
    std::string name = "xunwu";
    // 将std::string交给std::string_view
    return std::string_view(name);// 离开作用域时，name已经被回收销毁
}

int main()
{
    auto stringView = GetStringView();
    std::cout << stringView << std::endl; // 烫烫
}
```

```cpp
// std::string_view特有的两个函数remove_prefix和remove_suffix
#include<iostream>
#include<string>
#include<string_view>

std::string_view trim_view(std::string_view sv){
    auto const pos1{sv.find_first_not_of(" ")};
    auto const pos2{sv.find_last_not_of(" ")};
    std::cout<<pos1<<','<<pos2<<std::endl;
    sv.remove_prefix(pos1); // Moves the start of the view forward by n characters.
    sv.remove_suffix(sv.size()-pos2-1); //Moves the end of the view back by n characters.
    return sv;
}

int main(){
    auto sv1{trim_view("   sample   ")};
    std::string s{sv1}; // from string_view to sting
    std::cout<<s<<std::endl;
    std::cout<<sv1<<std::endl;
}
```

## std::format

> since c++20

### basic usage

```cpp
#include<iostream>
#include<vector>
#include<chrono>
#include<format>

int main(){
    auto t1=std::format("{} is {}\n", "John", 32);
    std::cout<<t1;

    auto t2=std::format("{:08x}\n", 32);
    std::cout<<t2; // 00000020

    int val =10;
    auto t3=std::format("val = {}, &val = {}\n", val, static_cast<void*>(&val)); // 必须转换成void*才能输出地址
    std::cout<<t3; // val = 10, &val = 0xaffc98

    auto now = std::chrono::system_clock::now();
    auto stamp = std::format("Today is {:%Y-%m-%d %X}\n", now);
    std::cout<<stamp;

    auto date=std::chrono::year { 2023 } / std::chrono::month{1} / std::chrono::day{9};
    auto msg = std::format("{:*^10}, {:*>10}, in{}!\n", "hello", "world", date);
    std::cout<<msg;// **hello***, *****world, in2023-01-09!

    std::vector<char> buf;
    // std::format_to(std::front_inserter(buf), "{} is {}", "John", 32); // error, vector has no member named 'push_front'
    std::format_to(std::back_inserter(buf), "{} is {}", "John", 32);
    for(auto&& i:buf){std::cout<<i<<',';}

    auto sz=std::formatted_size("{} is {}", "John", 32);
    std::cout<<sz<<std::endl; // 10

    std::format_to(buf.data(), "{} is {}\n", "John", 23);
    for(auto&& i:buf){std::cout<<i<<',';} // replace data as: J,o,h,n, ,i,s, ,2,3,

    char buf2[6]{0};
    std::format_to_n(buf2, 4, "{}xx", 27);
    std::cout<<std::string(buf2)<<std::endl;// 27xx
}
```

### std::format with user-defined types

> [Custom types and std::format from C++20](https://www.cppstories.com/2022/custom-stdformat-cpp20/)

需要实现`parse`和`format`两个函数

#### user-defined formatter with single value

```cpp
#include<iostream>
#include<format>

struct Index{
    unsigned int id_{0};
};

template<>
struct std::formatter<Index>{
    // ctor
    formatter(){std::cout<<"begin format\n";}

    constexpr auto parse(std::format_parse_context& ctx){
        return ctx.begin();
    }

    auto format(Index const & obj, std::format_context& ctx){
        return std::format_to(ctx.out(), "{}", obj.id_);
    }
};

int main(){
    Index obj{100};
    // 调用1次ctor
    std::cout<<std::format("obj={}\n", obj);// obj=100
    // 调用2次ctor
    std::cout<<std::format("obj={0},{0}\n", obj); // obj=100,100
}
```

```cpp
// The parse() function has to return the iterator to the closing bracket
"{}"       // ctx.begin() points to ctx.end()
"{0}"      // ctx.begin() points to `}`
"{0:d}"    // ctx.begin() points to `d`, begin-end is "d}"
"{:hello}" // ctx.begin points to 'h' and begin-end is "hello}"
```

```cpp
#include<iostream>
#include<format>

struct Index{
    unsigned int id_{0};
};

template<>
// 只有一个参数的class,struct可以继承标准的类型进行输出
struct std::formatter<Index>:std::formatter<int>{ 
    auto format(Index const & obj, std::format_context& ctx){
        return std::formatter<int>::format(obj.id_, ctx);
    }
};

int main(){
    Index obj{200};
    std::cout<<std::format("obj={}\n", obj);// obj=200
    std::cout<<std::format("obj={0},{0}\n", obj); // obj=200,200
    // 继承复杂的format
    std::cout << std::format("obj={:*>11d}\n", obj);//obj=********200
}
```

#### user-defined formatter with multiple values

user-defined without format specifier

```cpp
#include <iostream>
#include <format>

namespace v1
{
    struct employee
    {
        int id;
        std::string firstName;
        std::string lastName;
    };
}

template <>
struct std::formatter<v1::employee>
{
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    auto format(v1::employee const &e, std::format_context &ctx)
    {
        return std::format_to(ctx.out(), "[{}] {} {}", e.id, e.firstName, e.lastName);
    }
};

int main()
{
    v1::employee e1{12, "John", "Doe"};
    auto s1 = std::format("{}", e1);
    // auto s2 = std::format("{:L}", e1); // error, cannot parse L
    std::cout << s1 << std::endl; // [12] John Doe
}
```

```cpp
#include <iostream>
#include <format>
#include <string_view>

namespace v1
{
    struct employee
    {
        int id;
        std::string firstName;
        std::string lastName;
    };
}

template <>
// rely on the predefined string_view formatter
struct std::formatter<v1::employee>:std::formatter<string_view>
{
    auto format(v1::employee const &e, std::format_context &ctx)
    {
        auto temp= std::format("[{}] {} {}", e.id, e.firstName, e.lastName);
        return std::formatter<std::string_view>::format(temp, ctx);
    }
};

int main()
{
    v1::employee e1{12, "John", "Doe"};
    auto s1 = std::format("{}", e1);
    // auto s2 = std::format("{:L}", e1); // error, cannot parse L
    std::cout << s1 << std::endl; // [12] John Doe
}
```

user-defined with format specifier

```cpp
#include <iostream>
#include <format>
#include <string_view>

namespace v2
{
    struct employee
    {
        int id;
        std::string firstName;
        std::string lastName;
    };
}

template <>
struct std::formatter<v2::employee>
{
    bool lexicographic_order = false;

    constexpr auto parse(std::format_parse_context &ctx)
    {
        auto pos = ctx.begin();
        if (pos!=ctx.end() && *pos!='}'){
            switch(*pos){
                case 'L':lexicographic_order = true; return ++pos;
                default: throw std::format_error("invalid format");
            }
        }else{
            return pos;
        }
    }

    auto format(v2::employee const &e, std::format_context &ctx)
    {
        if (lexicographic_order)
            return std::format_to(ctx.out(), "[{}] {}, {}", e.id, e.lastName, e.firstName);

        return std::format_to(ctx.out(), "[{}] {} {}", e.id, e.firstName, e.lastName);
    }
};

int main()
{
    v2::employee e1{12, "John", "Doe"};
    auto s1 = std::format("{}", e1);
    auto s2 = std::format("{:L}", e1);
    std::cout << s1 << std::endl; // [12] John Doe
    std::cout << s2 << std::endl; // [12] Doe, John
}
```

```cpp
#include <iostream>
#include <format>

namespace v2
{
    struct employee
    {
        int id;
        std::string firstName;
        std::string lastName;
    };
}

template <>
struct std::formatter<v2::employee>
{
    bool lexicographic_order = false;

    constexpr auto parse(std::format_parse_context &ctx)
    {
        auto pos = ctx.begin();
        while (pos != ctx.end() && *pos != '}')
        {
            if (*pos == 'h' || *pos == 'H')
                lexicographic_order = true;
            ++pos;
        }
        return pos; // expect `}` at this position, otherwise it's error! exception!
    }

    auto format(const v2::employee &e, std::format_context &ctx)
    {
        if (lexicographic_order)
            return std::format_to(ctx.out(), "[{}] {}, {}", e.id, e.lastName, e.firstName);

        return std::format_to(ctx.out(), "[{}] {} {}", e.id, e.firstName, e.lastName);
    }
};

int main()
{
    v2::employee e2{12, "John", "Doe"};
    auto s1 = std::format("{}", e2);
    auto s2 = std::format("{:h}", e2);
    // auto s3 = std::format("{:A}", e2); // error, cannot parse A
    auto s4 = std::format("{:hello}", e2);
    std::cout << s1 << std::endl; // [12] John Doe
    std::cout << s2 << std::endl; // [12] Doe, John
    std::cout << s4 << std::endl; // [12] Doe, John
}
```