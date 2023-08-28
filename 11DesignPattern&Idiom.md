# Implementing Patterns & Idioms

- [Implementing Patterns \& Idioms](#implementing-patterns--idioms)
  - [Avoiding repetitive `if...else` statements in factory patterns](#avoiding-repetitive-ifelse-statements-in-factory-patterns)
  - [`pimpl`](#pimpl)
  - [named parameter idiom](#named-parameter-idiom)
  - [`non-virtual interface idiom`](#non-virtual-interface-idiom)
  - [`attorney-client idiom`](#attorney-client-idiom)
  - [curiously recurring template pattern (`CRTP`)](#curiously-recurring-template-pattern-crtp)
  - [thread-safe singleton](#thread-safe-singleton)

Definition:
- **Idioms**: provide instructions on how to resolve implementation-specific issues in a programming language, such as memory management in C++
- **Design Patters**: focus on universal structural principles.

## Avoiding repetitive `if...else` statements in factory patterns

```cpp
#include <iostream>
#include <map>

class Image {};

class BitmapImage : public Image {};
class PngImage : public Image {};
class JpgImage : public Image {};

struct IImageFactory {
    virtual std::unique_ptr<Image> Create(std::string_view type) = 0;
};

struct ImageFactoryClassic : public IImageFactory {
    virtual std::unique_ptr<Image> Create(std::string_view type) override {
        if (type == "bmp")
            return std::make_unique<BitmapImage>();
        else if (type == "png")
            return std::make_unique<PngImage>();
        else if (type == "jpg")
            return std::make_unique<JpgImage>();

        return nullptr;
    }
};

struct ImageFactory : public IImageFactory {
    virtual std::unique_ptr<Image> Create(std::string_view type) override {
        static std::map<std::string, std::function<std::unique_ptr<Image>()>> mapping{
            {"bmp", []() { return std::make_unique<BitmapImage>(); }},
            {"png", []() { return std::make_unique<PngImage>(); }},
            {"jpg", []() { return std::make_unique<JpgImage>(); }}};

        auto it = mapping.find(type.data());
        if (it != mapping.end())
            return it->second();

        return nullptr;
    }
};

struct IImageFactoryByType {
    virtual std::unique_ptr<Image> Create(std::type_info const &type) = 0;
};

struct ImageFactoryByType : public IImageFactoryByType {
    virtual std::unique_ptr<Image> Create(std::type_info const &type) override {
        auto it = mapping.find(&type);
        if (it != mapping.end())
            return it->second();

        return nullptr;
    }

   private:
    static std::map<std::type_info const *, std::function<std::unique_ptr<Image>()>> mapping;
};

std::map<std::type_info const *, std::function<std::unique_ptr<Image>()>> ImageFactoryByType::mapping{
    {&typeid(BitmapImage), []() { return std::make_unique<BitmapImage>(); }},
    {&typeid(PngImage), []() { return std::make_unique<PngImage>(); }},
    {&typeid(JpgImage), []() { return std::make_unique<JpgImage>(); }}};

int main() {
    {
        auto factory = ImageFactoryClassic{};
        auto img = factory.Create("png");
    }
    {
        auto factory = ImageFactory{};
        auto img = factory.Create("jpg");
    }
    {
        auto factory = ImageFactoryByType{};
        auto img = factory.Create(typeid(PngImage));
    }
}
```

## `pimpl`

> `pimpl`: pointer to implementation, it enables changing the implementation without modifying the interface and, therefore, avoiding the need to recompile the code that is using the interface. 

Tips: always put all the private member data and functions, except for the virtual ones, in the pimpl class and leave the protected data members and functions and all the private virtual functions in the public class.

a simple example for `pimpl`, and a complicate [example](examples/ch11-piml.cc)

```bash
CMakeLists.txt
MyClass.h
MyClass.cpp
main.cpp
```

```cmake
cmake_minimum_required(VERSION 3.18.0)
project(proj1 VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

add_executable(proj1 main.cpp MyClass.cpp)
```

```cpp
// Header file (MyClass.h)
#include <memory>

class MyClassImpl;  // Forward declaration

class MyClass {
   public:
    MyClass();
    ~MyClass();

    void doSomething();

   private:
    std::unique_ptr<MyClassImpl> pimpl;  // Unique pointer to implementation
};
```

```cpp
// Source file (MyClass.cpp)
#include "MyClass.h"

#include <iostream>

// Private implementation class
class MyClassImpl {
   public:
    void doSomething() {
        // Implementation details...
        std::cout << "do somthing" << '\n';
    }
};

// User Class
MyClass::MyClass() : pimpl(std::make_unique<MyClassImpl>()) {}  // Creating instance of the implementation class

MyClass::~MyClass() = default;  // Unique_ptr automatically manages the deallocation

void MyClass::doSomething() {
    pimpl->doSomething();  // Calling the implementation method
}
```

```cpp
// main.cpp
#include <iostream>
#include "MyClass.h"

int main() {
    MyClass obj;
    obj.doSomething();
}
```

## named parameter idiom

> The named parameter idiom has the advantage that it allows you to specify values only for the parameters that you want, in any order, using a name, which is much more intuitive than a fixed, positional order.

example: simple `named paramter idiom`

```cpp
#include <iostream>
#include <string>

class Person {
   public:
    Person() = default;
    Person(std::string const& name, int const age) : m_Name(name), m_Age(age) {}

    Person& setName(const std::string& name) {
        m_Name = name;
        return *this;
    }

    Person& setAge(int age) {
        m_Age = age;
        return *this;
    }

    void printInfo() const {
        std::cout << "Name: " << m_Name << "\tAge: " << m_Age << std::endl;
    }

   private:
    std::string m_Name;
    int m_Age;
};

class PersonBuilder {
   public:
    PersonBuilder() {
        m_Person = std::make_unique<Person>();
    }

    PersonBuilder& withName(const std::string& name) {
        m_Person->setName(name);
        return *this;
    }

    PersonBuilder& withAge(int age) {
        m_Person->setAge(age);
        return *this;
    }

    Person& build() {
        return *m_Person;
    }

   private:
    std::unique_ptr<Person> m_Person;
};

int main() {
    {
        Person p{"Tom", 22};
        p.printInfo();
    }
    {
        Person p1 = PersonBuilder().withAge(24).withName("James").build();
        p1.printInfo();

        // change parameter order
        Person p2 = PersonBuilder().withName("Tim").withAge(25).build();
        p2.printInfo();
    }
}
```

example: avoid getter-setter

```cpp
#include <iostream>
#include <string>

class PersonBuilder;

class Person {
   public:
    Person() = default;
    Person(std::string const& name, int const age) : m_Name(name), m_Age(age) {}

    void printInfo() const {
        std::cout << "Name: " << m_Name << "\tAge: " << m_Age << std::endl;
    }

   private:
    std::string m_Name;
    int m_Age;
    // PersonBuilder can visit private data
    // avoid writing getter-setter
    friend class PersonBuilder;
};

class PersonBuilder {
   public:
    PersonBuilder() {
        m_Person = std::make_unique<Person>();
    }

    PersonBuilder& withName(const std::string& name) {
        m_Person->m_Name = name;
        return *this;
    }

    PersonBuilder& withAge(int age) {
        m_Person->m_Age = age;
        return *this;
    }

    Person& build() {
        return *m_Person;
    }

   private:
    std::unique_ptr<Person> m_Person;
};

int main() {
    {
        Person p{"Tom", 22};
        p.printInfo();
    }
    {
        Person p1 = PersonBuilder().withAge(24).withName("James").build();
        p1.printInfo();

        // change parameter order
        Person p2 = PersonBuilder().withName("Tim").withAge(25).build();
        p2.printInfo();
    }
}
```

example: with object as parameters

```cpp
#include <iostream>
#include <string>

class Person;

class PersonParams {
   public:
    PersonParams& withName(const std::string& name) {
        name_ = name;
        return *this;
    }

    PersonParams& withAge(int age) {
        age_ = age;
        return *this;
    }

   private:
    std::string name_;
    int age_;
    // Person can visit PersonParams private data
    friend class Person;
};

class Person {
   public:
    Person(std::string const& name, int const age) : m_Name(name), m_Age(age) {}
    Person(PersonParams const& pp) : m_Name(pp.name_), m_Age(pp.age_) {}

    void printInfo() const {
        std::cout << "Name: " << m_Name << "\tAge: " << m_Age << std::endl;
    }

   private:
    std::string m_Name;
    int m_Age;
};

int main() {
    {
        Person p{"Tom", 22};
        p.printInfo();
    }
    {
        Person p1{
            PersonParams().withAge(24).withName("James")};
        p1.printInfo();

        Person p2{
            PersonParams().withName("Tim").withAge(25)};
        p2.printInfo();
    }
}
```

## `non-virtual interface idiom`

> `non-virtual interface idiom` is a design pattern for providing a unified interface for a class while allowing customization of its behavior through virtual functions by making (public) interfaces non-virtual and virtual functions private.

simple example of `non-virtual interface idiom`

```cpp
#include <iostream>

class Base {
   public:
    void performOperation() {
        preOperationHook();
        std::cout << "common code in Base" << '\n';
        postOperationHook();
    }

   private:
    virtual void preOperationHook() {
        std::cout << "Base class pre-operation hook" << std::endl;
    }

    virtual void postOperationHook() {
        std::cout << "Base class post-operation hook" << std::endl;
    }
};

class Derived : public Base {
   private:
    void preOperationHook() override {
        std::cout << "Derived class pre-operation hook" << std::endl;
    }

    void postOperationHook() override {
        std::cout << "Derived class post-operation hook" << std::endl;
    }
};

int main() {
    // implicitly convert Derived to Base
    std::unique_ptr<Base> pd = std::make_unique<Derived>();
    pd->performOperation();
}
// Derived class pre-operation hook
// common code in Base
// Derived class post-operation hook
```

example: **virtual destructor**

> When a base class pointer points to a derived class object, calling delete on that pointer will only invoke the base class destructor if the destructor is not virtual. This can lead to a memory leak if the derived class has allocated resources that need to be cleaned up.

```cpp
#include <iostream>

class Base {
   public:
    Base() { std::cout << "Base ctor" << '\n'; }
    virtual ~Base() { std::cout << "Base dtor" << '\n'; }
    // ~Base() { std::cout << "Base dtor" << '\n'; }
};

class Derived : public Base {
   public:
    Derived() { std::cout << "Derived ctor" << '\n'; }
    ~Derived() { std::cout << "Derived dtor" << '\n'; }
};

int main() {
    {
        // ~Base()有无virtual效果一样
        Derived d;
    }
    // Base ctor
    // Derived ctor
    // Derived dtor
    // Base dtor
    {
        std::cout << "----------" << '\n';
        Base* pb = new Derived{};
        delete pb;
    }
    // Base ctor
    // Derived ctor
    // Derived dtor
    // Base dtor
    {
        // recommended
        std::cout << "----------" << '\n';
        std::unique_ptr<Base> pb = std::make_unique<Derived>();
    }
    // Base ctor
    // Derived ctor
    // Derived dtor
    // Base dtor
}
```

`non-virtual interface idiom` example
> The core principles of the `non-virtual interface idiom` is that virtual functions should not be public; they should be either private or protected, in case the base class implementation could be called from a derived class. 

```cpp
#include <iostream>
#include <vector>

class control {
   private:
    virtual void paint() = 0;

   protected:
    virtual void erase_background() {
        std::cout << "erasing control background..." << '\n';
    }

   public:
    void draw() {
        erase_background();
        paint();
    }

    virtual ~control() {
        std::cout << "destroying control..." << '\n';
    }
};

class button : public control {
   private:
    virtual void paint() override {
        std::cout << "painting button..." << '\n';
    }

   protected:
    virtual void erase_background() override {
        control::erase_background();
        std::cout << "erasing button background..." << '\n';
    }

   public:
    ~button() {
        std::cout << "destroying button..." << '\n';
    }
};

class checkbox : public button {
   private:
    virtual void paint() override {
        std::cout << "painting checkbox..." << '\n';
    }

   protected:
    virtual void erase_background() override {
        button::erase_background();
        std::cout << "erasing checkbox background..." << '\n';
    }

   public:
    ~checkbox() {
        std::cout << "destroying checkbox..." << '\n';
    }
};

int main() {
    {
        button b;
        b.draw();
    }
    {
        std::cout << "--------------------" << '\n';
        std::unique_ptr<control> pc = std::make_unique<button>();
        pc->draw();
    }
    {
        std::cout << "--------------------" << '\n';
        std::unique_ptr<control> pc = std::make_unique<checkbox>();
        pc->draw();
    }
    {
        std::cout << "--------------------" << '\n';
        std::vector<std::unique_ptr<control>> controls;

        controls.emplace_back(std::make_unique<button>());
        controls.emplace_back(std::make_unique<checkbox>());

        for (auto& c : controls)
            c->draw();
    }
}
```

## `attorney-client idiom`

> The `attorney-client idiom` provides a simple mechanism to restrict friends access to only designated private parts of a class.

example for usage

```cpp
#include <iostream>

namespace old_version {

class Client {
    int data_1;
    int data_2;

    void action1() { std::cout << "action1" << '\n'; }
    void action2() { std::cout << "action2" << '\n'; }

    friend class Friend;

   public:
    // public interface
};

class Friend {
   public:
    // Friend can visit all the private data
    void access_client_data(Client& c) {
        c.action1();
        c.action2();
        auto d1 = c.data_1;
        auto d2 = c.data_1;
    }
};
}  // namespace old_version

namespace new_version {

class Client {
    int data_1;
    int data_2;

    void action1() { std::cout << "action1" << '\n'; }
    void action2() { std::cout << "action2" << '\n'; }

    friend class Attorney;

   public:
    // public interface
};

class Attorney {
    // inline: avoids any runtime overhead due to the level of indirection the attorney class introduces. 
    static inline void run_action1(Client& c) {
        c.action1();
    }
    // inline: avoids any runtime overhead due to the level of indirection the attorney class introduces. 
    static inline int get_data1(Client& c) {
        return c.data_1;
    }

    friend class Friend;
};

class Friend {
   public:
    // Friend can only visit private date_1 & action_1
    void access_client_data(Client& c) {
        Attorney::run_action1(c);
        auto d1 = Attorney::get_data1(c);
    }
};
}  // namespace new_version

int main() {
    {
        old_version::Client c1;
        old_version::Friend f1;
        f1.access_client_data(c1);
    }
    {
        new_version::Client c1;
        new_version::Friend f1;
        f1.access_client_data(c1);
    }
}
```

example: `attorney-client idiom` for inheritance by `virtual function`

```cpp
#include <iostream>

class Base {
    virtual void execute() { std::cout << "base" << '\n'; }

    friend class BAttorney;
};

class Derived : public Base {
    virtual void execute() override { std::cout << "derived" << '\n'; }
};

class BAttorney {
    static inline void execute(Base& b) {
        b.execute();
    }

    friend class Foo;
};

class Foo {
   public:
    void run() {
        Base b;
        BAttorney::execute(b);  // prints 'base'

        Derived d;
        BAttorney::execute(d);  // prints 'derived'
    }
};

int main() {
    {
        Foo f;
        f.run();
    }
}
```

## curiously recurring template pattern (`CRTP`)

Polymorphism methods:
- virtual functions: **runtime polymorphism**, Virtual functions can represent a performance issue, especially when they are small and called multiple times in a loop
- curiously recurring template pattern: **compile time polymorphism**

```cpp
#include <iostream>

template <typename T>
class Base {
   public:
    Base() { std::cout << "Base ctor" << '\n'; }
    void interface() {
        static_cast<T*>(this)->func1();
    }
};

class Derived : public Base<Derived> {
   public:
    Derived() { std::cout << "Derived ctor" << '\n'; }
    void func1() { std::cout << "Derived::func1" << '\n'; }
};

template <typename T>
void do_something(Base<T>& b) {
    b.func1();
}

int main() {
    Derived d;
    d.interface();
}
```

Detailed Process:
- Template Instantiation: When the compiler encounters the definition of `Derived`, it needs to instantiate the `Base<Derived>` template. This means that the compiler generates the necessary code for the `Base` class template using the `Derived` class as the template argument.
- Base Class Code Generation: The compiler generates the code for the `Base<Derived>` class using the `Derived` class as the template argument. This includes any member functions, variables, and other code defined within the `Base` template. (只看函数declaration `void interface();`,运行时查看函数体definition `static_cast<T*>(this)->func1();`)
- Derived Class Code Generation: After the base class code has been generated, the compiler generates the code for the `Derived` class. In this case, the `Derived` class only contains the inheritance relationship to `Base<Derived>`, so there might not be any additional code generated.
- Compilation and Linking: The generated code for both the base and derived classes is compiled and linked together to create the final executable or library.The `this` pointer inside `Base<Derived>::interface()` is of type `Base<Derived>*`, but due to the `static_cast`, it’s explicitly casted to `Derived*`, allowing the derived class implementation to be called.

`Base<Derived>::interface()` is declared before the `Derived` class is known to the compiler. However, the `Base` class is a class template, which means it is instantiated only when the compiler encounters code that uses it(at `Derived d;`). At that point, the `Derived` class is already defined and known to the compiler, so calls to `Derived::func1()` can be made.

When `d.interface()` is called, it invokes `Base<Derived>::interface()` because function resolution starts from the base class. Inside `Base<Derived>::interface()`, the line `static_cast<Derived*>(this)->func1()` is executed.

`CRTP` example

```cpp
#include <iostream>

template <typename T>
class control {
   public:
    void draw() {
        static_cast<T*>(this)->erase_background();
        static_cast<T*>(this)->paint();
    }
};

class button : public control<button> {
   private:
    // if private, friend is necessary
    friend class control<button>;

    void erase_background() {
        std::cout << "erasing button background..." << '\n';
    }

    void paint() {
        std::cout << "painting button..." << '\n';
    }
};

class checkbox : public control<checkbox> {
   public:
    void erase_background() {
        std::cout << "erasing checkbox background..." << '\n';
    }

    void paint() {
        std::cout << "painting checkbox..." << '\n';
    }
};

template <typename T>
void draw_control(control<T>& c) {
    c.draw();
}

int main() {
    {
        button b;
        checkbox c;

        draw_control(b);
        draw_control(c);
    }
    {
        button b;
        checkbox c;

        b.draw();
        c.draw();
    }
}
```

`CRTP` example in `std::vector`
> It is not possible to store, in a homogeneous container, such as a vector or list, objects of CRTP types because each base class is a unique type (such as `control<button>` and `control<checkbox>`)

```cpp
#include <iostream>
#include <vector>

class controlbase {
   public:
    virtual void draw() = 0;
    virtual ~controlbase() {}
};

template <typename T>
class control : public controlbase {
   public:
    virtual void draw() override {
        static_cast<T*>(this)->erase_background();
        static_cast<T*>(this)->paint();
    }
};

class button : public control<button> {
   public:
    void erase_background() {
        std::cout << "erasing button background..." << '\n';
    }

    void paint() {
        std::cout << "painting button..." << '\n';
    }
};

class checkbox : public control<checkbox> {
   public:
    void erase_background() {
        std::cout << "erasing checkbox background..." << '\n';
    }

    void paint() {
        std::cout << "painting checkbox..." << '\n';
    }
};

void draw_controls(std::vector<std::unique_ptr<controlbase>>& v) {
    for (auto& c : v) {
        c->draw();
    }
}

int main() {
    std::vector<std::unique_ptr<controlbase>> v;
    v.push_back(std::make_unique<button>());
    v.push_back(std::make_unique<checkbox>());
    draw_controls(v);
}
```

`CRTP` example like inheritance

```cpp
#include <iostream>

template <class T>
class control {
   public:
    void draw() {
        static_cast<T*>(this)->erase_background();
        static_cast<T*>(this)->paint();
    }
};

template <class T>
class fancybutton : public control<fancybutton<T>> {
   public:
    void erase_background() {
        static_cast<T*>(this)->paint_area();
    }

    void paint() {
        std::cout << "painting button..." << '\n';
    }
};

class transparentbutton : public fancybutton<transparentbutton> {
   public:
    void paint_area() {
        std::cout << "painting transparent button background..." << '\n';
    }
};

int main() {
    transparentbutton tb;
    tb.draw();
}
```

example: a practical CRTP example

```cpp
#include <iostream>

// Base class template using CRTP
template <typename Derived>
class Shape {
   public:
    void printArea() {
        Derived& derived = static_cast<Derived&>(*this);
        std::cout << "Area: " << derived.calculateArea() << std::endl;
    }
};

// Derived class template
template <typename T>
class Rectangle : public Shape<Rectangle<T>> {
   private:
    T length;
    T width;

   public:
    Rectangle(T l, T w) : length(l), width(w) {}

    T calculateArea() {
        return length * width;
    }
};

// Derived class template
template <typename T, typename = typename std::enable_if_t<std::is_floating_point_v<T>, T>>
class Ellipse : public Shape<Ellipse<T>> {
   private:
    T length;
    T width;

   public:
    Ellipse(T l, T w) : length(l), width(w) {}

    T calculateArea() {
        return 3.1415926 * length * width;
    }
};

template <typename T>
class Square : public Rectangle<T> {
   public:
    Square(T width) : Rectangle<T>(width, width) {}
};

class Circle : public Ellipse<double> {
   public:
    Circle(double radius) : Ellipse(radius, radius) {}
};

template <typename T>
void do_something(Shape<T>& b) {
    b.printArea();
}

int main() {
    Rectangle<int> rectangle1(5, 3);
    rectangle1.printArea();  // Output: Area: 15
    Rectangle<double> rectangle2(2.5, 4.5);
    rectangle2.printArea();  // Output: Area: 11.25

    // Ellipse<int> ellipse1(5, 3); // error, should be <double>
    Ellipse<double> ellipse2(2.5, 4.5);
    ellipse2.printArea();  // Output: Area: 35.3429

    Square<int> s1{10};
    s1.printArea();
    Square<double> s2{20};
    s2.printArea();

    Circle circle(10);
    circle.printArea();

    std::cout << "=============" << '\n';
    do_something(rectangle1);
    do_something(rectangle2);
    do_something(ellipse2);
    do_something(s1);
    do_something(s2);
    do_something(circle);
}
```

## thread-safe singleton

a simple thread-safe singleton

```cpp
#include <iostream>

class Singleton {
   private:
    // default not delted, because an instance of the class must be actually created in the class code
    Singleton() {}

   public:
    Singleton(Singleton const&) = delete;             // copy ctor
    Singleton& operator=(Singleton const&) = delete;  // copy assignment

    static Singleton& instance() {
        static Singleton single;
        return single;
    }
};

int main() {
    auto& s = Singleton::instance();
}
```

simple thread-safe singleton with `CRTP`

```cpp
#include <iostream>

template <typename T>
class SingletonBase {
   protected:
    SingletonBase() { std::cout << "SingletonBase ctor" << '\n'; }

   public:
    SingletonBase(SingletonBase const&) = delete;
    SingletonBase& operator=(SingletonBase const&) = delete;

    static T& instance() {
        std::cout << "call instance" << '\n';
        static T single;
        return single;
    }
};

class Single : public SingletonBase<Single> {
   private:
    Single() { std::cout << "Single ctor" << '\n'; }
    // static T single;构造的时候调用Single(){}，但是Single是private，所以需要Single对SingleBase<Single>friend
    friend class SingletonBase<Single>;

   public:
    void demo() { std::cout << "demo" << '\n'; }
};

int main() {
    auto& s = Single::instance();
    s.demo();
}
```