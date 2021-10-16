#include "src/Sql.hpp"
#include "src/Table.hpp"
using namespace std; using namespace orm;
struct Type : Table<Type> {
  int id;
  string language;
  Type(int a = 0, string b = "") :
	id(a), language(b) {} Type(bool);
}; CONSTRUCT(Type, id, language)
REGIST_PROTO(Type,
  TC::PRIMARY_KEY | TC::AUTO_INCREMENT, "",
  TC::DEFAULT, "c/c++");
struct Tab : Table<Tab> {
  int id;
  bool ok;
  string name;
  tm date;
  vector<Type> lang;
  Tab(int a = 0, bool b = false, string c = "", tm d = now(), vector<Type> e = {}) :
	id(a), ok(b), name(c), date(d), lang(e) {} Tab(bool);
}; CONSTRUCT(Tab, id, ok, name, date, lang)
REGIST_PROTO(Tab,
  TC::PRIMARY_KEY | TC::AUTO_INCREMENT, "",
  TC::DEFAULT, "false",
  TC::DEFAULT, "ww'zzgg",
  TC::DEFAULT | TC::NOT_NULL, "");