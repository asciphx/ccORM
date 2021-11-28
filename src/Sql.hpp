#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<stdarg.h>
#include"macros.hpp"
#define MAX_LIMIT 100
namespace orm {
  enum class Sort { ASC, DESC };
  template<typename T> class Table;
  template<typename T> struct Sql {
	friend class Table<T>;
	Sql<T>() : sql_("SELECT ") {}
	~Sql<T>() {}
	inline Sql<T>* limit(size_t limit);
	inline Sql<T>* offset(size_t offset);
	inline Sql<T>* orderBy(const std::string& col, const Sort& ord = Sort::ASC);
	inline Sql<T>* select() { sql_ += "* FROM "; sql_ += T::_name; return this; };
	template <typename... K>
	inline Sql<T>* select(K&&...k);

	inline Sql<T>* alias(const char* alias);
	template<unsigned short I>
	inline Sql<T>* where(const text<I>& str);
	inline std::vector<T> GetArr();
	inline T GetOne();
	static inline void setFields(std::string& os, const text<31>& val);
	inline decltype(D)::connection_type Query();
	//-------------------------------------DataMapper-------------------------------------
	static void InsertArr(typename T::ptr_arr& t);
	static void InsertArr(std::vector<T>* t);
  private: size_t limit_{ 10 }, offset_{ 0 }; std::string sql_; bool prepare_{ true };
		 inline void clear() { sql_ = "SELECT "; limit_ = 10; offset_ = 0; prepare_ = true; }
  };
  template<typename T> Sql<T>* Sql<T>::limit(size_t limit) { limit_ = limit; return this; }
  template<typename T> Sql<T>* Sql<T>::offset(size_t offset) { offset_ = offset; return this; }
  template<typename T> Sql<T>* Sql<T>::orderBy(const std::string& col, const Sort& ord) {
	sql_ += " ORDER BY " + col; if (ord == Sort::DESC)sql_ += " DESC"; return this;
	//sql_.push_back(','); sql_ += col; if (ord == Sort::DESC)sql_ += " DESC";
  }
  template<typename T>
  void Sql<T>::setFields(std::string& os, const text<31>& val) { os += val.c_str(); os.push_back(','); };
  template<typename T>
  template <typename... K> Sql<T>* Sql<T>::select(K&&...k) {
	(void)std::initializer_list<int>{(setFields(sql_, std::forward<K>(k)), 0)...};
	sql_.pop_back(); sql_ += " FROM "; sql_ += T::_name; return this;
  };
  template<typename T> Sql<T>* Sql<T>::alias(const char* alias) { sql_.push_back(' '); sql_ += alias; return this; }
  template<typename T>
  template<unsigned short I> Sql<T>* Sql<T>::where(const text<I>& str) { sql_ += " WHERE "; sql_ += str.c_str(); return this; }
  //Naming beginning with an uppercase letter means that the object returned is not "*this"
  template<typename T> std::vector<T> Sql<T>::GetArr()noexcept(false) {
	std::string sql(sql_); sql += " LIMIT " + std::to_string(limit_ > MAX_LIMIT ? MAX_LIMIT : limit_);
	if (offset_ > 0) { sql += " OFFSET " + std::to_string(offset_); } this->clear();// cout << sql << '\n';
	return D.conn()(sql).template findArray<T>();
  }
  template<typename T> T Sql<T>::GetOne()noexcept(false) {
	std::string sql(sql_); this->clear();// cout << sql << '\n';
	return D.conn()(sql).template findOne<T>();
  };
  template<typename T> decltype(D)::connection_type Sql<T>::Query() { prepare_ = true; return D.conn(); }
  template<typename T> void Sql<T>::InsertArr(typename T::ptr_arr& input) {
	char i = 0; std::ostringstream os, ov; ov << "VALUES "; os << "INSERT INTO " << T::_name << " (";
	for (; i < T::_size_; ++i) {
	  if (T::_[i][0] != 'S' && !(T::_tc_[i] & (TC::PRIMARY_KEY | TC::AUTO_INCREMENT))) os << T::$[i] << ',';
	} os.seekp(-1, os.cur);
	for (auto o : *input.get()) {
	  i = -1; ov << '(';
	  ForEachField(dynamic_cast<T*>(&o), [&i, &os, &ov](auto& t) {
		if (!(T::_tc_[++i] & (TC::PRIMARY_KEY | TC::AUTO_INCREMENT))) {
		  const char* def = T::_def_[i];
		  if constexpr (std::is_fundamental<std::remove_reference_t<decltype(t)>>::value) {
			if constexpr (std::is_same<bool, std::remove_reference_t<decltype(t)>>::value) {
			  ov << t << ',';
			} else { if (!*((char*)&t)) { ov << def << ','; } else { ov << t << ','; } }
		  } else if constexpr (std::is_same<tm, std::remove_reference_t<decltype(t)>>::value) {
			if (!*((char*)&t)) ov << '\'' << def << "',"; else ov << '\'' << t << "',";
		  } else if constexpr (std::is_same<std::string, std::remove_reference_t<decltype(t)>>::value) {
			if (!*((char*)&t)) { ov << '\'' << toQuotes(def) << "',"; } else ov << '\'' << toQuotes(t.c_str()) << "',";
		  } else if constexpr (is_text<std::remove_reference_t<decltype(t)>>::value) {
			if (!*((char*)&t)) { ov << '\'' << toQuotes(def) << "',"; } else ov << '\'' << toQuotes(t.c_str()) << "',";
		  } else { return; }
		}
		}); ov.seekp(-1, ov.cur); ov << "),";
	}
	os << ')'; os << ' ' << ov.str(); os.seekp(-1, os.cur); os << ";"; D.conn()(os.str());
  }
  template<typename T> void Sql<T>::InsertArr(std::vector<T>* input) {
	char i = 0; std::ostringstream os, ov; ov << "VALUES "; os << "INSERT INTO " << T::_name << " (";
	for (; i < T::_size_; ++i) {
	  if (T::_[i][0] != 'S' && !(T::_tc_[i] & (TC::PRIMARY_KEY | TC::AUTO_INCREMENT))) os << T::$[i] << ',';
	} os.seekp(-1, os.cur);
	for (auto o : *input) {
	  i = -1; ov << '(';
	  ForEachField(dynamic_cast<T*>(&o), [&i, &os, &ov](auto& t) {
		if (!(T::_tc_[++i] & (TC::PRIMARY_KEY | TC::AUTO_INCREMENT))) {
		  const char* def = T::_def_[i];
		  if constexpr (std::is_fundamental<std::remove_reference_t<decltype(t)>>::value) {
			if constexpr (std::is_same<bool, std::remove_reference_t<decltype(t)>>::value) {
			  ov << t << ',';
			} else { if (!*((char*)&t)) { ov << def << ','; } else { ov << t << ','; } }
		  } else if constexpr (std::is_same<tm, std::remove_reference_t<decltype(t)>>::value) {
			if (!*((char*)&t)) ov << '\'' << def << "',"; else ov << '\'' << t << "',";
		  } else if constexpr (std::is_same<std::string, std::remove_reference_t<decltype(t)>>::value) {
			if (!*((char*)&t)) { ov << '\'' << toQuotes(def) << "',"; } else ov << '\'' << toQuotes(t.c_str()) << "',";
		  } else if constexpr (is_text<std::remove_reference_t<decltype(t)>>::value) {
			if (!*((char*)&t)) { ov << '\'' << toQuotes(def) << "',"; } else ov << '\'' << toQuotes(t.c_str()) << "',";
		  } else { return; }
		}
		}); ov.seekp(-1, ov.cur); ov << "),";
	}
	os << ')'; os << ' ' << ov.str(); os.seekp(-1, os.cur); os << ";"; D.conn()(os.str());
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
