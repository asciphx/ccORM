#include "ccORM.hh"
#include "generic_sql_tests.hh"
auto d = D_mysql();//�ú�ķ�ʽ����ֹ�������׿����û���������
auto d1 = D_sqlite("test.db");
auto d2 = D_pgsql();
using namespace crow; using namespace std;
int main() {
  Timer t; int i = 0; t.setInterval([&i]() { cout << "After " << ++i << "s\n"; }, 1000);
  auto q = d.conn();
  q("DROP table if exists users_test;");
  q("CREATE TABLE users_test (id int,name TEXT,age int)");
  auto add = q.query(string("INSERT into users_test(id, name, age) values (?,?,?);"));
  add(1,"A��",44);
  add(2,"������",23);
  add(3,"�������",21);
  q.query(string("INSERT into users_test(id, name, age) values (?,?,?);"))(4, "�ϰ�", 32);
  tuple<int, string> rs = q("select id,name from Tab where id = 2").template r__<int, string>();
  EXPECT_EQUAL(make_tuple("�������", 21), (q("select name, age from users_test where id = 3").template r__<string, int>()));
  cout << rs << endl;
  d.flush();
  generic_sql_tests(d);//���� mysql
  generic_sql_tests(d1);//���� sqlite
  generic_sql_tests(d2);//���� pgsql
  return 0;
}
