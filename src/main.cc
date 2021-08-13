#include "ccORM.hh"
#include "generic_sql_tests.hh"
auto d = D_mysql();//用宏的方式，防止被人轻易看到用户名和密码
auto d1 = D_sqlite("test.db");
auto d2 = D_pgsql();
int main() {
  using namespace crow; using namespace std;
  auto q = d.conn();
  q("DROP table if exists users_test;");
  q("CREATE TABLE users_test (id int,name TEXT,age int)\
	ENGINE=INNODB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;");
  auto add = q.query(string("INSERT into users_test(id, name, age) values (?,?,?);"));
  add(1,"A神",44);
  add(2,"程序王",23);
  add(3,"超威码哥",21);
  q.query(string("INSERT into users_test(id, name, age) values (?,?,?);"))(4, "迪奥", 32);
  string s = q("select name from users_test where id = 2").template r__<string>();
  EXPECT_EQUAL(make_tuple("超威码哥", 21), (q("select name, age from users_test where id = 3").template r__<string, int>()));
  cout << s << endl;
  d.flush();
  generic_sql_tests(d);//测试 mysql
  generic_sql_tests(d1);//测试 sqlite
  generic_sql_tests(d2);//测试 pgsql
  return 0;
}
