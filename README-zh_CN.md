# ccORM[版本 0.2-beta]
ccORM是最好的数据库查询软件。
🚀 支持Mac、Linux、windows三种平台。开发速度最快、最迅速、最猛烈，即将支持ORM。
```c++
#include "ccORM.hh"
auto d = D_mysql();//很容易的连接
//auto d1 = D_sqlite("test.db");
//auto d2 = D_pgsql();
int main(){
  using namespace crow; using namespace std;
  auto q = d.conn();
  q("DROP table if exists users_test;");
  q("CREATE TABLE users_test (id int,name TEXT,age int);");
  //q.prepare(string("INSERT into users_test(id, name, age) values (?,?,?);"))(4, "Deaod", 32);
  auto add = q.prepare(string("INSERT into users_test(id, name, age) values (?,?,?);"));
  add(1,"Alex",44);
  add(2,"Boom",23);
  add(3,"Flux",21);
  string s = q("select name from users_test where id = 2").template r__<string>();
  EXPECT_EQUAL((make_tuple("Flux",21)),
    (q("select name, age from users_test where id = 3").template r__<string,int>()));
  d.flush();
  cout << s << endl;
  return 0;
}
```

## 特征
 - 非常快
 - 仅头文件
 - 简易编程

## 前提
cmake需求：[最好使用vcpkg安装mysql]
find_package(MYSQL REQUIRED)
...然后在需要连接库的位置加入
target_link_libraries(main ${MYSQL_LIBRARY})

或者
下面的方式，这只是示例，注意：必须让mariadb，mariadbclient在最前面，才不会报错
```
g++ -std=c++17 *.cc -o main -I./src -ldl -Wstack-protector -fstack-protector-all
-pthread -ggdb -lmariadb -lmariadbclient -Wwrite-strings -lssl -lcrypto -lz -fPIC 
```