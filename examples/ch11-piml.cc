/*------------------------------Header file (MyClass.h)------------------------------*/ 
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

class MyClassCopyable {
   public:
    MyClassCopyable();                                           // default ctor
    ~MyClassCopyable();                                          // dtor
    MyClassCopyable(MyClassCopyable&& rhs) noexcept;             // move ctor
    MyClassCopyable& operator=(MyClassCopyable&& rhs) noexcept;  // move assignment
    MyClassCopyable(const MyClassCopyable& rhs);                 // copy ctor
    MyClassCopyable& operator=(const MyClassCopyable& rhs);      // copy assignment

    int value = 0;
    void doSomething();

   private:
    std::unique_ptr<MyClassImpl> pimpl;
};





/*------------------------------Source file (MyClass.cpp)------------------------------*/ 
#include "MyClass.h"
#include <iostream>

// Private implementation class
class MyClassImpl {
   public:
    void doSomething() {
        // Implementation details...
        std::cout << "do somthing" << '\n';
    }
    void doOtherthing(int a) {
        std::cout << "do something:" << a << '\n';
    }
};

// User Class1
MyClass::MyClass() : pimpl(std::make_unique<MyClassImpl>()) {}  // Creating instance of the implementation class

MyClass::~MyClass() = default;  // Unique_ptr automatically manages the deallocation

void MyClass::doSomething() {
    pimpl->doSomething();  // Calling the implementation method
}

// User Class2
MyClassCopyable::MyClassCopyable() : pimpl(std::make_unique<MyClassImpl>()) {}
MyClassCopyable::~MyClassCopyable() = default;

MyClassCopyable::MyClassCopyable(MyClassCopyable&&) noexcept = default;
MyClassCopyable& MyClassCopyable::operator=(MyClassCopyable&&) noexcept = default;

MyClassCopyable::MyClassCopyable(const MyClassCopyable& rhs)
    : pimpl(std::make_unique<MyClassImpl>(*rhs.pimpl)), value(rhs.value) {}

MyClassCopyable& MyClassCopyable::operator=(const MyClassCopyable& rhs) {
    if (this != &rhs) {
        pimpl = std::make_unique<MyClassImpl>(*rhs.pimpl);
        value = rhs.value;
    }
    return *this;
}

void MyClassCopyable::doSomething() {
    pimpl->doOtherthing(value);  // Calling the implementation method
}





/*------------------------------Source file (main.cpp)------------------------------*/ 
#include <iostream>
#include "MyClass.h"

int main() {
    {
        MyClass obj;
        obj.doSomething();
    }
    {
        MyClassCopyable obj_copyable1;
        obj_copyable1.value = 100;

        // copy ctor
        MyClassCopyable obj_copyable2{obj_copyable1};

        MyClassCopyable obj_copyable3;
        // copy assignment
        obj_copyable3 = obj_copyable1;

        obj_copyable1.doSomething();
        obj_copyable2.doSomething();
        obj_copyable3.doSomething();
    }
}