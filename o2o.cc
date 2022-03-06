#include "src/ccORM.hh"
auto D =
//D_mysql();
//D_pgsql();
D_sqlite("any.db");
#include "src/Table.hh"
static int RES_INIT = orm::InitializationOrm();
using namespace std; using namespace orm;
struct Tab; //It's best to define each structure at the front, so that there is no need to consider the order later
struct Type; //D_M_TABLE(Type, Tab)//Can be commented out, but the intermediate table will not be reset in FastestDev
Struct(Tab) {
  uint32_t id;
  bool ok;
  text<15> name;
  tm date = now();
  Type* types = nullptr;
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
  Tab* tabs = nullptr;
  FIELD(id, language, bigBlob)
};
CONSTRUCT(Type, id, language, bigBlob, tabs)
PROTO(Type, id, language, bigBlob)
REGIST(Type,
  TC::PRIMARY_KEY | TC::AUTO_INCREMENT, "",
  TC::DEFAULT, "c/c++",
  TC::EMPTY, "");
M_TABLE(Type, id, Tab, id)
//one-to-one has been completed. In particular, one-to-one is actually a combination of two tables
int main(int argc, char* argv[]) { //For example, if on() has multiple conditions, up to three conditions. eg : two conditions
  //cout << ((Tab::$id == Type::$id) && (Type::$bigBlob == Tab::$name)) << '\n';//(`Tab`.`id`=`Type`.`id` AND `Type`.`bigBlob`=`Tab`.`name`)
  //cout << (Tab::$id == Type::$id.c_str()) << '\n';//`Tab`.`id`='`Type`.`id`', This is used in the where clause
  Type u; Tab t = TLinker<Tab, Type>(Tab::$id == Type::$id).where(Tab::$id == 2).GetOne(&u); cout << t << '\n';//Returns a one query of a one-to-one combined table
  vector<Type> vu; vector<Tab> vt = TLinker<Tab, Type>().GetArr(&vu); cout << vt << '\n';//Returns a paged query of a one-to-one combined table
  return 0;
}
