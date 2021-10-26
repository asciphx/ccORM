#include "src/ccORM.hh"
auto D =
//D_mysql();
//D_pgsql();
D_sqlite("any.db");
#include "module.hpp"
void test() {
  Tab::ptr t = Tab::create(1, true, u8"日期更变", now(), vector<Type>{ Type{ 1,"typescript" } });
  //t->Update();//更新,ActiveRecord
  Tab::Q()->Update(t);//DataMapper
  t->set(5, false, "yield", now(), vector<Type>{ Type{ 1,"python" }, Type{ 2,"ruby" } }); cout << t << '\n';
  *t = json::parse(u8R"({"id":4,"ok":false,"name":"完美杰作","date":"2021-09-08 01:04:30",
"lang":[{"id":1,"language":"c++"},{"id":2,"language":"js"},{"id":3,"language":"rust"}]})").get<Tab>();
  t->lang[1].language = "golang"; cout << t << '\n';
  cout << u8"编号为" << t->Insert() << u8"的id已经插入\n";//插入，将会有一个新增的id返回
  cout << Tab::Q()->select()->FindArr();
  t->Delete();//删除
  *t = Tab::Q()->select(&Tab::id, &Tab::name)->FindOne("id = 1"); cout << t << '\n';
}
int main() {
  InitializationOrm<Type, Tab>();//初始化建表语句和创建索引
  clock_t start = clock(); test();
  Timer t; bool run = true;//标记第二个线程的运行状态
  t.setTimeout([&run] {
	int i = 0; for (; i < 5999; ++i) {
	  Tab::Q()->select()->FindOne("id = 2");
	} printf("<%d>", i);
	run = false;//代表副线程结束
	}, 6);
  int i = 0; for (; i < 4999; ++i) {
	Tab::Q()->select(&Tab::id, &Tab::name, &Tab::date, &Tab::ok)->FindOne("id = 1");
  }//多线程测试，这里是第一个线程也就是主线程
  printf("<%d>", i);
  while (run) { this_thread::yield(); }//run为true则阻止程序提前结束
  printf("\nuse %.6f seconds", (float)(clock() - start) / CLOCKS_PER_SEC);//计算一共所花时间
  std::this_thread::sleep_for(std::chrono::milliseconds(500));//需要等待，可能因主线程未执行完
  return 0;
}