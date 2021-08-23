# ccORM[version 0.4]
ccORM is the best database query software.
🚀 Support Mac, Linux, windows, three platforms, the fastest development speed, the quickest and the strongest, the next step is to support ORM.
 ![Benchmark results (not cached)](./test.png)
```c++
#include "ccORM.hh"
auto d = D_mysql();//easy to connect
//auto d = D_sqlite("test.db");
//auto d = D_pgsql();
int main(int argc, char* argv[]) {
  Timer t; int i = 0; t.setInterval([&i]() { cout << "After " << ++i << "s\n"; }, 1000);
  std::locale::global(std::locale(u8"en_US.UTF8"));
  auto q = d.conn();
  if (argv[1]) cout << q(argv[1]).JSON() << endl;// ./main "SELECT * from users_test"

  q("DROP table if exists users_test;");
  q("CREATE TABLE users_test (id int,name TEXT,age int);");
  //q.query(string("INSERT into users_test(id, name, age) values (?,?,?);"))(4, "Deaod", 32);
  auto add = q.prepare(string("INSERT into users_test(id, name, age) values (?,?,?);"));
  add(1,"Alex God",44);
  add(2,"Program King",23);
  add(3,"Invincible Coder",21);
  q.query(string("INSERT into users_test(id, name, age) values (?,?,?);"))(4, "Deaod", 32);
  string s = q("select name from users_test where id = 2").template r__<string>();
  EXPECT_EQUAL((make_tuple("Invincible Coder",21)),
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