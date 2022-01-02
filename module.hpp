#include "src/Sql.hpp"
#include "src/Table.hpp"
using namespace std; using namespace orm;
Struct(Type) {
  uint8_t id;
  text<10> language;
  double bigBlob;
  Type(uint8_t a = 0, const char* b = "", double c = 0) :
	id(a), language(b), bigBlob(c) {}
  FIELD(id, language, bigBlob)
};
CONSTRUCT(Type, id, language, bigBlob)
PROTO(Type, id, language, bigBlob)
REGIST(Type,
  TC::PRIMARY_KEY | TC::AUTO_INCREMENT, "",
  TC::DEFAULT, "c/c++",
  TC::EMPTY, "");
Struct(Tab) {
  uint32_t id;
  bool ok;
  text<15> name;
  tm date;
  vector<Type> lang;
  Tab(uint32_t a = 0, bool b = false, const char* c = "", tm d = now(), vector<Type> e = {}) :
	id(a), ok(b), name(c), date(d), lang(e) {}
  FIELD(id, ok, name, date)
};
CONSTRUCT(Tab, id, ok, name, date, lang)
PROTO(Tab, id, ok, name, date)
REGIST(Tab,
  TC::PRIMARY_KEY | TC::AUTO_INCREMENT, "",
  TC::DEFAULT, "false",
  TC::DEFAULT, "ww'zzgg",
  TC::DEFAULT | TC::NOT_NULL, "");
