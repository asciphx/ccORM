#include "src/ccORM.hh"
auto D =
//D_mysql();
//D_pgsql();
D_sqlite("any.db");
#include "module.hpp"
void test() {
  Tab::ptr t = Tab::create(1, true, "日期更变", now(), vector<Type>{ Type{ 1,"typescript" } });
  t->Update();//更新
  t->set(5, false, "更变", now(), vector<Type>{ Type{ 1,"python" }, Type{ 2,"ruby" } }); cout << t << '\n';
  *t = json::parse(R"({"id":4,"ok":false,"name":"完美杰作","date":"2021-09-08 01:04:30",
"lang":[{"id":1,"language":"c++"},{"id":2,"language":"js"},{"id":3,"language":"rust"}]})").get<Tab>();
  t->lang[1].language = "go programing"; cout << t << '\n';
  //cout << "编号为" << t->Insert() << "的id已经插入\n";//插入，将会有一个新增的id返回
  //cout << Tab::Q()->$()->GetArr();
  //t->Delete();//删除
  //Type ty{1, "wwzzgg", 3.1415926}; cout << "编号为" << ty.Insert() << "的id已经插入\n";
  cout << Type::Q()->$()->orderBy(Type::$bigBlob)->GetArr(Sort::DESC);
  TLinker<Tab,Type>().$()->Join(Type::$bigBlob)->GetArr();//测试联表构建语句
}
int main() {
  InitializationOrm<Type, Tab>();//初始化建表语句和创建索引
  clock_t start = clock(); test();
  Timer t; bool run = true;//标记第二个线程的运行状态
  t.setTimeout([&run] {
	int i = 0; for (; i < 9999; ++i) {
	  Tab::Q()->$()->where(Tab::$id == 2)->GetOne();
	} printf("<%d>", i);
	run = false;//代表副线程结束
	}, 6);
  int i = 0; for (; i < 9998; ++i) {
	Tab::Q()->$()->where(Tab::$id == 1)->GetOne();
  }//多线程测试，这里是第一个线程也就是主线程
  printf("<%d>", i);
  while (run) { this_thread::yield(); }//run为true则阻止程序提前结束
  printf("\nuse %.6f seconds", (float)(clock() - start) / CLOCKS_PER_SEC);//计算一共所花时间
  return 0;
}