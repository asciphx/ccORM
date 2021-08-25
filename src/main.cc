#include "json.hpp"
#include "ccORM.hh"
#include "generic_sql_tests.hh"
auto d = D_mysql(); auto d1 = D_sqlite("test.db"); auto d2 = D_pgsql();
using namespace crow; using namespace std; using json = nlohmann::json;
int main(int argc, char* argv[]) {
  std::locale::global(std::locale(u8"en_US.UTF8"));//设置系统为utf8编码
  json js = json::parse(u8"{ \"state\": true, \"pi\": 3.1415926, \"name\": \"中文测试\" }");
  std::cout<<js<<std::endl;
  Timer t; int i = 0; t.setInterval([&i]() { cout << "After " << ++i << "s\n"; }, 1000);
  auto q = d.conn();
  const char* cc= "SELECT `u`.`id` AS `u_id`, `u`.`account` AS `u_account`, `u`.`name` AS `u_name`,\
 `u`.`photo` AS `u_photo`, `Role`.`id` AS `Role_id`, `Role`.`name` AS `Role_name` FROM `user` `u`\
 LEFT JOIN `user_role` `u_Role` ON `u_Role`.`user_id`=`u`.`id` LEFT JOIN `role` `Role`\
 ON `Role`.`id`=`u_Role`.`role_id`  ORDER BY `u`.`id`";//反冒号无法从控制台正确录入，但可以通过后台服务器
  if (argv[1]) cout << q(argv[1]).JSON() << endl; else  cout << q(cc).JSON() << endl;
  
  q("DROP table if exists users_test;");
  q("CREATE TABLE if not exists users_test (id int,name TEXT,age int);");
  auto add = q.query(string("INSERT into users_test(id, name, age) values (?,?,?);"));
  add(1, u8"代码哥", 44);
  add(2, u8"攻城师", 23);
  add(3, u8"程序员", 21); //使用方法： ./main "SELECT * from users_test"
  q.query(string("INSERT into users_test(id, name, age) values (?,?,?);"))(4, u8"编程王", 32);
  EXPECT_EQUAL(make_tuple(u8"程序员", 21), (q("select name, age from users_test where id = 3;").template r__<string, int>()));
  generic_sql_tests(d);//测试 mysql
  generic_sql_tests(d1);//测试 sqlite
  generic_sql_tests(d2);//测试 pgsql
  return 0;
}
