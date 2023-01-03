# Numbers and Strings

- [Numbers and Strings](#numbers-and-strings)
  - [raw string literals](#raw-string-literals)
    - [custom string with many functions](#custom-string-with-many-functions)

## raw string literals

```cpp
#include<iostream>

using namespace std::string_literals;

int main(){
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