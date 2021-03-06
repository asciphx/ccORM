# ccORM[version 1.6]
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
- [x] Provide DataMapper and ActiveRecord for curd
- [x] with compile time type detection
- [x] the VARCHAR data type is text<>
- [x] native types support unsigned type [for example: uint8_t, uint16_t, uint32_t, uint64_t]
- [x] Automatic table creation, global initialization, non intrusive
- [x] Add build macro for intermediate table `M_TABLE`
- [x] New the fastest developer mode, and the table will be rebuilt every time (note that it can only be used in the development environment!)
- [x] Loose boolean type deserialization, including but not limited to true, false, and 0, 1
- [x] Serialization and deserialization support loop nesting and loop dependency, as well as pointer structures
- [x] Supports one-to-one query, which can be as simple, elegant and convenient as JavaScript

## Coming soon
Many to many queries, index columns, unique keys, and cache queries....

## Model layer
```c++
class Tab;
class Type; D_M_TABLE(Type, Tab);


Class(Tab)
uint32_t id;
bool ok;
text<15> name;
tm date;
vector<Type> types;
Type* type;
Tab(uint32_t a = 0, bool b = false, const char* c = "", tm d = now(), vector<Type> e = {}, Type* f = nullptr) :
  id(a), ok(b), name(c), date(d), types(e), type(f) {}
~Tab() { type = nullptr; }
FIELD(id, ok, name, date)
CLASS(Tab, id, ok, name, date, types, type)
PROTO(Tab, id, ok, name, date)
REGIST(Tab,
  TC::PRIMARY_KEY | TC::AUTO_INCREMENT, "",
  TC::DEFAULT, "false",
  TC::DEFAULT, "ww'zzgg",
  TC::DEFAULT | TC::NOT_NULL, "");

Class(Type)
uint8_t id;
text<10> language;
double bigBlob;
vector<Tab> tabs;
Tab* tab;
Type(uint8_t a = 0, const char* b = "", double c = 0, vector<Tab> d = {}, Tab* e = nullptr) :
  id(a), language(b), bigBlob(c), tabs(d), tab(e) {}
~Type() { tab = nullptr; }
FIELD(id, language, bigBlob)
CLASS(Type, id, language, bigBlob, tabs, tab)
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
  *t = json::parse(R"({"id":4,"ok":false,"name":"Flawless masterpiece","date":"2021-09-08 01:04:30","types":
[{"id":1,"language":"c++"},{"id":2,"tabs":[{"id":1,"name":"wtf!","ok":1}]},{"id":3,"language":"rust"}]})").get<Tab>();
  t->lang[1].language = "golang"; cout << t << '\n';//Loose boolean type deserialization, adding 0,1 support
  t->Insert();//insert, the return value is long long type
  cout << Tab::Q().GetArr();
  t->Delete();//delete
  *t = Tab::Q().where(Tab::$id == 1).GetOne(); cout << t << '\n';
}
int main() {
  clock_t start = clock(); test();
  Timer t; bool run = true;
  t.setTimeout([&run] {
	int i = 0; for (; i < 99999; ++i) {
	Tab::Q().where(Tab::$id == 2).GetOne(); } printf("<%d>", i);
	run = false;
	}, 6);
  int i = 0; for (; i < 98888; ++i) {
	Tab::Q().where(Tab::$id == 1).GetOne();
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
If the switch changes, perform the required switch options, eg:
```
cmake -DFastestDev=OFF -DIsDevMode=OFF --build ./
cmake -DFastestDev=OFF -DIsDevMode=ON --build ./
cmake -DFastestDev=ON -DIsDevMode=ON --build ./
```
then
```
cmake --build .
```
Or the following method corresponds to the above switch options. This is just an example.
Note: please modify 'build' yourself_ linux. SH ` file Pass ` sh/ build_ linux. SH ` compile
```
g++ -DFastestDev=0 -DIsDevMode=0 ... In the online environment, the table will not be rebuilt
g++ -DFastestDev=0 -DIsDevMode=1 ... Do not re create the table (the table structure is not changed and can be used), and the development environment
g++ -DFastestDev=1 -DIsDevMode=1 ... Each time, the table will be rebuilt to develop the environment
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
