#include "ccORM.hh"
#include "generic_sql_tests.hh"
auto d = D_mysql();
auto d1 = D_sqlite("test.db");
auto d2 = D_pgsql();
int main() {
  using namespace crow; using namespace std;
  auto q = d.conn();
  q("DROP table if exists users_test;");
  q("CREATE TABLE users_test (id int,name TEXT,age int);");
  auto add = q.query(string("INSERT into users_test(id, name, age) values (?,?,?);"));
  add(1,"Alex",44);
  add(2,"Boom",23);
  add(3,"Flux",21);
  q.query(string("INSERT into users_test(id, name, age) values (?,?,?);"))(4, "Deaod", 32);
  string s = q("select name from users_test where id = 2").template r__<string>();
  EXPECT_EQUAL(make_tuple("Flux", 21), (q("select name, age from users_test where id = 3").template r__<string, int>()));
  cout << s << endl;
  d.flush();
  generic_sql_tests(d);//test mysql
  generic_sql_tests(d1);//test sqlite
  generic_sql_tests(d2);//test pgsql
  return 0;
}
