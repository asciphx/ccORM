#include "src/ccORM.hh"
auto D =
//D_mysql();
//D_pgsql();
D_sqlite("any.db");
#include "src/Table.hh"
static int RES_INIT = orm::InitializationOrm();
using namespace std; using namespace orm;
struct Tab; //It's best to define each structure at the front, so that there is no need to consider the order later
struct Type; //D_M_TABLE(Type, Tab);//Can be commented out, but the intermediate table will not be reset in FastestDev
Struct(Tab) {
  uint32_t id;
  bool ok;
  text<15> name;
  tm date;
  Type* types;
  Tab(uint32_t a = 0, bool b = false, const char* c = "", tm d = now(), Type * e = nullptr) :
	id(a), ok(b), name(c), date(d), types(e) {}
  ~Tab() { types = nullptr; }
  FIELD(id, ok, name, date)
};
CONSTRUCT(Tab, id, ok, name, date, types)
PROTO(Tab, id, ok, name, date)
REGIST(Tab,
  TC::PRIMARY_KEY | TC::AUTO_INCREMENT, "",
  TC::DEFAULT, "false",
  TC::DEFAULT, "ww'zzgg",
  TC::DEFAULT | TC::NOT_NULL, "");

Struct(Type) {
  uint8_t id;
  text<10> language;
  double bigBlob;
  Tab* tabs;
  Type(uint8_t a = 0, const char* b = "", double c = 0, Tab * d = nullptr) :
	id(a), language(b), bigBlob(c), tabs(d) {}
  ~Type() { tabs = nullptr; }
  FIELD(id, language, bigBlob)
};
CONSTRUCT(Type, id, language, bigBlob, tabs)
PROTO(Type, id, language, bigBlob)
REGIST(Type,
  TC::PRIMARY_KEY | TC::AUTO_INCREMENT, "",
  TC::DEFAULT, "c/c++",
  TC::EMPTY, "");
//M_TABLE(Type, id, Tab, id)//Actually,there is no need for an intermediate table in one-to-one
int main(int argc, char* argv[]) { //For example, if on() has multiple conditions, up to three conditions. eg : two conditions
  //cout << ((Tab::$id == Type::$id) && (Type::$bigBlob == Tab::$name)) << '\n';//(`Tab`.`id`=`Type`.`id` AND `Type`.`bigBlob`=`Tab`.`name`)
  Type u; Tab t = TLinker<Tab, Type>(Tab::$id == Type::$id).where(Tab::$id == 2).GetOne(&u); cout << t << '\n' << std::boolalpha;
  vector<Type> vu; vector<Tab> vt = TLinker<Tab, Type>().GetArr(&vu); cout << vt << '\n';
  return 0;
}
