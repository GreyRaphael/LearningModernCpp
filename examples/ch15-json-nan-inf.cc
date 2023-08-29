#include <cmath>
#include <cstring>
#include <deque>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>, T>>
inline std::array<std::byte, sizeof(T)> value2bytes(T const& value) {
    std::array<std::byte, sizeof(T)> dest_vec;
    std::memcpy(dest_vec.data(), &value, sizeof(T));
    return dest_vec;
}

template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>, T>>
inline T bytes2value(std::array<std::byte, sizeof(T)> const& src_vec) {
    T result;
    std::memcpy(&result, src_vec.data(), sizeof(T));
    return result;
}

inline std::vector<std::byte> deque2bytes_method1(std::deque<double> const& src_dq) {
    std::vector<std::byte> dest_bytes;

    for (const auto& value : src_dq) {
        auto value_bytes = value2bytes(value);
        dest_bytes.insert(dest_bytes.end(), value_bytes.begin(), value_bytes.end());
    }
    return dest_bytes;
}

inline std::vector<std::byte> deque2bytes(std::deque<double> const& src_dq) {
    std::vector<std::byte> dest_bytes(src_dq.size() * sizeof(double));

    auto dest_ptr = dest_bytes.data();
    for (const auto& value : src_dq) {
        auto value_bytes = value2bytes(value);
        std::memcpy(dest_ptr, value_bytes.data(), sizeof(double));
        dest_ptr += sizeof(double);
    }
    return dest_bytes;
}

inline std::deque<double> bytes2deque(const std::vector<std::byte>& src_bytes) {
    std::deque<double> dest_dq;
    size_t num_values = src_bytes.size() / sizeof(double);

    for (size_t i = 0; i < num_values; ++i) {
        std::array<std::byte, sizeof(double)> value_bytes;
        std::memcpy(value_bytes.data(), &src_bytes[i * sizeof(double)], sizeof(double));
        double value = bytes2value<double>(value_bytes);
        dest_dq.push_back(value);
    }

    return dest_dq;
}

inline std::vector<double> bytes2vector(std::vector<std::byte> const& src_bytes) {
    std::vector<double> dest_vec(src_bytes.size() / sizeof(double));
    std::memcpy(dest_vec.data(), src_bytes.data(), src_bytes.size());
    return dest_vec;
}

inline std::vector<std::byte> vector2bytes(std::vector<double> const& src_vec) {
    auto bytes_size = src_vec.size() * sizeof(double);
    std::vector<std::byte> dest_bytes(bytes_size);
    std::memcpy(dest_bytes.data(), src_vec.data(), bytes_size);
    return dest_bytes;
}

int main() {
    {
        double init_val = 3.1415926;
        auto vec1 = value2bytes<double>(init_val);
        auto val1 = bytes2value<double>(vec1);
        std::cout << "src = " << init_val << ", dest = " << val1 << '\n';
    }
    {
        std::deque<double> init_dq{1.1, 2.3, 4.7, NAN, INFINITY};
        auto vec1 = deque2bytes(init_dq);
        auto dq1 = bytes2deque(vec1);
        for (auto v : dq1) std::cout << v << '\t';
        std::cout << '\n';
    }
    {
        std::vector<double> init_vec{100.1, 2.3, 4.7, NAN, INFINITY};
        auto bytes_vec = vector2bytes(init_vec);
        auto vec1 = bytes2vector(bytes_vec);
        for (auto v : vec1) std::cout << v << '\t';
        std::cout << '\n';
    }
    {
        std::deque<double> dq1{1.1, 2.3, 4.7, NAN, INFINITY};
        nlohmann::json init_j{
            {"feature", 3.1415926},
            // {"feature", NAN},
            // {"feature", INFINITY},
            {"_xSeries", dq1}};
        // std::cout << init_j << '\n';// dump cannot NULL, INFINITY

        nlohmann::json j_converted;
        j_converted["feature"] = value2bytes<double>(init_j["feature"]);
        j_converted["_xSeries"] = deque2bytes(init_j["_xSeries"]);
        std::cout << j_converted << '\n';

        auto value_bytes = j_converted["feature"].get<std::array<std::byte, sizeof(double)>>();
        auto dq_bytes = j_converted["_xSeries"].get<std::vector<std::byte>>();
        
        std::cout << bytes2value<double>(value_bytes) << '\n';
        for (auto e : bytes2deque(dq_bytes)) std::cout << e << '\t';
        std::cout << '\n';
    }
}