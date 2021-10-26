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
	Sql<T>() : sql_("SELECT ") {}
	~Sql<T>() {}
	Sql<T>* limit(size_t limit);
	Sql<T>* offset(size_t offset);
	Sql<T>* orderBy(const string& col, const Sort& ord = Sort::ASC);
	template <typename... K>
	Sql<T>* select(K T::*&&...sc);
	Sql<T>* alias(const char* alias);
	Sql<T>* where(const string& str);
	vector<T> FindArr();
	T FindOne(const char* where);
	template<typename K>
	static void setFields(string& os, K T::** val);
	decltype(D)::connection_type Query();
	//DataMapper
	static long long Insert(typename T::ptr& t);
	static long long Insert(T* t);
	static void Delete(typename T::ptr& t);
	static void Delete(T* t);
	static void Update(typename T::ptr& t);
	static void Update(T* t);
  private: size_t limit_{ 10 }, offset_{ 0 }; string sql_; bool prepare_{ true };
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
  void Sql<T>::setFields(string& ios, K T::** $$$) {
	constexpr auto schema = Schema<T>(); int8_t i = -1;
	ForEachTuple(schema, [&i, &ios, $$$](auto ___) {
	  ++i; if (typeid($$$) == typeid(&___)) {
		ios += T::$[i]; ios += ',';
	  }}, std::make_index_sequence<std::tuple_size<decltype(schema)>::value>{});
  }
  template<typename T>
  template<typename... K>inline Sql<T>* Sql<T>::select(K T::*&&... __) {
	if (sizeof...(K) == 0) { sql_ += "* FROM "; sql_ += T::_name; return this; }
	(void)initializer_list<int>{(setFields(sql_, std::forward<K T::**>(&__)), 0)...};
	sql_.pop_back(); sql_ += " FROM "; sql_ += T::_name; return this;
  }
  template<typename T>inline Sql<T>* Sql<T>::alias(const char* alias) { sql_.push_back(' '); sql_ += alias; return this; }
  template<typename T>inline Sql<T>* Sql<T>::where(const string& str) { sql_ += " WHERE " + str; return this; }
  //Naming beginning with an uppercase letter means that the object returned is not "*this"
  template<typename T>inline vector<T> Sql<T>::FindArr()noexcept(false) {
	string sql(sql_); sql += " LIMIT " + to_string(limit_ > MAX_LIMIT ? MAX_LIMIT : limit_);
	if (offset_ > 0) { sql += " OFFSET " + to_string(offset_); } this->clear();// cout << sql << '\n';
	return D.conn()(sql).findArray<T>();
  }
  template<typename T>inline T Sql<T>::FindOne(const char* where)noexcept(false) {
	string sql(sql_); sql += " WHERE "; sql += where; this->clear();// cout << sql << '\n';
	return D.conn()(sql).findOne<T>();
  };
  template<typename T> decltype(D)::connection_type Sql<T>::Query() { prepare_ = true; return D.conn(); }
  template<typename T> long long Sql<T>::Insert(typename T::ptr& input) {
	int8_t i = -1; std::ostringstream os, ov; ov << "VALUES ("; os << "INSERT INTO " << T::_name << " (";
	ForEachField(dynamic_cast<T*>(input.get()), [&i, &os, &ov](auto& t) {
	  if (!(T::_tc_[++i] & TC::PRIMARY_KEY & TC::AUTO_INCREMENT)) {
		const char* def = T::_def_[i];
		if constexpr (std::is_fundamental<std::remove_reference_t<decltype(t)>>::value) {
		  if constexpr (std::is_same<bool, std::remove_reference_t<decltype(t)>>::value) {
			ov << t << ',';
		  } else { if (!*((char*)&t)) { ov << def << ','; } else { ov << t << ','; } }
		} else if constexpr (std::is_same<tm, std::remove_reference_t<decltype(t)>>::value) {
		  if (!*((char*)&t)) ov << '\'' << def << "',"; else ov << '\'' << t << "',";
		} else if constexpr (std::is_same<std::string, std::remove_reference_t<decltype(t)>>::value) {
		  if (!*((char*)&t)) ov << '\'' << toQuotes(def) << "',"; else ov << '\'' << toQuotes(t.c_str()) << "',";
		} else { return; }
		os << T::$[i] << ',';
	  }
	  });
	os.seekp(-1, os.cur); os << ')'; ov.seekp(-1, ov.cur); ov << ")"; os << ' ' << ov.str() << ";";
	crow::sql_result<decltype(D)::db_rs>&& rs = D.conn()(os.str());
	if (T::_tc_[0] & TC::PRIMARY_KEY) { return rs.last_insert_id(); } else { return 0LL; }
  }
  //DataMapper
  template<typename T> long long Sql<T>::Insert(T* input) {
	int8_t i = -1; std::ostringstream os, ov; ov << "VALUES ("; os << "INSERT INTO " << T::_name << " (";
	ForEachField(dynamic_cast<T*>(input), [&i, &os, &ov](auto& t) {
	  if (!(T::_tc_[++i] & TC::PRIMARY_KEY & TC::AUTO_INCREMENT)) {
		const char* def = T::_def_[i];
		if constexpr (std::is_fundamental<std::remove_reference_t<decltype(t)>>::value) {
		  if constexpr (std::is_same<bool, std::remove_reference_t<decltype(t)>>::value) {
			ov << t << ',';
		  } else { if (!*((char*)&t)) { ov << def << ','; } else { ov << t << ','; } }
		} else if constexpr (std::is_same<tm, std::remove_reference_t<decltype(t)>>::value) {
		  if (!*((char*)&t)) ov << '\'' << def << "',"; else ov << '\'' << t << "',";
		} else if constexpr (std::is_same<std::string, std::remove_reference_t<decltype(t)>>::value) {
		  if (!*((char*)&t)) ov << '\'' << toQuotes(def) << "',"; else ov << '\'' << toQuotes(t.c_str()) << "',";
		} else { return; }
		os << T::$[i] << ',';
	  }
	  });
	os.seekp(-1, os.cur); os << ')'; ov.seekp(-1, ov.cur); ov << ")"; os << ' ' << ov.str() << ";";
	decltype(D)::connection_type q = D.conn(); crow::sql_result<decltype(D)::db_rs>&& rs = q(os.str());
	if (T::_tc_[0] & TC::PRIMARY_KEY) { return rs.last_insert_id(); } else { return 0LL; }
  }
  template<typename T> void Sql<T>::Delete(typename T::ptr& input) {
	std::ostringstream os; os << "DELETE FROM " << T::_name << " WHERE " << T::$[0] << '=';
	auto& t = dynamic_cast<T*>(input.get())->*std::get<0>(Schema<T>());
	if constexpr (std::is_fundamental<std::remove_reference_t<decltype(t)>>::value) {
	  os << t;
	} else if constexpr (std::is_same<std::string, std::remove_reference_t<decltype(t)>>::value) {
	  os << '\'' << toQuotes(t.c_str()) << '\'';
	} os << ";"; D.conn()(os.str());
  }
  template<typename T> void Sql<T>::Delete(T* input) {
	std::ostringstream os; os << "DELETE FROM " << T::_name << " WHERE " << T::$[0] << '=';
	auto& t = dynamic_cast<T*>(input)->*std::get<0>(Schema<T>());
	if constexpr (std::is_fundamental<std::remove_reference_t<decltype(t)>>::value) {
	  os << t;
	} else if constexpr (std::is_same<std::string, std::remove_reference_t<decltype(t)>>::value) {
	  os << '\'' << toQuotes(t.c_str()) << '\'';
	} os << ";"; D.conn()(os.str());
  }
  template<typename T> void Sql<T>::Update(typename T::ptr& input) {
	int8_t i = -1; std::ostringstream os; os << "UPDATE " << T::_name << " SET ";
	std::string condition(" WHERE "); condition += T::$[0]; condition.push_back('=');
	T* temp = input.get(); auto& t = dynamic_cast<T*>(temp)->*std::get<0>(Schema<T>());
	if constexpr (std::is_fundamental<std::remove_reference_t<decltype(t)>>::value) {
	  condition += std::to_string(t);
	} else if constexpr (std::is_same<std::string, std::remove_reference_t<decltype(t)>>::value) {
	  condition.push_back('\''); condition += toQuotes(t.c_str()); condition.push_back('\'');
	}
	ForEachField(dynamic_cast<T*>(temp), [&i, &os, &condition](auto& t) {
	  if (++i && !(T::_tc_[i] & TC::AUTO_INCREMENT)) {
		if constexpr (std::is_fundamental<std::remove_reference_t<decltype(t)>>::value) {
		  os << T::$[i] << '=' << t << ',';
		} else if constexpr (std::is_same<tm, std::remove_reference_t<decltype(t)>>::value) {
		  os << T::$[i] << '=' << '\'' << t << "',";
		} else if constexpr (std::is_same<std::string, std::remove_reference_t<decltype(t)>>::value) {
		  os << T::$[i] << '=' << '\'' << toQuotes(t.c_str()) << "',";
		} else { return; }
	  }
	  });
	os.seekp(-1, os.cur); os << condition << ";"; D.conn()(os.str());
  }
  template<typename T> void Sql<T>::Update(T* input) {
	int8_t i = -1; std::ostringstream os; os << "UPDATE " << T::_name << " SET ";
	std::string condition(" WHERE "); condition += T::$[0]; condition.push_back('=');
	auto& t = dynamic_cast<T*>(input)->*std::get<0>(Schema<T>());
	if constexpr (std::is_fundamental<std::remove_reference_t<decltype(t)>>::value) {
	  condition += std::to_string(t);
	} else if constexpr (std::is_same<std::string, std::remove_reference_t<decltype(t)>>::value) {
	  condition.push_back('\''); condition += toQuotes(t.c_str()); condition.push_back('\'');
	}
	ForEachField(dynamic_cast<T*>(input), [&i, &os, &condition](auto& t) {
	  if (++i && !(T::_tc_[i] & TC::AUTO_INCREMENT)) {
		if constexpr (std::is_fundamental<std::remove_reference_t<decltype(t)>>::value) {
		  os << T::$[i] << '=' << t << ',';
		} else if constexpr (std::is_same<tm, std::remove_reference_t<decltype(t)>>::value) {
		  os << T::$[i] << '=' << '\'' << t << "',";
		} else if constexpr (std::is_same<std::string, std::remove_reference_t<decltype(t)>>::value) {
		  os << T::$[i] << '=' << '\'' << toQuotes(t.c_str()) << "',";
		} else { return; }
	  }
	  });
	os.seekp(-1, os.cur); os << condition << ";"; D.conn()(os.str());
  }
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
