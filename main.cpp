#include "src/ccORM.hh"
auto D =
D_mysql();
//D_pgsql();
//D_sqlite("any.db");
#include "module.hh"
//many-to-many test. It's going on, but I need more stars and honors. The following contents are all machine translation
void test() {
  Tab::ptr t = Tab::create(1, true, "日期更变", now(), vector<Type>{ Type{ 1,"typescript" } });
  t->Update();//update
  t->set(5, false, "更变", now(), vector<Type>{ Type{ 1,"python" }, Type{ 2,"ruby" } }); cout << t << '\n';
  *t = json::parse(R"({"id":4,"ok":false,"name":"完美杰作","date":"2021-09-08 01:04:30","types":
[{"id":1,"language":"c++"},{"id":2,"tabs":[{"id":1,"name":"wtf!","ok":1}]},{"id":3,"language":"rust"}]})").get<Tab>();
  t->types[1].language = "go programing"; t->types[1].tabs[0].name = "Megatron"; cout << t << '\n';
  //cout << "The number with ID " << t->Insert() << " has been inserted\n";//After insertion, a new ID will be returned
  //t->Delete();//delete
  //Type ty{1, "wwzzgg", 3.1415926}; cout << "The number with ID " << ty.Insert() << " has been inserted\n";
  cout << Tab::Q().orderBy(Tab::$date).GetArr(Sort::DESC) << '\n';
  cout << TLinker<Tab, Type>().GetJson();//Test combination, joint table debugging
}
int main(int argc, char* argv[]) {
  clock_t start = clock(); test(); if constexpr (FastestDev) { return 0; }
  Timer t; bool run = true;//Marks the running state of the second thread
  t.setTimeout([&run] {
	int i = 0; for (; i < 9999; ++i) {
	  Tab::Q().where(Tab::$id == 2).GetOne();
	} printf("<%d>", i);
	run = false;//Represents the end of the secondary thread
	}, 6);
  int i = 0; for (; i < 9998; ++i) {
	Tab::Q().where(Tab::$id == 1).GetOne();
  }//Multithreading test, here is the first thread, that is, the main thread
  printf("<%d>", i);
  while (run) { this_thread::yield(); }//If run is true, the program is prevented from ending prematurely
  printf("\nuse %.6f seconds", (float)(clock() - start) / CLOCKS_PER_SEC);//Calculate the total time spent
  return 0;
}