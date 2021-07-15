#include "ccORM.hh"
namespace crow {
  auto d = D("127.0.0.1","mysql_test","root","",3306,"utf8");//D_();//easy to connect
}
int main() {
  using namespace crow; using namespace std;
  auto q = d.connect();
  q("DROP table if exists users_test;");
  q("CREATE TABLE users_test (id int,name TEXT,age int);");
  auto add = q.prepare(string("INSERT into users_test(id, name, age) values (?,?,?);"));
  add(1,"Alex",44);
  add(2,"Boom",23);
  add(3,"F",21);
  string s = q("select name from users_test where id = 2").template r__<string>();
  EXPECT_EQUAL((make_tuple("Boom",23)),
    (q("select name, age from users_test where id = 2").template r__<string,int>()));
  d.clear_connections();
  cout << s << endl;
  return 0;
}
