#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<stdarg.h>
#include"macros.hpp"
#define MAX_LIMIT 100
namespace orm {
  enum class Sort { ASC, DESC }; using namespace std;
  template<typename T> class Table;
  template<typename T> struct Sql {
	friend class Table<T>;
	Sql<T>() : table_(T::_name.c_str()), sql_("SELECT ") {}
	~Sql<T>() {}
	Sql<T>* limit(size_t limit);
	Sql<T>* offset(size_t offset);
	Sql<T>* orderBy(const string& col, const Sort& ord = Sort::ASC);
	template <typename... K>
	Sql<T>* select(K T::*... sc);
	Sql<T>* alias(const char* alias);
	Sql<T>* where(const string& str);
	vector<T> FindArr();
	T FindOne(const char* where);
	template<typename K>
	static void setFields(string& os, K T::* val);
	static decltype(D)::connection_type Query();
  private: uint32_t limit_{ 10 }, offset_{ 0 }; string sql_; const std::string table_; bool prepare_{ true };
		 inline void clear() { sql_ = "SELECT "; limit_ = 10; offset_ = 0; prepare_ = true; }
  };
  template<typename T>inline Sql<T>* Sql<T>::limit(size_t limit) { limit_ = limit; return this; }
  template<typename T>inline Sql<T>* Sql<T>::offset(size_t offset) { offset_ = offset; return this; }
  template<typename T>inline Sql<T>* Sql<T>::orderBy(const string& col, const Sort& ord) {
	sql_ += " ORDER BY " + col; if (ord == Sort::DESC)sql_ += " DESC"; return this;
	//sql_.push_back(','); sql_ += col; if (ord == Sort::DESC)sql_ += " DESC";
  }
  template<typename T>
  template<typename K>
  void Sql<T>::setFields(string& os, K T::* val) {
	constexpr auto schema = Schema<T>(); int8_t i = -1;
	ForEachTuple(schema, [&i, &os, &val](auto field) {
	  ++i; if (typeid(&val) == typeid(&field)) {
		os += T::$[i]; os += ',';
	  }}, std::make_index_sequence<std::tuple_size<decltype(schema)>::value>{});
  }
  template<typename T>
  template<typename... K>inline Sql<T>* Sql<T>::select(K T::*... sc) {
	if (sizeof...(K) == 0) { sql_ += "* FROM "; sql_ += table_; return this; }
	(void)initializer_list<int>{(setFields(sql_, sc), void(), 0)...};
	sql_.pop_back(); sql_ += " FROM "; sql_ += table_; return this;
  }
  template<typename T>inline Sql<T>* Sql<T>::alias(const char* alias) { sql_.push_back(' '); sql_ += alias; return this; }
  template<typename T>inline Sql<T>* Sql<T>::where(const string& str) { sql_ += " WHERE " + str; return this; }
  //Naming beginning with an uppercase letter means that the object returned is not "*this"
  template<typename T>inline vector<T> Sql<T>::FindArr()noexcept(false) {
	string sql(sql_); sql += " LIMIT " + to_string(limit_ > MAX_LIMIT ? MAX_LIMIT : limit_);
	if (offset_ > 0) { sql += " OFFSET " + to_string(offset_); }
	this->clear();// cout << sql << '\n';
	decltype(D)::connection_type q_ = D.conn(); return q_(sql).findArray<T>();
  }
  template<typename T>inline T Sql<T>::FindOne(const char* where)noexcept(false) {
	string sql(sql_); sql += " WHERE "; sql += where; this->clear();// cout << sql << '\n';
	decltype(D)::connection_type q_ = D.conn(); return q_(sql).findOne<T>();
  };
  template<typename T> decltype(D)::connection_type Sql<T>::Query() { return D.conn(); }
}//chrono::milliseconds(100)microseconds
/*
"SELECT u.id, u.account, u.name,u.photo, Role.id AS Role_id, Role.name AS Role_name FROM user u\
 LEFT JOIN user_role u_Role ON u_Role.user_id=u.id LEFT JOIN role Role\
 ON Role.id=u_Role.role_id  ORDER BY u.id"
"SELECT `u`.`id` AS `u_id`, `u`.`account` AS `u_account`, `u`.`name` AS `u_name`,\
 `u`.`photo` AS `u_photo`, `Role`.`id` AS `Role_id`, `Role`.`name` AS `Role_name` FROM `user` `u`\
 LEFT JOIN `user_role` `u_Role` ON `u_Role`.`user_id`=`u`.`id` LEFT JOIN `role` `Role`\
 ON `Role`.`id`=`u_Role`.`role_id`  ORDER BY `u`.`id`"
*/
