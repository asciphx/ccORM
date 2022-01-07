#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<stdarg.h>
#include"macros.hpp"
#include"ccORM.hh"
#define MAX_LIMIT 100
namespace orm {
  enum class Sort { ASC, DESC }; static const char* SORT[2] = { "", " DESC" };
  constexpr bool ce_is_pgsql = std::is_same<decltype(D)::db_tag, crow::pgsql_tag>::value;
  constexpr bool ce_is_mysql = std::is_same<decltype(D)::db_tag, crow::mysql_tag>::value;
  constexpr bool ce_is_sqlite = std::is_same<decltype(D)::db_tag, crow::sqlite_tag>::value;
  template<typename T> class Table;//eg: T::Q()->$(T::$id, T::$name, T::$date)->where(T::$id == 1)->GetOne();
  template<typename T> class Sql;
  template<typename T, typename U>//test
  struct TLinkU {
	TLinkU<T, U>(std::string& sql, std::string& ob) : sql_(sql), ob_(ob) {}
	~TLinkU<T, U>() {}
	inline void GetArr(const Sort& ord = Sort::ASC) {
	  constexpr auto v = Schema<T>(); constexpr auto u = Schema<U>();
	  constexpr auto $ = std::tuple_cat(v, u); std::string s("{");
	  ForEachTuple($, [](auto _) {
		std::cout << '\n' << typeid(&_).name();
		}, std::make_index_sequence<std::tuple_size<decltype($)>::value>{});
	  T table; int8_t i = 0;
	  ForEachField(&table, [&s, &i](auto& t) {
		s.push_back('"'); s.push_back('$'); s += std::to_string(++i);
		if constexpr (std::is_same<bool, std::remove_reference_t<decltype(t)>>::value) {
		  s += "\":", s += t == true ? "true" : "false";
		} else if constexpr (std::is_fundamental<std::remove_reference_t<decltype(t)>>::value) {
		  s += "\":" + std::to_string(t);
		} else if constexpr (std::is_same<std::string, std::remove_reference_t<decltype(t)>>::value) {
		  s += "\":\"" + t + "\"";
		} else if constexpr (crow::is_vector<std::remove_reference_t<decltype(t)>>::value) {
		  s += "\":"; s << t;
		} else if constexpr (std::is_same<tm, std::remove_reference_t<decltype(t)>>::value) {
		  s += "\":\""; std::ostringstream os; const tm* time = &t; os << std::setfill('0');
#ifdef _WIN32
		  os << std::setw(4) << time->tm_year + 1900;
#else
		  int y = time->tm_year / 100; os << std::setw(2) << 19 + y << std::setw(2) << time->tm_year - y * 100;
#endif
		  os << '-' << std::setw(2) << (time->tm_mon + 1) << '-' << std::setw(2) << time->tm_mday << ' ' << std::setw(2)
			<< time->tm_hour << ':' << std::setw(2) << time->tm_min << ':' << std::setw(2) << time->tm_sec << '"'; s += os.str();
		} else {
		  s += "\":"; s << t;
		} s.push_back(',');
		}); s[s.size() - 1] = '}';
		std::cout << '\n' << s << '\n' << sql_;
	};
  private: size_t limit_{ 10 }, offset_{ 0 }; std::string sql_, ob_;
  };
  template<typename T> struct Sql {
	friend class Table<T>;
	Sql<T>() : sql_("SELECT ") { sql_.reserve(0xff); ob_.reserve(0x3f); wh_.reserve(0x7f); }
	~Sql<T>() {}
	inline Sql<T>* limit(size_t limit);
	inline Sql<T>* offset(size_t offset);
	inline Sql<T>* orderBy(const text<0x3f>& col, const Sort& ord = Sort::ASC);//default Sort::ASC
	//select <_.`$`>,... from <T> _ WHERE <_.`$`=?> ORDER BY _.`$1` LIMIT 10 OFFSET 0;
	inline Sql<T>* $() { sql_ += T::_ios_; return this; };
	template <typename... K>
	inline Sql<T>* $(K&&...k);
	template<unsigned short I>
	inline Sql<T>* where(const text<I>& str);
	template<typename U>
	inline TLinkU<T, U> Join() { /*warning: not choose field*/return TLinkU<T, U>(sql_, ob_); };
	template<typename U, typename... K>
	inline TLinkU<T, U> Join(K&&...k) {
	  sql_.push_back(','); (void)std::initializer_list<int>{(useFields<U>(sql_, std::forward<K>(k)), 0)...};
	  sql_.pop_back(); return TLinkU<T, U>(sql_, ob_);
	};
	inline std::vector<T> GetArr(const Sort& ord = Sort::ASC);
	inline T GetOne();
	template<typename U>
	inline void useFields(std::string& os, const text<0x3f>& v_);//
	inline void setFields(std::string& os, const text<0x3f>& v_);//not only but also
	inline void setFields(std::string& os, const char* v_);//v_ only belongs T
	inline decltype(D)::connection_type Query();
	//-------------------------------------DataMapper-------------------------------------
	static void InsertArr(typename T::ptr_arr& t);
	static void InsertArr(std::vector<T>* t);
  private: size_t limit_{ 10 }, offset_{ 0 }; std::string sql_, ob_, wh_; bool prepare_{ true };
		 inline void clear() { sql_ = "SELECT "; limit_ = 10; ob_[0] = wh_[0] = offset_ = 0; prepare_ = true; }
  };
  template<typename T> Sql<T>* Sql<T>::limit(size_t limit) { limit_ = limit; return this; }
  template<typename T> Sql<T>* Sql<T>::offset(size_t offset) { offset_ = offset; return this; }
  template<typename T> Sql<T>* Sql<T>::orderBy(const text<0x3f>& col, const Sort& ord) {
	ob_ += col.c_str(); ob_ += SORT[static_cast<short>(ord)]; ob_.push_back(','); return this;
  }
  template<typename T>
  template<typename U>
  void Sql<T>::useFields(std::string& os, const text<0x3f>& v_) {
	os += v_.c_str(); os += " AS "; os += U::_alias; os.push_back(0x5f); const char* c = v_.c_str();
	while (*++c) { if (*c == 0x60)break; }; while (*++c != 0x60)os.push_back(*c); os.push_back(',');
  };
  template<typename T>
  void Sql<T>::setFields(std::string& os, const text<0x3f>& v_) { os += v_.c_str(); os.push_back(','); };
  template<typename T>
  void Sql<T>::setFields(std::string& os, const char* v_) {
	os += T::_alias; os.push_back('.'); if constexpr (ce_is_pgsql) {
	  os.push_back('"'); os += v_; os.push_back('"');
	} else { os.push_back('`'); os += v_; os.push_back('`'); } os.push_back(',');
  };
  template<typename T>
  template <typename... K> Sql<T>* Sql<T>::$(K&&...k) {
	(void)std::initializer_list<int>{(setFields(sql_, std::forward<K>(k)), 0)...}; sql_.pop_back(); return this;
  };
  template<typename T>
  template<unsigned short I> Sql<T>* Sql<T>::where(const text<I>& v_) { wh_ = " WHERE "; wh_ += v_.c_str(); return this; }
  //Naming beginning with an uppercase letter means that the object returned is not "*this"
  template<typename T> std::vector<T> Sql<T>::GetArr(const Sort& ord)noexcept(false) {
	std::string sql(sql_); sql += " FROM "; sql += T::_name; sql.push_back(' '); sql += T::_alias; sql += wh_; sql += " ORDER BY ";
	ob_ += T::_alias; ob_.push_back('.'); if constexpr (ce_is_pgsql) {
	  ob_.push_back('"'); ob_ += T::$[0]; ob_.push_back('"');
	} else { ob_.push_back('`'); ob_ += T::$[0]; ob_.push_back('`'); }
	sql += ob_; sql += SORT[static_cast<short>(ord)]; sql += " LIMIT " + std::to_string(limit_ > MAX_LIMIT ? MAX_LIMIT : limit_);
	sql += " OFFSET " + std::to_string(offset_);// std::cout << sql << '\n';
	this->clear(); return D.conn()(sql).template findArray<T>();
  }
  template<typename T> T Sql<T>::GetOne()noexcept(false) {
	std::string sql(sql_); sql += " FROM "; sql += T::_name; sql.push_back(' '); sql += T::_alias; sql += wh_;// std::cout << sql << '\n';
	this->clear(); return D.conn()(sql).template findOne<T>();
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
		  if constexpr (std::is_same<bool, std::remove_reference_t<decltype(t)>>::value) {
			if constexpr (ce_is_pgsql) { ov << (t ? "true" : "false") << ','; } else { ov << t << ','; }
		  } if constexpr (std::is_fundamental<std::remove_reference_t<decltype(t)>>::value) {
			if (!*((char*)&t)) { ov << def << ','; } else { ov << t << ','; }
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
		  if constexpr (std::is_same<bool, std::remove_reference_t<decltype(t)>>::value) {
			if constexpr (ce_is_pgsql) { ov << (t ? "true" : "false") << ','; } else { ov << t << ','; }
		  } else if constexpr (std::is_fundamental<std::remove_reference_t<decltype(t)>>::value) {
			if (!*((char*)&t)) { ov << def << ','; } else { ov << t << ','; }
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
SELECT User.id, User.account, User.name, User.photo, Role.id AS Role_id, Role.name AS Role_name FROM user User
LEFT JOIN user_role UserRole ON UserRole.user_id = User.id LEFT JOIN role Role
ON Role.id = UserRole.role_id  WHERE User.id>1 ORDER BY User.id
*/
