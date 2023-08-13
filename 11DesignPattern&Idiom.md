# Implementing Patterns & Idioms

- [Implementing Patterns \& Idioms](#implementing-patterns--idioms)
  - [Avoiding repetitive `if...else` statements in factory patterns](#avoiding-repetitive-ifelse-statements-in-factory-patterns)
  - [`pimpl`](#pimpl)

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