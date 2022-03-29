#include "src/Table.hh"
static int RES_INIT = orm::InitializationOrm();
using namespace std; using namespace orm;
class Tab; //It's best to define each structure at the front, so that there is no need to consider the order later
class Type; D_M_TABLE(Type, Tab);//Delete the intermediate table before deleting the original table(Only FastestDev)

Class(Tab)
uint32_t id;
bool ok;
text<15> name;
tm date;
vector<Type> types;
Type* type;
Tab(uint32_t a = 0, bool b = false, const char* c = "", tm d = now(), vector<Type> e = {}, Type* f = nullptr) :
  id(a), ok(b), name(c), date(d), types(e), type(f) {}
~Tab() { type = nullptr; }
FIELD(id, ok, name, date)
CLASS(Tab, id, ok, name, date, types, type)
PROTO(Tab, id, ok, name, date)
REGIST(Tab,
  TC::PRIMARY_KEY | TC::AUTO_INCREMENT, "",
  TC::DEFAULT, "false",
  TC::DEFAULT, "ww'zzgg",
  TC::DEFAULT | TC::NOT_NULL, "");

Class(Type)
uint8_t id;
text<10> language;
double bigBlob;
vector<Tab> tabs;
Tab* tab;
Type(uint8_t a = 0, const char* b = "", double c = 0, vector<Tab> d = {}, Tab* e = nullptr) :
  id(a), language(b), bigBlob(c), tabs(d), tab(e) {}
~Type() { tab = nullptr; }
FIELD(id, language, bigBlob)
CLASS(Type, id, language, bigBlob, tabs, tab)
PROTO(Type, id, language, bigBlob)
REGIST(Type,
  TC::PRIMARY_KEY | TC::AUTO_INCREMENT, "",
  TC::DEFAULT, "c/c++",
  TC::EMPTY, "");
M_TABLE(Type, id, Tab, id)//Create intermediate table