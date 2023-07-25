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
};

template <typename T, typename C, size_t const Size>
class dummy_array_iterator_type {
    size_t index;
    C& collection;

   public:
    dummy_array_iterator_type(C& collection, size_t const index) : collection(collection), index(index) {}

    bool operator!=(dummy_array_iterator_type const& other) const {
        return index != other.index;
    }

    T const& operator*() const {
        return collection[index];
    }

    dummy_array_iterator_type& operator++() {
        ++index;
        return *this;
    }

    dummy_array_iterator_type operator++(int) {
        auto temp = *this;
        ++*this;
        return temp;
    }
};

template <typename T, size_t const Size>
using dummy_array_iterator = dummy_array_iterator_type<T, dummy_array<T, Size>, Size>;

template <typename T, size_t const Size>
using dummy_array_const_iterator = dummy_array_iterator_type<T, dummy_array<T, Size> const, Size>;

template <typename T, size_t const Size>
// inline dummy_array_iterator<T, Size> begin(dummy_array<T, Size>& collection)
inline auto begin(dummy_array<T, Size>& collection) {
    return dummy_array_iterator<T, Size>(collection, 0);
}
template <typename T, size_t const Size>
inline auto end(dummy_array<T, Size>& collection) {
    return dummy_array_iterator<T, Size>(collection, collection.size());
}

template <typename T, size_t const Size>
inline auto begin(dummy_array<T, Size> const& collection) {
    return dummy_array_const_iterator<T, Size>(collection, 0);
}
template <typename T, size_t const Size>
inline auto end(dummy_array<T, Size> const& collection) {
    return dummy_array_const_iterator<T, Size>(collection, collection.size());
}

template <typename T, const size_t Size>
// 这里有dummy_array<T, Size> const &
// 所以需要实现对应的begin,end的dummy_array<T, Size> const &
void print_dummy_array(dummy_array<T, Size>& arr) {
// void print_dummy_array(dummy_array<T, Size> const& arr) {
    for (auto&& e : arr) {
        std::cout << e << ';';
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