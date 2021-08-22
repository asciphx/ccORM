#include "json.hpp"
#define SYS_IS_UTF8 1
#define MaxSyncConnections 64
#include "ccORM.hh"
#include "generic_sql_tests.hh"
auto d = D_mysql();
auto d1 = D_sqlite("test.db");
auto d2 = D_pgsql();
using namespace crow; using namespace std; using json = nlohmann::json;
int main() {
  Timer t; int i = 0; t.setInterval([&i]() { cout << "After " << ++i << "s\n"; }, 1000);
  auto q = d.conn();
  q("DROP table if exists users_test;");
  q("CREATE TABLE if not exists users_test (id int,name TEXT,age int);");
  auto add = q.query(string("INSERT into users_test(id, name, age) values (?,?,?);"));
  add(1, u8"代码哥",44);
  add(2, u8"攻城师",23);
  add(3, u8"程序员",21);
  q.query(string("INSERT into users_test(id, name, age) values (?,?,?);"))(4, u8"编程王", 32);
  json rs = q("select id,name from users_test where id = 2;").JSON();
  EXPECT_EQUAL(make_tuple(u8"程序员", 21), (q("select name, age from users_test where id = 3;").template r__<string, int>()));
  rs["nick"] = u8"天下第一";
  cout << rs <<endl;
  d.flush();
  generic_sql_tests(d);//测试 mysql
  generic_sql_tests(d1);//测试 sqlite
  generic_sql_tests(d2);//测试 pgsql
  return 0;
}
