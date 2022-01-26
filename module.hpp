#include "src/Table.hpp"
static int RES_INIT = orm::InitializationOrm();
using namespace std; using namespace orm;
struct Type;
Struct(Tab) {
  uint32_t id;
  bool ok;
  text<15> name;
  tm date;
  vector<Type> types;
  Tab(uint32_t a = 0, bool b = false, const char* c = "", tm d = now(), vector<Type> e = {}) :
	id(a), ok(b), name(c), date(d), types(e) {}
  FIELD(id, ok, name, date)
};
CONSTRUCT(Tab, id, ok, name, date, types)
const char* Tab_Json = R"([{"id":1,"name":"FuckJson","date":"2021-12-23 13:34:43"},
{"id":2,"name":"EasyMock","date":"2022-01-01 23:14:41"},{"id":3,"name":"LazyTest"}])";//Automated build testing
PROTO(Tab, id, ok, name, date)
REGIST(Tab,
  TC::PRIMARY_KEY | TC::AUTO_INCREMENT, "",
  TC::DEFAULT, "false",
  TC::DEFAULT, "ww'zzgg",
  TC::DEFAULT | TC::NOT_NULL, "");
//The code above represents the write database test data for rapid development. Named by `<Table>_Json`
Struct(Type) {
  uint8_t id;
  text<10> language;
  double bigBlob;
  vector<Tab> tabs;
  Type(uint8_t a = 0, const char* b = "", double c = 0, vector<Tab> d = {}) :
	id(a), language(b), bigBlob(c), tabs(d) {}
  FIELD(id, language, bigBlob)
};
CONSTRUCT(Type, id, language, bigBlob, tabs)
const char* Type_Json = R"([{"id":1,"language":"c++"},{"id":2,"language":"js"},
{"id":3,"language":"rust"}])";//or FASTEST_DEV(Type,R"([])"), need FastestDev = true.
PROTO(Type, id, language, bigBlob)
REGIST(Type,
  TC::PRIMARY_KEY | TC::AUTO_INCREMENT, "",
  TC::DEFAULT, "c/c++",
  TC::EMPTY, "");
M_TABLE(Type, id, Tab, id)