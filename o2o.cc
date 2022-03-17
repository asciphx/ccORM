#include "src/ccORM.hh"
auto D =
//D_mysql();
//D_pgsql();
D_sqlite("any.db");
#include "module.hh"
int main(int argc, char* argv[]) { //For example, if on() has multiple conditions, up to three conditions. eg : two conditions
  //cout << ((Tab::$id == Type::$id) && (Type::$bigBlob == Tab::$name)) << '\n';//(`Tab`.`id`=`Type`.`id` AND `Type`.`bigBlob`=`Tab`.`name`)
  Type u; Tab t = Tab::Q().innerJoin<Type>().where(Tab::$id == 2).GetOne(&u); cout << t << '\n';//default frist key = an other frist key
  //Tab t = Tab::Q().leftJoin<Type>().GetOne();
  vector<Type> vu; vector<Tab> vt = Tab::Q().innerJoin<Type>(Tab::$id == Type::$id).GetArr(&vu); cout << vt << '\n';
  text<> c = Query<Tab>(Tab::$id).size(3).Get<text<>>(); cout << " IN" << c; // IN(1,2,3) Prepare splice clause
  return 0;
}
