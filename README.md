# ccORM[version 1.3]
> ccORM is the best ORM object relational mapping underlying library, which adopts the most philosophical, classic and minimalist design, low code and modular development, and friendly user experience.
> 🚀 Support Linux and Windows Platforms(Mac platform does not adapt string type detection temporarily), performance surpasses RTTI and protobuf and is compile time static reflection. As the name suggests, it is already an optimized machine code.

 ![Benchmark results (not cached)](./test.jpg)

 ## Advantage
- [x] powerful static reflection, maximum performance, minimum overhead, the fastest response, the easiest to maintain, and the low code
- [x] supports serializing objects or vector objects to strings, serializing objects to get JSON, and deserializing JSON format strings to objects
- [x] allows TM type, that is, date type (and serialization and deserialization of date type)
- [x] tolerate multithreading and high concurrency. SQLite supports dual threads, while MySQL and PgSQL support complete CPU threads
- [x] intelligent automatic table creation and deep binding between model classes and database is a function similar to decorator realized by macro
- [x] the function of automatically Ping the database at an interval, so that the database does not sleep and is always in the state of wake-up and recovery
- [x] add, delete, modify and query based on OOP. The insertion function also returns the new ID
- [x] surpassing RTTI and protobuf, it is the fastest and most violent idea similar to "dynamic type C++"
- [x] you can nest structs with structs, so as to realize one-to-one or one to many, or self nesting, etc
- [x] Provide DataMapper and ActiveRecord for curd
- [x] with compile time type detection
- [x] the VARCHAR data type is text<>
- [x] native types support unsigned type [for example: uint8_t, uint16_t, uint32_t, uint64_t]
- [x] Automatic table creation, global initialization, non intrusive
- [x] Add build macro for intermediate table `M_TABLE`
- [x] New the fastest developer mode, and the table will be rebuilt every time (note that it can only be used in the development environment!)
## Coming soon
One to many query, many to many query, index column establishment, and cache query...

## Model layer
```c++
struct Type; D_M_TABLE(Type, Tab)
Struct(Tab) {
  uint32_t id;
  bool ok;
  text<15> name;
  tm date;
  vector<Type> types;
  Tab(uint32_t a = 0, bool b = false, const char* c = "", tm d = now(), vector<Type> e = {}) :
	id(a), ok(b), name(c), date(d), types(e) {}
  FIELD(id, ok, name, date)
};
CONSTRUCT(Tab, id, ok, name, date, types)
PROTO(Tab, id, ok, name, date)
REGIST(Tab,
  TC::PRIMARY_KEY | TC::AUTO_INCREMENT, "",
  TC::DEFAULT, "false",
  TC::DEFAULT, "ww'zzgg",
  TC::DEFAULT | TC::NOT_NULL, "");
Struct(Type) {
  uint8_t id;
  text<10> language;
  double bigBlob;
  vector<Tab> tabs;
  Type(uint8_t a = 0, const char* b = "", double c = 0, vector<Tab> d = {}) :
	id(a), language(b), bigBlob(c), tabs(d) {}
  FIELD(id, language, bigBlob)
};
CONSTRUCT(Type, id, language, bigBlob, tabs)
PROTO(Type, id, language, bigBlob)
REGIST(Type,
  TC::PRIMARY_KEY | TC::AUTO_INCREMENT, "",
  TC::DEFAULT, "c/c++",
  TC::EMPTY, "");
M_TABLE(Type, id, Tab, id)
```
## Main function
```c++
#include "src/ccORM.hh"
auto D =
//D_mysql();
//D_pgsql();
D_sqlite("any.db");//Select database
#include "module.hpp"
void test() {
  Tab::ptr t = Tab::create(1, true, "Date change", now(), vector<Type>{ Type{ 1,"typescript" } });
  t->Update();//update
  t->set(5, false, "yield", now(), vector<Type>{ Type{ 1,"python" }, Type{ 2,"ruby" } }); cout << t << '\n';
  *t = json::parse(u8R"({"id":4,"ok":false,"name":"Flawless masterpiece","date":"2021-09-08 01:04:30",
"lang":[{"id":1,"language":"c++"},{"id":2,"language":"js"},{"id":3,"language":"rust"}]})").get<Tab>();
  t->lang[1].language = "golang"; cout << t << '\n';
  t->Insert();//insert, the return value is long long type
  cout << Tab::Q()->$()->GetArr();
  t->Delete();//delete
  *t = Tab::Q()->$()->where(Tab::$id == 1)->GetOne(); cout << t << '\n';
}
int main() {
  clock_t start = clock(); test();
  Timer t; bool run = true;
  t.setTimeout([&run] {
	int i = 0; for (; i < 99999; ++i) {
	Tab::Q()->$()->where(Tab::$id == 2)->GetOne(); } printf("<%d>", i);
	run = false;
	}, 6);
  int i = 0; for (; i < 98888; ++i) {
	Tab::Q()->$()->where(Tab::$id == 1)->GetOne();
  }//Multithreading test
  printf("<%d>", i);
  while (run) { this_thread::yield(); }
  printf("\nuse %.6f seconds", (float)(clock() - start) / CLOCKS_PER_SEC);
  return 0;
}
```

## Features
- Modularization
- Header files only
- Low code
- High performance

## Premise
Cmake requirements: [it is best to install package with vcpkg]
```shell
mkdir build
cd build
cmake ..
cmake --build .
```
...or then add it where you need to connect to the library
target_link_libraries(main ${MYSQL_LIBRARY})
perhaps
This is just an example. Note: please modify the `build_linux.sh` file .Through `sh ./build_linux.sh` compilation
```
g++ -std=c++17 *.cc -o main -I./src -ldl -Wstack-protector -fstack-protector-all
-pthread -ggdb -lmariadb -lmariadbclient -Wwrite-strings -lssl -lcrypto -lz -fPIC 
```
# Supported compilers(minimum version):
    - Linux: G++ 9.2, Clang++ 9.0
    - MacOS: Apple clang version 12.0.0 
    - Windows: MSVC C++ compiler version 1930.

## General naming rules
Constexpr -> lowercase + uppercase hump separation, and the last one is uppercase ending => `nameBegiN`  
Static attribute -> `_` Beginning, followed by all lowercase + underscore => `_name_begin`  
Static global variables -> ` res_` Beginning, followed by all uppercase + underscore => `RES_NAME_BEGIN`  
private Attribute -> lowercase + uppercase hump separation, `_` Finishing => `nameBegin_`  
public Attribute -> lowercase + uppercase hump separation => `nameBegin`  
Parameter -> lowercase + underscore => `name_begin`  
Special fields -> `_` Beginning, separated by lowercase + underline in the middle, `_` Finishing => `_name_begin_`  
Struct or class -> starts with uppercase, followed by lowercase + uppercase, separated by hump => `NameBegin`  
Macro or static constexpr -> uppercase + underscore => `NAME_BEGIN`  

### Attributions
    ccORM uses the following libraries.

    lithium

    https://github.com/matt-42/lithium

	Copyright (c) 2014 Matthieu Garrigues

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
	

	json 

    https://github.com/nlohmann/json

    Copyright (c) 2013-2021 Niels Lohmann
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
