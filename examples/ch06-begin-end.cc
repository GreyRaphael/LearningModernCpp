#include <iostream>

template <typename T, size_t const Size>
class dummy_array {
   private:
    T data[Size] = {};

   public:
    T const& operator[](size_t const i) const {
        if (i < Size) return data[i];
        throw std::out_of_range("index out of range");
    }
    T& operator[](size_t const i) {
        if (i < Size) return data[i];
        throw std::out_of_range("index out of range");
    }

    size_t size() const { return Size; }

    T* begin() { return data; }
    T* end() { return data + Size; }

    T const* begin() const { return data; }
    T const* end() const { return data + Size; }

    T const* cbegin() const { return data; }
    T const* cend() const { return data + Size; }

    T* rbegin() { return data + Size - 1; }
    T* rend() { return data - 1; }

    T const* rbegin() const { return data + Size - 1; }
    T const* rend() const { return data - 1; }
};

template <typename T, const size_t SZ>
// void print_dummy_array(dummy_array<T, SZ>& arr) {
void print_dummy_array(dummy_array<T, SZ> const& arr) {
    for (auto& e : arr) {
        std::cout << e << ',';
    }
    std::cout << '\n';

    for (auto it = arr.cbegin(); it != arr.cend(); it++) {
        std::cout << *it << '\t';
    }
    std::cout << '\n';

    // 这里是--it，一般都是++it, 所以需要iterator封装
    for (auto it = arr.rbegin(); it != arr.rend(); it--) {
        std::cout << *it << '\t';
    }
    std::cout << '\n';
}

int main() {
    dummy_array<int, 3> arr;
    arr[0] = 100;
    arr[1] = 200;
    arr[2] = 300;
    print_dummy_array(arr);
}