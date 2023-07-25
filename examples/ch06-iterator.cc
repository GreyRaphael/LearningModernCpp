#include <algorithm>   // std::transform, std::for_each
#include <cassert>     // assert()
#include <functional>  // bad_function_call()
#include <iostream>
#include <iterator>  // reverse_iterator
#include <memory>    // make_unique
#include <string>

template <typename Type, size_t const Size>
class dummy_array {
    Type data[Size] = {};

   public:
    Type& operator[](size_t const i) {
        if (i < Size) return data[i];
        throw std::out_of_range("index out of range");
    }

    Type const& operator[](size_t const i) const {
        if (i < Size) return data[i];
        throw std::out_of_range("index out of range");
    }

    size_t size() const { return Size; }

    template <typename T, size_t const SZ>
    class dummy_array_iterator {
       public:
        typedef dummy_array_iterator self_type;
        typedef T value_type;
        typedef T& reference;
        typedef T* pointer;
        typedef ptrdiff_t difference_type;

       private:
        pointer ptr = nullptr;
        size_t index = 0;

        bool compatible(self_type const& other) const {
            return ptr == other.ptr;
        }

       public:
        // ctor with inputs
        explicit dummy_array_iterator(pointer ptr, size_t const index) : ptr(ptr), index(index) {
        }
        // default ctor
        dummy_array_iterator() = default;
        // copy ctor
        dummy_array_iterator(dummy_array_iterator const& o) = default;
        // assign ctor
        dummy_array_iterator& operator=(dummy_array_iterator const& o) = default;
        // desctor
        ~dummy_array_iterator() = default;

        // prefix ++
        self_type& operator++() {
            if (index >= SZ)
                throw std::out_of_range("Iterator cannot be incremented past the end of range.");
            ++index;
            return *this;
        }
        // postfix ++
        self_type operator++(int) {
            self_type tmp = *this;
            ++*this;
            return tmp;
        }
        // prefix --
        self_type& operator--() {
            if (index <= 0)
                throw std::out_of_range("Iterator cannot be decremented past the end of range.");
            --index;
            return *this;
        }
        // postfix --
        self_type operator--(int) {
            self_type tmp = *this;
            --*this;
            return tmp;
        }
        // comparisons
        bool operator==(self_type const& other) const {
            assert(compatible(other));
            return index == other.index;
        }
        bool operator!=(self_type const& other) const {
            return !(*this == other);
        }
        bool operator<(self_type const& other) const {
            assert(compatible(other));
            return index < other.index;
        }
        bool operator>(self_type const& other) const {
            return other < *this;
        }
        bool operator<=(self_type const& other) const {
            return !(*this > other);
        }
        bool operator>=(self_type const& other) const {
            return !(*this < other);
        }

        // can be dereferenced as an rvalue
        reference operator*() const {
            if (ptr == nullptr)
                throw std::bad_function_call();
            return *(ptr + index);
        }
        reference operator->() const {
            if (ptr == nullptr)
                throw std::bad_function_call();
            return *(ptr + index);
        }

        // offset
        self_type& operator+=(difference_type const offset) {
            if (index + offset < 0 || index + offset > SZ)
                throw std::out_of_range("Iterator cannot be incremented past the end of range.");

            index += offset;
            return *this;
        }
        self_type& operator-=(difference_type const offset) {
            return *this += -offset;
        }
        self_type operator+(difference_type offset) const {
            self_type tmp = *this;
            return tmp += offset;
        }
        self_type operator-(difference_type offset) const {
            self_type tmp = *this;
            return tmp -= offset;
        }
        difference_type operator-(self_type const& other) const {
            assert(compatible(other));
            return (index - other.index);
        }
        // offset dereference operator ([])
        value_type& operator[](difference_type const offset) {
            return (*(*this + offset));
        }
        value_type const& operator[](difference_type const offset) const {
            return (*(*this + offset));
        }
    };

    typedef dummy_array_iterator<Type, Size> iterator;
    typedef dummy_array_iterator<Type const, Size> constant_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<constant_iterator> reverse_constant_iterator;

   public:
    iterator begin() {
        return iterator(data, 0);
    }
    iterator end() {
        return iterator(data, Size);
    }
    constant_iterator begin() const {
        return constant_iterator(data, 0);
    }
    constant_iterator end() const {
        return constant_iterator(data, Size);
    }
    constant_iterator cbegin() const {
        return constant_iterator(data, 0);
    }
    constant_iterator cend() const {
        return constant_iterator(data, Size);
    }
    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }
    reverse_iterator rend() {
        return reverse_iterator(begin());
    }
};

template <typename T, const size_t SZ>
void print_dummy_array(dummy_array<T, SZ> const& arr) {
    for (auto& e : arr) {
        std::cout << e << ',';
    }
    std::cout << '\n';
}
template <typename T, const size_t SZ>
void print_dummy_array2(dummy_array<T, SZ> const& arr) {
    for (unsigned i = 0; i < arr.size(); ++i) {
        std::cout << arr[i] << ',';
    }
    std::cout << '\n';
}

void test_example1() {
    dummy_array<int, 3> arr;
    arr[0] = 100;
    arr[1] = 200;
    arr[2] = 300;
    print_dummy_array(arr);
    std::transform(arr.begin(), arr.end(), arr.begin(), [](int const e) { return e * 2; });
    print_dummy_array2(arr);
}

struct Task {
    int priority;
    std::string name;
};

void test_example2() {
    dummy_array<Task, 3> arr;
    arr[0] = Task{20, "task1"};
    arr[1] = Task{10, "task2"};
    arr[2] = Task{30, "task3"};
    for (auto e : arr) {
        std::cout << e.name << ':' << e.priority << '\n';
    }
    auto p = &arr[0];
    std::cout << p->priority << '\n';

    auto task = arr[2];
    std::cout << task.priority << '\n';
}

struct Tag {
    int id;
    std::string name;

    Tag(int const id = 0, std::string const& name = "") : id(id), name(name) {}
};

void test_example3() {
    dummy_array<std::unique_ptr<Tag>, 3> arr;
    arr[0] = std::make_unique<Tag>(20, "tag1");
    arr[1] = std::make_unique<Tag>(10, "tag2");
    arr[2] = std::make_unique<Tag>(30, "tag3");
    for (auto& e : arr) {
        // std::cout << e.get()->name << '\n';
        std::cout << e->name << '\n';
    }
    for (auto it = arr.begin(); it != arr.end(); ++it) {
        std::cout << it->name << '\n';
    }
}

void test_example4() {
    dummy_array<int, 3> arr;
    arr[0] = 100;
    arr[1] = 200;
    arr[2] = 300;
    for (auto cit = arr.cbegin(); cit != arr.cend(); ++cit) {
        std::cout << *cit << ',';
    }
    std::cout << '\n';
    for (auto it = arr.rbegin(); it != arr.rend(); ++it) {
        std::cout << *it << ',';
    }
    std::cout << '\n';
}

void test_example5() {
    dummy_array<std::string, 3> s_arr;
    s_arr[0] = "100";
    s_arr[1] = "200";
    s_arr[2] = "300";
    dummy_array<int, 3> num_arr;
    std::transform(s_arr.begin(), s_arr.end(), num_arr.begin(), [](std::string const& s) { return std::stoi(s); });
    print_dummy_array(num_arr);
}

template <typename F, typename C>
void process(F&& func, C const& container) {
    std::for_each(std::begin(container), std::end(container), std::forward<F>(func));
}

void test_example6() {
    auto func = [](auto const e) { std::cout << e << '\t'; };

    std::vector v = {1, 2, 3, 4, 5};
    process(func, v);
    std::cout << '\n';

    dummy_array<std::string, 3> s_arr;
    s_arr[0] = "100";
    s_arr[1] = "200";
    s_arr[2] = "300";
    process(func, s_arr);
    std::cout << '\n';
}

int main() {
    test_example1();
    test_example2();
    test_example3();
    test_example4();
    test_example5();
    test_example6();
}