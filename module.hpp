#include "src/Sql.hpp"
#include "src/Table.hpp"
using namespace std; using namespace orm;
struct Type : Table<Type> {
  int id;
  text<10> language;
  Type(int a = 0, const char* b = "") :
	id(a), language(b) {}
  FIELD(Type, id, language)
}; CONSTRUCT(Type, id, language)
REGIST_PROTO(Type,
  TC::PRIMARY_KEY | TC::AUTO_INCREMENT, "",
  TC::DEFAULT, "c/c++");
struct Tab : Table<Tab> {
  int id;
  bool ok;
  text<15> name;
  tm date;
  vector<Type> lang;
  Tab(int a = 0, bool b = false, const char* c = "", tm d = now(), vector<Type> e = {}) :
	id(a), ok(b), name(c), date(d), lang(e) {}
  FIELD(Tab, id, ok, name, date, lang)
}; CONSTRUCT(Tab, id, ok, name, date, lang)
REGIST_PROTO(Tab,
  TC::PRIMARY_KEY | TC::AUTO_INCREMENT, "",
  TC::DEFAULT, "false",
  TC::DEFAULT, "ww'zzgg",
  TC::DEFAULT | TC::NOT_NULL, "");