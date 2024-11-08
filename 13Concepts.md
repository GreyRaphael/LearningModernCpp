# C++20 Concepts

- [C++20 Concepts](#c20-concepts)
  - [Simple constraint by Concepts](#simple-constraint-by-concepts)
  - [Concepts `requires` expr](#concepts-requires-expr)
  - [concepts for polymorphism](#concepts-for-polymorphism)

> A concept is a named set of constraints and a constraint is a requirement for a template argument. 

> These are used to select the appropriate function overloads and template specializations.

## Simple constraint by Concepts

simple example

```cpp
#include <iostream>

namespace v1 {
// without concepts
template <typename T, typename = typename std::enable_if_t<std::is_arithmetic_v<T>, T>>
T myadd(T x, T y) {
    return x + y;
}
}  // namespace v1

namespace v2 {
// with concepts
template <typename T>
concept Numerical = std::is_arithmetic_v<T>;

template <Numerical T>
T myadd(T x, T y) {
    return x + y;
}
}  // namespace v2

int main() {
    {
        // without concepts
        auto v1 = v1::myadd(1, 2);
        auto v2 = v1::myadd(11.1, 22.2);
        // auto v3 = myadd(std::string{"hello"}, std::string{"world"});  // error
    }
    {
        // with concepts
        auto v1 = v2::myadd(1, 2);
        auto v2 = v2::myadd(11.1, 22.2);
        // auto v3 = myadd(std::string{"hello"}, std::string{"world"});  // error
    }
}
```

concept conjection: `&&`

```cpp
#include <iostream>

struct IComparableToInt {
    virtual bool CompareTo(int const o) = 0;
};

struct IConvertibleToInt {
    virtual int ConvertTo() = 0;
};

template <typename T>
struct SmartNumericalValue : public IComparableToInt, IConvertibleToInt {
    T value;

    SmartNumericalValue(T v) : value(v) {}

    bool CompareTo(int const o) override { return static_cast<int>(value) == o; }
    int ConvertTo() override { return static_cast<int>(value); }
};

template <typename T>
struct DullNumericalValue : public IConvertibleToInt {
    T value;

    DullNumericalValue(T v) : value(v) {}

    int ConvertTo() override { return static_cast<int>(value); }
};

template <class T>
concept IntComparable = std::is_base_of_v<IComparableToInt, T>;

template <class T>
concept IntConvertible = std::is_base_of_v<IConvertibleToInt, T>;

template <class T>
concept IntComparableAndConvertible = IntComparable<T> && IntConvertible<T>;

template <IntComparableAndConvertible T>
void print(T o) {
    std::cout << o.value << '\n';
}

int main() {
    SmartNumericalValue v1{100.2};
    DullNumericalValue v2{200.3};
    print(v1);  // 100.2
    // print(v2);// error, v2 isn't IntComparableAndConvertible
}
```

concept disjunction: `||`

```cpp
template <typename T>
concept Numerical = std::integral<T> || std::floating_point<T>;
```

## Concepts `requires` expr

example: parameter type `T` of function `myadd()` must be `Addable`, which means `operator+` must be overloaded for the `T` template argument

```cpp
#include <iostream>

template <typename T>
concept Addable = requires(T x, T y) { x + y; };

template <Addable T>
T myadd(T x, T y) {
    return x + y;
}

class Foo {
    int val_;

   public:
    Foo(int const v) : val_(v) {
    }
};

class Bar {
   private:
    int val_;

   public:
    Bar(int const v) : val_(v) {}
    Bar operator+(Bar const& other) {
        Bar temp{0};
        temp.val_ = other.val_ + val_;
        return temp;
    }
    void print() {
        std::cout << val_ << '\n';
    }
};

int main() {
    auto v1 = myadd(1, 2);
    auto v2 = myadd(10.1, 20.2);
    auto v3 = myadd(std::string{"hello"}, std::string{"world"});
    std::cout << v3 << '\n';  // helloworld
    // auto v4 = myadd(Foo{2}, Foo{3}); // error
    auto v5 = myadd(Bar{10}, Bar{20});
    v5.print();  // 30
}
```

example: paramter type `T` of function `do_wrap()` must be `Wrapable`, which means `T` must be `Numerical`

```cpp
#include <iostream>

template <typename T>
concept Numerical = std::is_arithmetic_v<T>;

template <Numerical T>
struct Num {
    T value;
};

template <Numerical T>
Num<T> wrap(T value) {
    return {value};
}

template <Numerical T>
T unwrap(Num<T> s) {
    return s.value;
}

template <typename T>
concept Wrapable = requires(T x) { wrap(x); };

template <Wrapable T>
auto do_wrap(T x) {
    return wrap(x);
}

int main() {
    auto v1 = do_wrap(1);
    auto v2 = do_wrap(10.1);
    // auto v3 = do_wrap(std::string{"hello"});// not arithmetic_v
}
```

example: paramter type `T` of function `pass_container` must be `Container`
- `typename T::value_type;` states that `T` must have a nested type called `value_type`  
- `typename T::iterator;` states that `T` must have a nested type called `iterator`
- `begin(x);` checks if the expression `begin(x)` is valid
- `end(x);` checks if the expression `end(x)` is valid

Overall, the `Container` concept ensures that a type `T` satisfies the requirements necessary to be used as a container. It checks if `T` has the nested types `value_type` and `iterator`, and if it has valid `begin()` and `end()` member functions.

```cpp
#include <array>
#include <deque>
#include <iostream>
#include <vector>

template <typename T>
concept Container = requires(T x) {
    typename T::value_type;
    typename T::iterator;
    begin(x);
    end(x);
};

template <Container T>
void pass_container(T const& c) {
    for (auto const& x : c) std::cout << x << '\t';
    std::cout << '\n';
}

int main() {
    int arr[4]{4, 3, 2, 1};
    // pass_container(arr); // error, not Container

    std::array<int, 4> a1{10, 20, 30, 40};
    pass_container(a1);

    std::vector<double> v1{1, 2, 3, 4, 5.1};
    pass_container(v1);

    std::deque<int> d1{11, 22, 33, 44};
    pass_container(d1);
}
```

example: paramter type `T` of function `do_wrap_numerical()` must be `NumericalWrapable`, which means return type must be `Num<T>` of function `wrap` and `T` must be `Numerical`

```cpp
#include <iostream>

template <class T>
concept Numerical = std::is_arithmetic_v<T>;

template <Numerical T>
struct Num {
    T value;
};

template <Numerical T>
Num<T> wrap(T value) {
    return {value};
}

template <typename T>
concept NumericalWrapable = requires(T x) {
    // return must be Num<T>
    { wrap(x) } -> std::same_as<Num<T>>;
};

template <typename T>
struct any_wrapper {
    T value;
};

any_wrapper<std::string> wrap(std::string const s) {
    return {s};
}

template <NumericalWrapable T>
auto do_wrap_numerical(T x) {
    return wrap(x);
}

int main() {
    auto w10 = wrap("hello");
    std::cout << w10.value << '\n';
    // auto w11 = do_wrap_numerical("hello");  // error, wrap() not return Num<T>
    // std::cout << w11.value << '\n';

    auto w20 = wrap(10);
    std::cout << w20.value << '\n';
    auto w21 = do_wrap_numerical(11);
    std::cout << w21.value << '\n';

    auto w30 = wrap(20.2);
    std::cout << w30.value << '\n';
    auto w31 = do_wrap_numerical(30.3);
    std::cout << w31.value << '\n';
}
```

## concepts for polymorphism

simple example

```cpp
#include <fmt/core.h>
#include <fmt/format.h>
#include <concepts>

// Define a concept for Drawable objects
template <typename T>
concept Drawable = requires(T t) {
    { t.draw() } -> std::same_as<void>;
};

// Concrete implementation
struct Circle {
    void draw() const {
        fmt::println("This is a circle!");
    }
};

// Concrete implementation
struct Rectangle {
    void draw() const {
        fmt::println("This is a Rectangle!");
    }
};

// Corrected display function using concept-constrained parameter
void display(Drawable auto const& obj) {  // Note the placement of 'Drawable' before 'auto'
    obj.draw();
}

int main() {
    Circle c;
    display(c);
    Rectangle r;
    display(r);
}
```

advanced example

```cpp
#include <fmt/core.h>
#include <fmt/format.h>
#include <concepts>
#include <vector>

// Define a concept for Drawable objects
template <typename T>
concept Drawable = requires(T t) {
    { t.draw() } -> std::same_as<void>;
};

// ShapeContainer using type erasure with concepts
class ShapeContainer {
   public:
    template <Drawable T>
    void addShape(T shape) {
        shapes.emplace_back(std::make_unique<Model<T>>(std::move(shape)));
    }

    void renderAll() const {
        for (const auto& shape : shapes) {
            shape->draw();
        }
    }

   private:
    struct Concept {
        virtual void draw() const = 0;
        virtual ~Concept() = default;
    };

    template <Drawable T>
    struct Model : Concept {
        Model(T shape) : shape_(std::move(shape)) {}
        void draw() const override { shape_.draw(); }
        T shape_;
    };

    std::vector<std::unique_ptr<Concept>> shapes;
};

// Concrete implementation
struct Circle {
    void draw() const {
        fmt::println("This is a circle!");
    }
};

// Concrete implementation
struct Rectangle {
    void draw() const {
        fmt::println("This is a Rectangle!");
    }
};

int main() {
    ShapeContainer container;
    container.addShape(Circle{});
    container.addShape(Rectangle{});
    container.renderAll();
}
```

advanced example like proxy

```cpp
#include <fmt/core.h>

#include <concepts>
#include <memory>
#include <vector>

// Define a concept for Drawable objects
template <typename T>
concept Drawable = requires(T t) {
    { t.draw() } -> std::same_as<void>;
};

// Proxy class for Drawable objects
class ShapeProxy {
   public:
    // Constructor accepts any Drawable type
    template <Drawable T>
    ShapeProxy(T obj) : impl_(std::make_unique<Model<T>>(std::move(obj))) {}

    // Copy constructor
    ShapeProxy(const ShapeProxy& other) : impl_(other.impl_->clone()) {}

    // Move constructor
    ShapeProxy(ShapeProxy&&) noexcept = default;

    // Copy assignment
    ShapeProxy& operator=(const ShapeProxy& other) {
        if (this != &other) {
            impl_ = other.impl_->clone();
        }
        return *this;
    }

    // Move assignment
    ShapeProxy& operator=(ShapeProxy&&) noexcept = default;

    // Interface method
    void draw() const {
        impl_->draw();
    }

   private:
    // Abstract base class for type erasure
    struct Concept {
        virtual ~Concept() = default;
        virtual void draw() const = 0;
        virtual std::unique_ptr<Concept> clone() const = 0;
    };

    // Template derived class to hold concrete objects
    template <typename T>
    struct Model : Concept {
        Model(T obj) : data_(std::move(obj)) {}
        void draw() const override {
            data_.draw();
        }
        std::unique_ptr<Concept> clone() const override {
            return std::make_unique<Model<T>>(*this);
        }
        T data_;
    };

    // Pointer to the abstract base class
    std::unique_ptr<Concept> impl_;
};

// Concrete implementation
struct Circle {
    void draw() const {
        fmt::println("This is a circle!");
    }
};

// Concrete implementation
struct Rectangle {
    void draw() const {
        fmt::println("This is a Rectangle!");
    }
};

int main() {
    std::vector<ShapeProxy> shapes;
    // invoke move
    shapes.emplace_back(Circle{});
    shapes.emplace_back(Rectangle{});

    // Draw all shapes
    for (const auto& shape : shapes) {
        shape.draw();
    }

    // Demonstrate copy and assignment
    ShapeProxy shape1 = Circle();
    ShapeProxy shape2 = Rectangle();

    shape1.draw();  // Output: Drawing a Circle
    shape2.draw();  // Output: Drawing a Square

    // Copy shape1 to shape3
    ShapeProxy shape3 = shape1;
    shape3.draw();  // Output: Drawing a Circle

    // Assign shape2 to shape1
    shape1 = shape2;
    shape1.draw();  // Output: Drawing a Square
}
```

polymorphism with microsoft [proxy](https://github.com/microsoft/proxy)

```cpp
#include <fmt/core.h>
#include <proxy/proxy.h>
#include <vector>

PRO_DEF_MEM_DISPATCH(MemDraw, Draw);
PRO_DEF_MEM_DISPATCH(MemArea, Area);

struct Drawable : pro::facade_builder 
::add_convention<MemDraw, void()>
::add_convention<MemArea, double() noexcept>
::support_copy<pro::constraint_level::nontrivial>::build {};

struct Rectangle {
    Rectangle(double width, double height) : width_(width), height_(height) {}

    void Draw() const {
        fmt::println("Rectangle with width: {}, height: {}", width_, height_);
    }
    double Area() const noexcept { return width_ * height_; }

    double width_;
    double height_;
};

struct Circle {
    Circle(double radius) : radius_(radius) {}
    void Draw() const {
        fmt::println("Rectangle with radius: {}", radius_);
    }

    double Area() const noexcept { return 3.14 * radius_ * radius_; }

    double radius_;
};

int main() {
    std::vector<pro::proxy<Drawable>> shapes;
    auto p1 = pro::make_proxy<Drawable, Rectangle>(3, 5);
    shapes.emplace_back(p1);
    auto p2 = pro::make_proxy<Drawable, Circle>(10);
    shapes.emplace_back(p2);

    for (auto &&shape : shapes) {
        shape->Draw();
        fmt::println("area={}", shape->Area());
    }
}
```