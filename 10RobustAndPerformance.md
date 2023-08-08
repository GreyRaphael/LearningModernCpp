# Robustness and Performance

- [Robustness and Performance](#robustness-and-performance)
  - [exception](#exception)

## exception

custom exception

```cpp
#include <iostream>
#include <sstream>

class advanced_error : public std::runtime_error {
    int error_code;

    std::string make_message(int const e) {
        std::stringstream ss;
        ss << "error with code " << e;
        return ss.str();
    }

   public:
    advanced_error(int const e) : std::runtime_error(make_message(e).c_str()), error_code(e) {
    }
};

int main() {
    {
        try {
            throw advanced_error(110);
        } catch (advanced_error const& e) {
            std::cout << e.what() << '\n';
        }
    }
}
```

> `noexcept` 编译期完成声明和检查工作.`noexcept` 主要是解决的问题是减少运行时开销. 运行时开销指的是, 编译器需要为代码生成一些额外的代码用来包裹原始代码，当出现异常时可以抛出一些相关的堆栈stack unwinding错误信息, 这里面包含，错误位置, 错误原因, 调用顺序和层级路径等信息.当使用`noexcept`声明一个函数不会抛出异常候, 编译器就不会去生成这些额外的代码, 直接的减小的生成文件的大小, 间接的优化了程序运行效率.
1. 当`noexcept` 是标识符时, 它的作用是在函数后面声明一个函数是否会抛出异常.
2. 当`noexcept` 是函数时, 它的作用是检查一个函数是否会抛出异常.

where `noexcept` recommendation
- destructor
- move constructor
- move assignment