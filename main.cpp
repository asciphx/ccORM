#include "src/ccORM.hh"
auto D =
//D_mysql();
//D_pgsql();
D_sqlite("any.db");
#include "module.hpp"
void test() {
  Tab::ptr t = Tab::create(1, true, u8"日期更变", now(), vector<Type>{ Type{ 1,"typescript" } });
  t->Update();//更新
  t->set(5, false, "yield", now(), vector<Type>{ Type{ 1,"python" }, Type{ 2,"ruby" } }); cout << t << '\n';
  *t = json::parse(u8R"({"id":4,"ok":false,"name":"完美杰作","date":"2021-09-08 01:04:30",
"lang":[{"id":1,"language":"c++"},{"id":2,"language":"js"},{"id":3,"language":"rust"}]})").get<Tab>();
  t->lang[1].language = "golang"; cout << t << '\n';
  t->Insert();//插入
  cout << Tab::Q()->select()->FindArr();
  t->Delete();//删除
  *t = Tab::Q()->select()->field(&Tab::id, &Tab::name)->FindOne("id = 1"); cout << t << '\n';
}
int main() {
  InitializationOrm<Type, Tab>(); clock_t start = clock(); test();
  Timer t; bool run = true;
  t.setTimeout([&t, &run] {
	int i = 0; for (; i < 5999; ++i) {
	Tab::Q()->select()->FindOne("id = 2"); } printf("<%d>", i);
	t.stop(); run = false;
	}, 6);
  int i = 0; for (; i < 4999; ++i) {
	Tab::Q()->select()->field(&Tab::id, &Tab::name, &Tab::date, &Tab::ok)->FindOne("id = 1");
  }//多线程测试
  printf("<%d>", i);
  while (run) { this_thread::yield(); }
  printf("\nuse %.6f seconds", (float)(clock() - start) / CLOCKS_PER_SEC);
  std::this_thread::sleep_for(std::chrono::milliseconds(500));//wait for something
  return 0;
}