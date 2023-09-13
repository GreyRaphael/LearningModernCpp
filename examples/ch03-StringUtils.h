#pragma once

#include <algorithm>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace StringUtils {

inline std::vector<std::string_view> split(std::string_view str, std::string_view delim_str) {
    size_t pos_start = 0, pos_end, delim_len = delim_str.length();
    std::string_view token;
    std::vector<std::string_view> tokens;

    while ((pos_end = str.find(delim_str, pos_start)) != std::string::npos) {
        token = str.substr(pos_start, pos_end - pos_start);
        tokens.push_back(token);
        pos_start = pos_end + delim_len;
    }
    if (pos_start < str.length()) {
        tokens.push_back(str.substr(pos_start));
    }
    return tokens;
}

inline std::vector<std::string_view> split(std::string_view str, char const delim_char) {
    size_t pos_start = 0, pos_end;
    std::string_view token;
    std::vector<std::string_view> tokens;

    while ((pos_end = str.find(delim_char, pos_start)) != std::string::npos) {
        token = str.substr(pos_start, pos_end - pos_start);
        tokens.push_back(token);
        pos_start = pos_end + 1;
    }
    if (pos_start < str.length()) {
        tokens.push_back(str.substr(pos_start));
    }
    return tokens;
}

inline std::string tolower(std::string_view str) {
    std::string result;
    result.resize(str.length());
    std::transform(str.begin(), str.end(),
                   result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

inline std::string toupper(std::string_view str) {
    std::string result;
    result.resize(str.length());
    std::transform(str.begin(), str.end(),
                   result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

inline std::string capitalize(std::string_view str) {
    std::string result = tolower(str);
    if (!result.empty()) {
        result.front() = std::toupper(result.front());
    }
    return result;
}

inline bool contains(std::string_view str, std::string_view substring) {
    return str.find(substring) != std::string::npos;
}

inline bool contains(std::string_view str, char const character) {
    return str.find(character) != std::string::npos;
}

inline std::string_view lstrip(std::string_view str, std::string_view substring = " \t\r\n") {
    auto pos_start = str.find_first_not_of(substring);
    if (pos_start != std::string::npos) {
        return str.substr(pos_start);
    }
    return str;
}

inline std::string_view rstrip(std::string_view str, std::string_view substring = " \t\r\n") {
    auto pos_end = str.find_last_not_of(substring);
    if (pos_end != std::string::npos) {
        return str.substr(0, pos_end + 1);
    }
    return str;
}

inline std::string_view strip(std::string_view str, std::string_view substring = " \t\r\n") {
    return rstrip(lstrip(str, substring), substring);
}

inline std::string_view ltrim(std::string_view str, std::string_view substring) {
    auto pos_start = str.find(substring);
    if (pos_start != std::string::npos) {
        return str.substr(pos_start + substring.size());
    }
    return str;
}

inline std::string_view rtrim(std::string_view str, std::string_view substring) {
    auto pos_start = str.rfind(substring);
    if (pos_start != std::string::npos) {
        return str.substr(0, pos_start);
    }
    return str;
}

inline std::string_view trim(std::string_view str, std::string_view substring) {
    return rtrim(ltrim(str, substring), substring);
}

inline bool startswith(std::string_view str, std::string_view substring) {
    return str.find(substring) == 0;
}

inline bool endswith(std::string_view str, std::string_view substring) {
    return str.rfind(substring) == str.size() - substring.size();
}

// template <typename Container, typename = std::enable_if_t<std::is_same_v<typename Container::value_type, std::string>, std::string>>
template <typename Container, typename = std::enable_if_t<std::is_same_v<typename Container::value_type, std::string>>>
inline std::string join(Container const& str_container, std::string_view delim_str) {
    if (str_container.empty()) return "";

    auto it = str_container.begin();
    std::string result = *it++;

    for (; it != str_container.end(); ++it) {
        result += delim_str;
        result += *it;
    }

    return result;
}

// to be optimized
inline size_t count(std::string_view str, std::string substring) {
    size_t count = 0;
    size_t pos = 0;
    while ((pos = str.find(substring, pos)) != std::string::npos) {
        ++count;
        pos += substring.length();
    }
    return count;
}

// to be optimized
inline std::string replace(std::string_view str, std::string_view from, std::string_view to) {
    size_t pos = 0;
    std::string str_copy{str};
    while ((pos = str_copy.find(from, pos)) != std::string::npos) {
        str_copy.replace(pos, from.length(), to);
    }
    return str_copy;
}

// encode string to utf8 bytes, to be rewritten
inline std::string encode_utf8(std::string_view str) {
    return std::string(str.begin(), str.end());
}

inline bool isalnum(std::string_view str) {
    for (char const c : str) {
        if (!std::isalnum(c)) return false;
    }
    return true;
}

inline bool isalpha(std::string_view str) {
    for (char const c : str) {
        if (!std::isalpha(c)) return false;
    }
    return true;
}

inline bool isdigit(std::string_view str) {
    for (char const c : str) {
        if (!std::isdigit(c)) return false;
    }
    return true;
}

inline std::string ljust(std::string_view str, size_t width, char fill = ' ') {
    std::string result{str};
    if (str.size() >= width) return result;
    return std::string(width - str.size(), fill) + result;
}

inline std::string rjust(std::string_view str, size_t width, char fill = ' ') {
    std::string result{str};
    if (str.size() >= width) return result;
    return result + std::string(width - str.size(), fill);
}

inline std::string zfill(std::string_view str, size_t width) {
    std::string result{str};
    if (str.size() >= width) {
        return result;
    }
    auto num_zeros = width - str.length();
    if ('-' == str[0]) {
        return "-" + std::string(num_zeros, '0') + result.substr(1);
    } else {
        return std::string(num_zeros, '0') + result;
    }
}

inline std::tuple<std::string_view, std::string_view> lpartition(std::string_view str, std::string_view delim_str) {
    size_t pos = str.find(delim_str);
    if (pos == std::string::npos) return {str, ""};
    return {str.substr(0, pos), str.substr(pos + delim_str.size())};
}

inline std::tuple<std::string_view, std::string_view> rpartition(std::string_view str, std::string_view delim_str) {
    size_t pos = str.rfind(delim_str);
    if (pos == std::string::npos) return {str, ""};
    return {str.substr(0, pos), str.substr(pos + delim_str.size())};
}

}  // namespace StringUtils