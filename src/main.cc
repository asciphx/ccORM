#include "json.hpp"
#include "ccORM.hh"
#include "generic_sql_tests.hh"
auto d = D_mysql();//用宏的方式，防止被人轻易看到用户名和密码
auto d1 = D_sqlite("test.db");
auto d2 = D_pgsql();//在web需给content-type加: text/html;charset=UTF-8
using namespace crow; using namespace std; using json = nlohmann::json;
int main() { setlocale(LC_ALL, ".936");//仅控制台，linux可能会乱码
  Timer t; int i = 0; t.setInterval([&i]() { cout << "After " << ++i << "s\n"; }, 1000);
  auto q = d.conn();
  q("DROP table if exists users_test;");
  q("CREATE TABLE if not exists users_test (id int,name TEXT,age int);");
  auto add = q.query(string("INSERT into users_test(id, name, age) values (?,?,?);"));
  add(1,"A神",44);
  add(2,"程序王",23);
  add(3,"超威码哥",21);
  q.query(string("INSERT into users_test(id, name, age) values (?,?,?);"))(4, "迪奥", 32);
  json rs = q("select id,name from users_test where id = 2;").JSON();
  EXPECT_EQUAL(make_tuple("超威码哥", 21), (q("select name, age from users_test where id = 3;").template r__<string, int>()));
  cout << rs <<endl;
  d.flush();
  generic_sql_tests(d);//测试 mysql
  generic_sql_tests(d1);//测试 sqlite
  generic_sql_tests(d2);//测试 pgsql
  return 0;
}
