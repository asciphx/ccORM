# ccORM[版本 0.8]
> ccORM是最好的ORM对象关系映射底层库，采用最哲学最经典极简的设计，低代码和模块化式的开发，友好的用户体验度。
> 🚀 支持Linux、windows平台(Mac平台暂时未适配字符串类型检测)。性能超越RTTI和protobuf，是编译期的静态反射。

 ![基准结果(未缓存)](./test.jpg)

## 优势
- [x] 强大静态反射，最高性能，最低开销，最迅速的反应，最容易维护，以及最少代码
- [x] 支持序列化对象或者vector对象到字符串，序列化对象得到JSON，反序列化Json格式字符串到对象
- [x] 允许tm类型，也就是日期类型（以及日期类型的序列化反序列化）
- [x] 容忍多线程高并发，sqlite支持到了双线程，而mysql和pgsql支持完整的cpu线程
- [x] 智能自动建表，模型类与数据库实现深度绑定，是利用宏实现一种类似装饰器功能
- [x] 自动间隔ping数据库的功能，实现数据库不休眠，一直出于唤醒复苏的状态
- [x] 基于oop的增删改查，插入功能还自带返回新增的id
- [x] 超越RTTI以及protobuf，是目前最快最迅速最猛烈的类似于"动态类型c++"的思想理念
- [x] 可以结构体嵌套结构体，从而实现类似一对一或者一对多，或者自身嵌套自身等等
- [x] 提供DataMapper，ActiveRecord两种方式进行curd
- [x] 具备编译期类型检测，运行时候会忽略错误，也会打印出被忽略的位置

## 即将推出
一对多查询，多对多查询，索引列建立，以及缓存查询

## 模型层
```c++
struct Type : Table<Type> {
  int id;
  string language;
  Type(int a = 0, string b = "") :
	id(a), language(b) {} Type(bool);
}; CONSTRUCT(Type, id, language)
REGIST_PROTO(Type,
  TC::PRIMARY_KEY | TC::AUTO_INCREMENT, "",
  TC::DEFAULT, "c/c++");
struct Tab : Table<Tab> {
  int id;
  bool ok;
  string name;
  tm date;
  vector<Type> lang;
  Tab(int a = 0, bool b = false, string c = "", tm d = now(), vector<Type> e = {}) :
	id(a), ok(b), name(c), date(d), lang(e) {} Tab(bool);
}; CONSTRUCT(Tab, id, ok, name, date, lang)
REGIST_PROTO(Tab,
  TC::PRIMARY_KEY | TC::AUTO_INCREMENT, "",
  TC::DEFAULT, "false",
  TC::DEFAULT, "ww'zzgg",
  TC::DEFAULT | TC::NOT_NULL, "");
```
## 主函数
```c++
#include "src/ccORM.hh"
auto D =
//D_mysql();
//D_pgsql();
D_sqlite("any.db");//选择数据库
#include "module.hpp"
void test() {
  Tab::ptr t = Tab::create(1, true, "日期更变", now(), vector<Type>{ Type{ 1,"typescript" } });
  t->Update();//更新
  t->set(5, false, "yield", now(), vector<Type>{ Type{ 1,"python" }, Type{ 2,"ruby" } }); cout << t << '\n';
  *t = json::parse(R"({"id":4,"ok":false,"name":"完美杰作","date":"2021-09-08 01:04:30",
"lang":[{"id":1,"language":"c++"},{"id":2,"language":"js"},{"id":3,"language":"rust"}]})").get<Tab>();
  t->lang[1].language = "golang"; cout << t << '\n';
  t->Insert();//插入,返回值是long long类型
  cout << Tab::Q()->select()->FindArr();
  t->Delete();//删除
  *t = Tab::Q()->select(&Tab::id, &Tab::name)->FindOne("id = 1"); cout << t << '\n';
}
int main() {
  InitializationOrm<Type, Tab>(); clock_t start = clock(); test();
  Timer t; bool run = true;
  t.setTimeout([&run] {
	int i = 0; for (; i < 5999; ++i) {
	Tab::Q()->select()->FindOne("id = 2"); } printf("<%d>", i);
	run = false;
	}, 6);
  int i = 0; for (; i < 4999; ++i) {
	Tab::Q()->select(&Tab::id, &Tab::name, &Tab::date, &Tab::ok)->FindOne("id = 1");
  }//多线程测试
  printf("<%d>", i);
  while (run) { this_thread::yield(); }
  printf("\nuse %.6f seconds", (float)(clock() - start) / CLOCKS_PER_SEC);
  std::this_thread::sleep_for(std::chrono::milliseconds(500));//wait for something
  return 0;
}
```

## 特征
 - 模块化
 - 仅头文件
 - 低代码
 - 高性能

## 前提
cmake需求：[最好使用vcpkg安装mysql]
find_package(MYSQL REQUIRED)
...然后在需要连接库的位置加入
target_link_libraries(main ${MYSQL_LIBRARY})

或者
下面的方式，这只是示例，注意：请自行修改`build_linux.sh`文件.通过`sh ./build_linux.sh`编译
```
g++ -std=c++17 *.cc -o main -I./src -ldl -Wstack-protector -fstack-protector-all
-pthread -ggdb -lmariadb -lmariadbclient -Wwrite-strings -lssl -lcrypto -lz -fPIC 
```
# 支持的编译器（最低版本）:
    - Linux: G++ 9.2, Clang++ 9.0
    - MacOS: Apple clang version 12.0.0 
    - Windows: MSVC C++ compiler version 1930.

### 归属
    ccORM使用以下库。

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
