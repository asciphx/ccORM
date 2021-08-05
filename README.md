# ccORM[version 0.1-alpha]
ccORM is the best database query software.
🚀 Support Mac, Linux, windows, three platforms, the fastest development speed, the quickest and the strongest, the next step is to support ORM.
```c++
#include "ccORM.hh"
auto d = D_mysql();//easy to connect
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

## Features
 - Very Fast
 - Header only
 - Simple code

##Premise
Cmake requirements: [it is best to install MySQL with vcpkg]
find_ package(MYSQL REQUIRED)
...and then add it where you need to connect to the library
target_link_libraries(main ${MYSQL_LIBRARY})
perhaps
This is just an example. Note: you must make MariaDB, mariadbclient in the front to avoid error
```
g++ -std=c++17 *.cc -o main -I./src -ldl -Wstack-protector -fstack-protector-all
-pthread -ggdb -lmariadb -lmariadbclient -Wwrite-strings -lssl -lcrypto -lz -fPIC 
```