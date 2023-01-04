# Numbers and Strings

- [Numbers and Strings](#numbers-and-strings)
  - [built-in literals](#built-in-literals)
  - [custom literals](#custom-literals)
    - [raw user-defined literals](#raw-user-defined-literals)
  - [raw string literals](#raw-string-literals)
    - [custom string with many functions](#custom-string-with-many-functions)

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