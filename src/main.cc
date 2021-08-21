#include "json.hpp"
#include "ccORM.hh"
#include "generic_sql_tests.hh"
auto d = D_mysql();//�ú�ķ�ʽ����ֹ�������׿����û���������
auto d1 = D_sqlite("test.db");
auto d2 = D_pgsql();//��web���content-type��: text/html;charset=UTF-8
using namespace crow; using namespace std; using json = nlohmann::json;
int main() { setlocale(LC_ALL, ".936");//������̨��linux���ܻ�����
  Timer t; int i = 0; t.setInterval([&i]() { cout << "After " << ++i << "s\n"; }, 1000);
  auto q = d.conn();
  q("DROP table if exists users_test;");
  q("CREATE TABLE if not exists users_test (id int,name TEXT,age int);");
  auto add = q.query(string("INSERT into users_test(id, name, age) values (?,?,?);"));
  add(1,"A��",44);
  add(2,"������",23);
  add(3,"�������",21);
  q.query(string("INSERT into users_test(id, name, age) values (?,?,?);"))(4, "�ϰ�", 32);
  json rs = q("select id,name from users_test where id = 2;").JSON();
  EXPECT_EQUAL(make_tuple("�������", 21), (q("select name, age from users_test where id = 3;").template r__<string, int>()));
  cout << rs <<endl;
  d.flush();
  generic_sql_tests(d);//���� mysql
  generic_sql_tests(d1);//���� sqlite
  generic_sql_tests(d2);//���� pgsql
  return 0;
}
