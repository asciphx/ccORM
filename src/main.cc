#include "ccORM.hh"
auto d = crow::D("127.0.0.1", "test", "root", "", 3306, "utf8");//D_();//easy to connect
int main() {
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
  d.close();//if return,don't need this line.
  cout << s << endl;
  return 0;
}
