# Third-Party Library

- [Third-Party Library](#third-party-library)
  - [Code Organized by CMake](#code-organized-by-cmake)
  - [pybind11](#pybind11)
  - [`nlohmann::json`](#nlohmannjson)
    - [`json` with struct](#json-with-struct)
    - [`json` with vector](#json-with-vector)
    - [`json` with file](#json-with-file)
    - [`json` with raw-string](#json-with-raw-string)
    - [Read/Write bson](#readwrite-bson)
    - [json deal with `NAN` \& `INFINITY`](#json-deal-with-nan--infinity)
  - [`zlib`](#zlib)
  - [`spdlog`](#spdlog)
  - [`fmt`](#fmt)
  - [`SQLite3`](#sqlite3)
  - [CLI11](#cli11)


## Code Organized by CMake

```bash
├─CMakeLists.txt
├─main.cpp
│
├─mylib1
│  │  CMakeLists.txt
│  │  myadd.cpp
│  │
│  └─include
│          myadd.h
│
└─mylib2
    │  CMakeLists.txt
    │  point3d.cpp
    │
    └─include
            point3d.h
```

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.25.0)
project(proj1 VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(mylib1)
add_subdirectory(mylib2)

add_executable(proj1 main.cpp)

target_link_libraries(proj1 PRIVATE mymath)
target_link_libraries(proj1 PRIVATE mypoint)
```

```cmake
# mylib1/CMakeLists.txt
cmake_minimum_required(VERSION 3.25.0)
project(mymath VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)

add_library(mymath SHARED myadd.cpp)
target_include_directories(mymath PUBLIC include)
```

```cmake
# mylib2/CMakeLists.txt
cmake_minimum_required(VERSION 3.25.0)
project(mypoint VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)

add_library(mypoint SHARED point3d.cpp)
target_include_directories(mypoint PUBLIC include)
```

```cpp
// main.cpp
#include <myadd.h>
#include <point3d.h>

#include <iostream>

int main() {
    std::cout << myadd(10, 20) << '\n';
    Point3D p{11, 22, 33};
    p.print();
}
```

```h
// mylib1/myadd.h
int myadd(int x, int y);
```

```cpp
// mylib1/myadd.cpp
#include "myadd.h"

int myadd(int x, int y) {
    return x + y;
}
```

```h
// mylib2/point3d.h
class Point3D {
   private:
    int x_;
    int y_;
    int z_;

   public:
    Point3D(int x, int y, int z) : x_(x), y_(y), z_(z) {}
    void print();
};
```

```cpp
// mylib2/point3d.cpp
#include <iostream>

#include "point3d.h"

void Point3D::print() {
    std::cout << '(' << x_ << ',' << y_ << ',' << z_ << ')' << '\n';
}
```

## pybind11

> [pybind11](https://github.com/pybind/pybind11) is a lightweight header-only library that exposes C++ types in Python and vice versa, mainly to create Python bindings of existing C++ code.

simple example of **pybind11** in Linux

```bash
├── CMakeLists.txt
├── main.cpp
└── pybind11-2.11.1
    └── include
        └── pybind11
```

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.25.0)
project(proj1 VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)

# cmake version > 3.12.0, https://cmake.org/cmake/help/latest/module/FindPython.html
find_package (Python COMPONENTS Development REQUIRED)
message("> Python_INCLUDE_DIRS = ${Python_INCLUDE_DIRS}")

add_library(proj1 SHARED main.cpp)
# include Python.h
target_include_directories(proj1 PRIVATE ${Python_INCLUDE_DIRS})
# include pybind11/*.hpp
# download from https://github.com/pybind/pybind11/tree/master/include/pybind11
target_include_directories(proj1 PRIVATE pybind11-2.11.1/include)
# remove prefix "lib": libproj1->proj1
set(CMAKE_SHARED_LIBRARY_PREFIX "")
```

```cpp
// main.cpp
#include <pybind11/pybind11.h>

int add(int i, int j) {
    return i + j;
}

// 因为编译出来是`proj1.so`, 所以需要`PYBIND11_MODULE(proj1, m)`中设置为`proj1`
PYBIND11_MODULE(proj1, m) { // change here libproj1->proj1
    m.doc() = "pybind11 proj1 plugin"; 

    m.def("add", &add, "A function that adds two numbers");
}
```

```py
# test proj1.so
import proj1

num = proj1.add(200, 100) # 300
```

example: support Linux & Windows
> output is `proj1.cp39-win_amd64.pyd` or `proj1.cpython-310-x86_64-linux-gnu.so`

```cmake
cmake_minimum_required(VERSION 3.25.0)
project(proj1 VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)

find_package (Python COMPONENTS Interpreter Development REQUIRED)
message("> Python_INCLUDE_DIRS = ${Python_INCLUDE_DIRS}")

# get .cp39-win_amd64.pyd or .cpython-310-x86_64-linux-gnu.so
execute_process(
  COMMAND "${Python_EXECUTABLE}" "-c" "import sys, importlib; s = importlib.import_module('distutils.sysconfig' if sys.version_info < (3, 10) else 'sysconfig'); print(s.get_config_var('EXT_SUFFIX') or s.get_config_var('SO'))"
  OUTPUT_VARIABLE _PYTHON_MODULE_EXTENSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
message("> _PYTHON_MODULE_EXTENSION = ${_PYTHON_MODULE_EXTENSION}")
# proj1.cp39-win_amd64.pyd or proj1.cpython-310-x86_64-linux-gnu.so
set(CMAKE_SHARED_LIBRARY_SUFFIX "${_PYTHON_MODULE_EXTENSION}")
# libxxxx.so -> xxxx.so
set(CMAKE_SHARED_LIBRARY_PREFIX "")

add_library(proj1 SHARED main.cpp)
# include pybind11/*.hpp
target_include_directories(proj1 PRIVATE pybind11-2.11.1/include)
# include Python.h
target_include_directories(proj1 PRIVATE ${Python_INCLUDE_DIRS})

if(WIN32)
    # in windows, should linked with python3x.lib, compiler can be MSVC or mingw
    target_link_libraries(proj1 PRIVATE Python::Python)
endif()
```

```cpp
// main.cpp
#include <pybind11/pybind11.h>

int add(int i, int j) {
    return i + j;
}

PYBIND11_MODULE(proj1, m) {
    m.doc() = "pybind11 proj1 plugin";  // optional module docstring

    m.def("add", &add, "A function that adds two numbers");
}
```

## `nlohmann::json`

> [nlohmann::json](https://github.com/nlohmann/json)

simple example

```bash
include
    └──nlohmann
        └── json.hpp
main.cpp
CMakeLists.txt
```

```cmake
cmake_minimum_required(VERSION 3.25.0)
project(proj1 VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)

add_executable(proj1 main.cpp)
target_include_directories(proj1 PRIVATE include)
```

```cpp
// main.cpp
#include <iostream>
#include <nlohmann/json.hpp>

class Foo {
   private:
    int id_ = 0;
    double score_ = 0;

   public:
    Foo() = default;
    nlohmann::json to_json() {
        nlohmann::json j;
        j["id"] = id_;
        j["score"] = score_;
        return j;
    }
    void from_json(nlohmann::json const& j) {
        id_ = j["id"];
        score_ = j["score"];
    }
    void print_info() {
        std::cout << "id=" << id_ << ", score=" << score_ << '\n';
    }
};

int main() {
    Foo f;
    f.print_info();  // id=0, score=0
    f.from_json({{"id", 10}, {"score", 90.5}});
    f.print_info();  // id=10, score=90.5
    auto j = f.to_json();
    std::cout << j << '\n';  // {"id":10,"score":90.5}
}
```

### `json` with struct

```cpp
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

struct Person {
    int age;
    double height;
    char symbols[3];
};

struct Student {
    std::string name;
    int scores[3];
};

void to_json(json& j, const Person& p) {
    j = json{{"age", p.age}, {"height", p.height}, {"symbols", p.symbols}};
}

void from_json(const json& j, Person& p) {
    j.at("age").get_to(p.age);
    j.at("height").get_to(p.height);
    j.at("symbols").get_to(p.symbols);
}

void to_json(json& j, const Student& s) {
    j = json{{"name", s.name}, {"scores", s.scores}};
}

void from_json(const json& j, Student& s) {
    j.at("name").get_to(s.name);
    j.at("scores").get_to(s.scores);
}

int main() {
    // Serialize Person
    Person person{25, 175.5, {'a', 'b', 'c'}};
    json personJson = person;
    std::cout << "Person JSON:\n"<< personJson.dump(2) << std::endl;

    // Serialize Student
    Student student{"Alice", {90, 85, 95}};
    json studentJson = student;
    std::cout << "Student JSON:\n"<< studentJson.dump(2) << std::endl;
}
```

### `json` with vector

```cpp
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>

int main() {
    const int N = 5;
    std::vector<nlohmann::json> jsons(N);
    for (size_t i = 0; i < N; ++i) {
        jsons[i]["id"] = 10 + i;
    }
    nlohmann::json root_json;
    root_json["jsons"] = jsons;

    // to_json
    std::cout << root_json << '\n';  // {"jsons":[{"id":10},{"id":11},{"id":12},{"id":13},{"id":14}]}

    // parse json
    std::string json_str = R"({"jsons":[{"id":20},{"id":21},{"id":22},{"id":23},{"id":24}]})";
    auto j = nlohmann::json::parse(json_str);
    // for (auto& e : j["jsons"]) {
    //     std::cout << e["id"] << ',';
    // } // 20,21,22,23,24,
    for (size_t i = 0; i < N; ++i) {
        std::cout << j["jsons"][i]["id"] << ',';
    } // 20,21,22,23,24,
}
```

example: from/to json for vector

```cpp
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>

class Operator {
   public:
    Operator(int val) : _vsum(val + 100) {}
    void to_json(nlohmann::json& state) {
        state["_vsum"] = _vsum;
    }
    void from_json(const nlohmann::json& state) {
        _vsum = state["_vsum"];
    }
    void print() { std::cout << _vsum << '\n'; }

   private:
    int _vsum = 0;
};

class Factor1 {
   public:
    Factor1() {
        for (int i = 0; i < PRICE_COUNT_; i++) {
            auto op = Operator(i);
            _rolling_ask_prices.push_back(op);
        }
    }
    void to_json(nlohmann::json& state);
    void from_json(const nlohmann::json& state);

    std::vector<Operator> _rolling_ask_prices;

   private:
    int const PRICE_COUNT_ = 5;
};

void Factor1::to_json(nlohmann::json& state) {
    std::vector<nlohmann::json> j1s(PRICE_COUNT_);
    for (size_t i = 0; i < PRICE_COUNT_; ++i) {
        _rolling_ask_prices[i].to_json(j1s[i]);
    }
    state["_rolling_ask_prices"] = j1s;
}

void Factor1::from_json(const nlohmann::json& state) {
    auto j1s = state["_rolling_ask_prices"];
    for (size_t i = 0; i < PRICE_COUNT_; ++i) {
        _rolling_ask_prices[i].from_json(j1s[i]);
    }
}

int main() {
    {
        nlohmann::json state;
        Factor1 f1;
        f1.to_json(state);
        std::cout << state.dump() << '\n';  //{"_rolling_ask_prices":[{"_vsum":100},{"_vsum":101},{"_vsum":102},{"_vsum":103},{"_vsum":104}]}
    }
    {
        std::string json_str = R"({"_rolling_ask_prices":[{"_vsum":200},{"_vsum":201},{"_vsum":202},{"_vsum":203},{"_vsum":204}]})";
        nlohmann::json state = nlohmann::json::parse(json_str);
        Factor1 f1;
        f1.from_json(state);
        for (auto& op : f1._rolling_ask_prices) {
            op.print();
        }
    }
}
```

### `json` with file

example: `load` from json file & `dump` to json file

```json
// example.json
{
    "name":"grey",
    "年龄":23,
    "scores":[60, 95, 32]
}
```

```json
// output.json
{"id":100010,"name":"grey","scores":[60,95,32],"年龄":23}
```

```cpp
// main.cpp
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

int main() {
    // load from file
    std::ifstream fin{"example.json"};
    auto j = nlohmann::json::parse(fin); // nlohmann::json

    std::cout << j << '\n';                           // {"age":23,"name":"grey","scores":[60,95,32]}
    std::cout << j["scores"] << '\n';                 // [60,95,32]
    std::cout << typeid(j["scores"]).name() << '\n';  // nlohmann allocator

    // dump to file
    j["id"] = 100010;
    std::ofstream fout{"output.json"};
    fout << j;
}
```

### `json` with raw-string

example: json `load` from string & `dump` to string

```cpp
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <vector>

int main() {
    // generate data
    std::ifstream fin{"example.json"};
    std::ostringstream oss;
    oss << fin.rdbuf();

    // load from string
    {
        std::string raw_str1 = oss.str();
        std::cout << raw_str1 << '\n';
        // load from raw string, 无需考虑\"
        auto j1 = nlohmann::json::parse(raw_str1);
        std::cout << j1 << '\n';

        // load from string literal
        std::string raw_str2 = R"({"name":"Tim","scores":[60,95,32],"年龄":54})";
        auto j2 = nlohmann::json::parse(raw_str2);
        std::cout << j2 << '\n';

        // load from string literal, stupid
        std::string raw_str3 = "{\"name\":\"Jerry\",\"scores\":[60,95,32],\"年龄\":66}";
        auto j3 = nlohmann::json::parse(raw_str3);
        std::cout << j3 << '\n';
    }

    // generate data
    std::vector<int> v{11, 22, 33};
    nlohmann::json j{
        {"name", "James"},
        {"年龄", 43},
        {"scores", v}

    };
    // dump to string
    {
        std::string s1 = j.dump();
        std::cout << s1 << '\n';

        // string to file
        std::ofstream fout{"output.json"};
        fout << s1;
    }
}
```

### Read/Write bson

[bson](https://json.nlohmann.me/api/basic_json/to_bson): Binary  Json

```cpp
#include <cmath>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main() {
    // Create a JSON object
    json jsonData = {
        {"name", "Tom"},
        {"age", 23},
        // {"field", NAN}, // NOT support NAN, INFINITY
        {"city", 12.3}};

    // write bson
    {
        // Convert JSON to BSON
        std::vector<uint8_t> bsonData = json::to_bson(jsonData);

        // Write BSON data to a file
        std::ofstream outFile("data.bson", std::ios::binary);
        if (!outFile) {
            std::cerr << "Failed to open file for writing." << std::endl;
            return 1;
        }
        outFile.write(reinterpret_cast<const char*>(bsonData.data()), bsonData.size());
        outFile.close();
    }
    // read bson
    {
        // Read BSON data from the file
        std::ifstream inFile("data.bson", std::ios::binary);
        if (!inFile) {
            std::cerr << "Failed to open file for reading." << std::endl;
            return 1;
        }
        std::vector<char> buffer((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
        inFile.close();

        // Convert BSON to JSON
        json decodedData = json::from_bson(buffer);

        // Print the decoded JSON data
        std::cout << decodedData.dump(4) << std::endl;
    }
}
```

### json deal with `NAN` & `INFINITY`

json not support `NAN` & `INFINITY`
> `NAN` & `INFINITY` are dumped as `null` in `nlohmann::json`

```cpp
#include <cmath>
#include <iostream>
#include <nlohmann/json.hpp>

int main() {
    nlohmann::json j{
        {"key1", NAN},
        {"key2", INFINITY},
        {"key3", 3.1415926}};
    std::cout << j << '\n'; // {"key1":null,"key2":null,"key3":3.1415926}
    std::string j_str = j.dump();

    auto j2 = nlohmann::json::parse(j_str);
    std::cout << j2 << '\n'; // // {"key1":null,"key2":null,"key3":3.1415926}
}
```

Solution for parse, visit [solutions](examples/ch15-json-nan-inf.cc):
1. single double value: **double -> bytes -> double**
2. double in a container: **double container -> bytes container -> double container**

## `zlib`

download [zlib](https://github.com/madler/zlib/releases) and build(`Release`) by its `CMakeLists.txt`, copy the output to following project in `zilb` directory

zlib Read/Write *.gz file

```bash
├─main.cpp
├─CMakeLists.txt
│ 
└─zlib
    ├─include
    │  ├─zconf.h
    │  └─zlib.h
    └─lib
        ├─zlib.lib
        ├─zlib.dll
        └─zlibstatic.lib
```

```cmake
# current project CMakeLists.txt
cmake_minimum_required(VERSION 3.25.0)
project(proj1 VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)

add_executable(proj1 main.cpp)
target_include_directories(proj1 PRIVATE zlib/include)

target_link_directories(proj1 PRIVATE zlib/lib)

# # static library, link zlibstatic.lib
# target_link_libraries(proj1 PRIVATE zlibstatic)

# shared library, link zlib.lib
target_link_libraries(proj1 PRIVATE zlib)
```

```cpp
//main.cpp
#include <zlib.h>

#include <fstream>
#include <iostream>
#include <sstream>


std::string readGzFile(const std::string& filename) {
    gzFile file = gzopen(filename.c_str(), "rb");
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return "";
    }

    std::string result;
    char buffer[2048];
    int bytesRead = 0;

    while ((bytesRead = gzread(file, buffer, sizeof(buffer))) > 0) {
        result.append(buffer, bytesRead);
    }

    gzclose(file);

    return result;
}

void writeGzFile(const std::string& filename, const std::string& data) {
    gzFile file = gzopen(filename.c_str(), "wb");
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    gzwrite(file, data.c_str(), data.length());

    gzclose(file);
}

int main() {
    std::string source_filename{"data.json"};
    std::string gz_filename{"test.json.gz"};
    // generate string data
    std::ifstream fin{source_filename};
    std::ostringstream oss;
    oss << fin.rdbuf();
    std::string data = oss.str();

    {
        // write gz file
        writeGzFile(gz_filename, data);
    }
    {
        // read gz file
        std::string data = readGzFile(gz_filename);
        std::cout << data << '\n';
    }
}
```

## `spdlog`

> [spdlog](https://github.com/gabime/spdlog): Very fast, header-only/compiled, C++ logging library. ci

```cpp
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

int main(){
    // config logger
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("log/custom.log", true);  // true: delete previous log.txt

    spdlog::logger logger("mylogger", {console_sink, file_sink});
    spdlog::set_default_logger(std::make_shared<spdlog::logger>(logger));  // SPDLOG_XXX is default logger
    spdlog::set_pattern("[%^%4!l%$] [%T] [%s:%#:%!] %v");

    // use spdlog
    SPDLOG_INFO("test info");
    SPDLOG_WARN("test warning");
    SPDLOG_ERROR("test error");
}
```

## `fmt`

[fmt](https://github.com/fmtlib/fmt) is an open-source formatting library providing a fast and safe alternative to C stdio and C++ iostreams.
> [fmt cheat sheet](https://hackingcpp.com/cpp/libs/fmt.html)

## `SQLite3`

Use `sqlite3` in C++
1. download source code of [sqlite](https://www.sqlite.org/download.html), e.g. `sqlite-amalgamation-3430100.zip`
2. extract `sqlite3.h` & `sqlite3.c` from `sqlite-amalgamation-3430100.zip` file to your dependency folder
3. create `CMakeLists.txt`
4. build with your project

example: C++ Read/Write SQLite3

```bash
_deps/
    sqlite-3.43.1/
        sqlite3.c
        sqlite3.h
        CMakeLists.txt
main.cpp
CMakeLists.txt
```

```cmake
# _deps/sqlite-3.43.1/CMakeLists.txt
# add_library(sqlite3 sqlite3.c) # static lib
add_library(sqlite3 SHARED sqlite3.c) # shared lib
```


```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.25.0)
project(proj1 VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(_deps/sqlite-3.43.1)

add_executable(proj1 main.cpp)
target_include_directories(proj1 PRIVATE _deps/sqlite-3.43.1)
target_link_libraries(proj1 PRIVATE sqlite3)
```

```cpp
#include <sqlite3.h>

#include <cmath>   // std::round
#include <format>  // std::format
#include <iostream>
#include <tuple>
#include <vector>

void writeDb(sqlite3 *db, std::string const &sql) {
    std::cout << "Writing to database: " << sql << '\n';
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        // Error handling if executing the INSERT statement fails
        std::cout << "insert error: " << sqlite3_errmsg(db) << '\n';
    }
}

std::vector<std::tuple<std::string, int, double>> readDb(sqlite3 *db, std::string const &sql) {
    std::cout << "Reading from database: " << sql << '\n';
    std::vector<std::tuple<std::string, int, double>> results;
    int rc = sqlite3_exec(
        db, sql.c_str(), [](void *data, int argc, char **argv, char **colNames) {
            // result->data->vec
            auto vec = static_cast<std::vector<std::tuple<std::string, int, double>> *>(data);
            vec->push_back(std::make_tuple(std::string{argv[0]}, std::stoi(argv[1]), std::stod(argv[2])));
            return 0;
        },
        &results, nullptr);
    if (rc != SQLITE_OK) {
        std::cout << "SQL error: " << sqlite3_errmsg(db) << '\n';
    }
    return results;
}

int main() {
    sqlite3 *db;
    int rc = sqlite3_open("your_database.db", &db);
    if (rc) {
        // Error handling if opening the database fails
        std::cout << "cannot open database" << sqlite3_errmsg(db) << '\n';
        return rc;
    }

    auto createTableSql = "CREATE TABLE IF NOT EXISTS your_table (column1 TEXT, column2 INT, column3 REAL);";
    rc = sqlite3_exec(db, createTableSql, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        // Error handling if executing the CREATE TABLE statement fails
        std::cout << "cannot create table" << sqlite3_errmsg(db) << '\n';
        sqlite3_close(db);
        return rc;
    }
    // write date to sqlite3
    {
        for (size_t i = 0; i < 10; ++i) {
            auto name = std::format("stu-{}", i);
            auto score = std::round(1.1 * i * 100) / 100;  // .2f
            auto insertSql = std::format("INSERT INTO your_table (column1, column2, column3) VALUES ('{}', {}, {});", name, i, score);
            writeDb(db, insertSql);
        }
    }
    // read from sqlite3
    {
        auto results = readDb(db, "SELECT * FROM your_table");
        for (auto &row : results) {
            auto [name, id, score] = row;
            std::cout << name << " " << id << " " << score << '\n';
        }
    }
    sqlite3_close(db);
}
```

## CLI11

[CLI11](https://github.com/CLIUtils/CLI11): `vcpkg install cli11`

```cpp
#include <fmt/core.h>
#include <CLI/CLI.hpp>
#include <string>

int main(int argc, char** argv) {
    CLI::App app("future trade client");

    // order
    CLI::App* subcommand1 = app.add_subcommand("order", "send orders");
    int side = 0;
    subcommand1->add_option("-s,--side", side, "trading direction");
    int volume = 100;
    subcommand1->add_option("-v,--vol", volume, "trading volume");
    std::string stock_info{"input/stock.csv"};
    subcommand1->add_option("-i,--in", stock_info, "stock price info");
    std::string client_info{"clients/gewei.json"};
    subcommand1->add_option("-u,--u", client_info, "account info");
    int order_type = 1;
    subcommand1->add_option("-t,--type", side, "trading direction");

    // cancel
    CLI::App* subcommand2 = app.add_subcommand("cancel", "cancel orders");

    // query
    CLI::App* subcommand3 = app.add_subcommand("query", "query");

    app.parse(argc, argv);

    if (app.got_subcommand("order")) {
        fmt::println("send orders");
        fmt::println("input {}: {}, {}, {}, {}, {}", app.count_all(), side, volume, stock_info, client_info, order_type);
    } else if (app.got_subcommand("cancel")) {
        fmt::println("cancel orders");
    } else if (app.got_subcommand("query")) {
        fmt::println("query orders");
    } else {
    }
}
```