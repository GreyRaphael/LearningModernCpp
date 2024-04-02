#pragma once

#include <algorithm>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace StringUtils {

// Split input str to vector<string_view> by delimiter string
inline std::vector<std::string_view> split(std::string_view strv, std::string_view delims = " ") {
    std::vector<std::string_view> output;
    size_t first = 0;

    while (first < strv.size()) {
        const auto second = strv.find_first_of(delims, first);

        if (first != second)
            output.emplace_back(strv.substr(first, second - first));

        if (second == std::string_view::npos)
            break;

        first = second + 1;
    }

    return output;
}

inlne std::vector<std::string_view> splitByRawpointer(std::string_view strv, std::string_view delims = " ") {
    std::vector<std::string_view> output;
    auto delim_len = delims.size();
    if (delim_len == 0) {  // delims is empty
        output.emplace_back(strv);
        return output;
    }

    auto start = strv.data();
    auto end = start + strv.size();
    auto current = start;

    while (start < end) {
        current = std::search(start, end, delims.begin(), delims.end());
        if (current != start)
            output.emplace_back(std::string_view(start, current - start));

        start = current + delim_len;
    }

    return output;
}

// Split input str to vector<string_view> by delimiter character
inline std::vector<std::string_view> split(std::string_view str, char const delim_char) {
    size_t pos_start = 0, pos_end;
    std::string_view token;
    std::vector<std::string_view> tokens;

    // if str startswith delim_char, ignore
    if (str.find(delim_char) == 0) pos_start = 1;

    while ((pos_end = str.find(delim_char, pos_start)) != std::string_view::npos) {
        token = str.substr(pos_start, pos_end - pos_start);
        tokens.push_back(token);
        pos_start = pos_end + 1;
    }
    // if str endswith delim_char, ignore
    if (pos_start < str.size()) {
        tokens.push_back(str.substr(pos_start));
    }
    return tokens;
}

// Split input str by regex, regex_split("hello23world56grey", R"(\d+)")
inline std::vector<std::string> regex_split(std::string const& src, std::string const& regex_delim) {
    std::regex rgx(regex_delim);
    std::vector<std::string> results(
        std::sregex_token_iterator(src.begin(), src.end(), rgx, -1),
        std::sregex_token_iterator());
    return results;
}

// lowercase all character in the str
inline std::string tolower(std::string_view str) {
    std::string result;
    result.resize(str.size());
    std::transform(str.begin(), str.end(),
                   result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

// uppercase all character in the str
inline std::string toupper(std::string_view str) {
    std::string result;
    result.resize(str.size());
    std::transform(str.begin(), str.end(),
                   result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

// first character upppercase, others lowercase
inline std::string capitalize(std::string_view str) {
    std::string result = tolower(str);
    if (!result.empty()) {
        result.front() = std::toupper(result.front());
    }
    return result;
}

//  Checks if input str contains specified substring.
inline bool contains(std::string_view str, std::string_view substring) {
    return str.find(substring) != std::string_view::npos;
}

//  Checks if input str contains specified character.
inline bool contains(std::string_view str, char const character) {
    return str.find(character) != std::string_view::npos;
}

//  Remove specified leading characters
inline std::string_view lstrip(std::string_view str, std::string_view substring = " \t\r\n") {
    auto pos_start = str.find_first_not_of(substring);
    if (pos_start != std::string_view::npos) {
        return str.substr(pos_start);
    }
    return str;
}

//  Remove specified trailing characters
inline std::string_view rstrip(std::string_view str, std::string_view substring = " \t\r\n") {
    auto pos_end = str.find_last_not_of(substring);
    if (pos_end != std::string_view::npos) {
        return str.substr(0, pos_end + 1);
    }
    return str;
}

//  Remove specified leading and trailing characters
inline std::string_view strip(std::string_view str, std::string_view substring = " \t\r\n") {
    return rstrip(lstrip(str, substring), substring);
}

// Remove specified leading substring once
inline std::string_view ltrim(std::string_view str, std::string_view substring) {
    auto pos_start = str.find(substring);
    if (pos_start != std::string_view::npos) {
        return str.substr(pos_start + substring.size());
    }
    return str;
}

// Remove specified tailing substring once
inline std::string_view rtrim(std::string_view str, std::string_view substring) {
    auto pos_start = str.rfind(substring);
    if (pos_start != std::string_view::npos) {
        return str.substr(0, pos_start);
    }
    return str;
}

// Remove specified leading/tailing substring once respectively
inline std::string_view trim(std::string_view str, std::string_view substring) {
    return rtrim(ltrim(str, substring), substring);
}

// Checks if input str starts with specified substring.
inline bool startswith(std::string_view str, std::string_view substring) {
    return str.find(substring) == 0;
}

// Checks if input str starts with specified character.
inline bool startswith(std::string_view str, char const character) {
    return str.find(character) == 0;
}

// Checks if input str ends with specified substring.
inline bool endswith(std::string_view str, std::string_view substring) {
    return str.rfind(substring) == str.size() - substring.size();
}

// Checks if input str ends with specified character.
inline bool endswith(std::string_view str, char const character) {
    return str.rfind(character) == str.size() - 1;
}

//  join all strings in the container as one string by delimiter string
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

// get the number of non-overlapping occurrences of substring in the input str
inline size_t count(std::string_view str, std::string_view substring) {
    size_t count = 0;
    size_t pos = 0;
    while ((pos = str.find(substring, pos)) != std::string_view::npos) {
        ++count;
        pos += substring.size();
    }
    return count;
}

// convert input str to hex string
inline std::string to_hex(std::string_view str) {
    std::stringstream ss;
    ss << std::hex;
    for (unsigned char c : str) {
        ss << "\\x" << static_cast<int>(c);
    }
    return ss.str();
}

// convert input hex string to original string
inline std::string from_hex(std::string_view hex_str) {
    std::string result;
    auto hex_vec = split(hex_str, "\\x");
    for (std::string_view sv : hex_vec) {
        std::string temp{sv};
        unsigned char val = std::stoi(temp, nullptr, 16);
        result += val;
    }
    return result;
}

// Replace all of input str, `from`->`to`
inline std::string replace(std::string_view str, std::string_view from, std::string_view to) {
    size_t pos = 0;
    std::string str_copy{str};
    while ((pos = str_copy.find(from, pos)) != std::string::npos) {
        str_copy.replace(pos, from.size(), to);
    }
    return str_copy;
}

// Check whether all character of input str is
// digit: 0123456789
// uppercase letters: ABCDEFGHIJKLMNOPQRSTUVWXYZ
// lowercase letters: abcdefghijklmnopqrstuvwxyz
inline bool isalnum(std::string_view str) {
    for (char const c : str) {
        if (!std::isalnum(c)) return false;
    }
    return true;
}

// Check whether all character of input str is
// uppercase letters: ABCDEFGHIJKLMNOPQRSTUVWXYZ
// lowercase letters: abcdefghijklmnopqrstuvwxyz
inline bool isalpha(std::string_view str) {
    for (char const c : str) {
        if (!std::isalpha(c)) return false;
    }
    return true;
}

// Check whether all character of input str is
// digit: 0123456789
inline bool isdigit(std::string_view str) {
    for (char const c : str) {
        if (!std::isdigit(c)) return false;
    }
    return true;
}

// Add leading charater to a width of input str
inline std::string ljust(std::string_view str, size_t width, char fill_char = ' ') {
    std::string result{str};
    if (str.size() >= width) return result;
    return std::string(width - str.size(), fill_char) + result;
}

// Add tailing charater to a width of input str
inline std::string rjust(std::string_view str, size_t width, char fill_char = ' ') {
    std::string result{str};
    if (str.size() >= width) return result;
    return result + std::string(width - str.size(), fill_char);
}

// Add leading/tailing charater to a width of input str, which make it center
inline std::string center(std::string_view str, size_t width, char fill_char = ' ') {
    std::string result{str};
    if (str.size() >= width) return result;
    size_t left_pad = (width - str.size()) / 2;
    size_t right_pad = (width - str.size()) - left_pad;
    return std::string(left_pad, fill_char) + result + std::string(right_pad, fill_char);
}

// Add leading '0' to number string
inline std::string zfill(std::string_view str, size_t width) {
    std::string result{str};
    if (str.size() >= width) {
        return result;
    }
    auto num_zeros = width - str.size();
    if ('-' == str[0]) {
        return "-" + std::string(num_zeros, '0') + result.substr(1);
    } else {
        return std::string(num_zeros, '0') + result;
    }
}

// Split the string at the first occurrence of delim_str, return {left_part, right_part}
inline std::tuple<std::string_view, std::string_view> lpartition(std::string_view str, std::string_view delim_str) {
    size_t pos = str.find(delim_str);
    if (pos == std::string_view::npos) return {str, ""};
    return {str.substr(0, pos), str.substr(pos + delim_str.size())};
}

// Split the string at the last occurrence of delim_str, return {left_part, right_part}
inline std::tuple<std::string_view, std::string_view> rpartition(std::string_view str, std::string_view delim_str) {
    size_t pos = str.rfind(delim_str);
    if (pos == std::string_view::npos) return {str, ""};
    return {str.substr(0, pos), str.substr(pos + delim_str.size())};
}

// Creates new std::string with repeated n times substring str.
inline std::string repeat(std::string_view str, size_t n) {
    std::string result;
    result.reserve(str.size() * n);
    for (size_t i = 0; i < n; i++) {
        result.append(str);
    }
    return result;
}

// Creates new std::string with repeated n times char c.
inline std::string repeat(char c, size_t n) {
    return std::string(c, n);
}

// check if input str match regex patter_str
inline bool match(std::string_view str, std::string pattern_str) {
    return std::regex_match(str.begin(), str.end(), std::regex(pattern_str));
}

}  // namespace StringUtils