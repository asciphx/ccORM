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
  template<typename T, typename U>//Table Linker
  struct TLinker {
	TLinker(TLinker const&) = delete; TLinker& operator=(TLinker&&) = delete;
	TLinker(TLinker&&) = default; TLinker& operator=(TLinker const&) = delete;
	TLinker<T, U>() : sql_("SELECT "), ob_("") {} ~TLinker<T, U>() {}
	inline void useFields(text<0xbff>& os, const text<0x3f>& v_);
	inline TLinker<T, U>* $() { sql_ += T::_ios_; return this; };
	inline void GetArr(const Sort& ord = Sort::ASC) {
	  constexpr auto v = Tuple<T>(); constexpr auto u = Tuple<U>();
	  constexpr auto $ = std::tuple_cat(v, u); std::string s("{");
	  ForEachTuple($, [](auto _) {
		//std::cout << '\n' << typeid(&_).name();
		}, std::make_index_sequence<std::tuple_size<decltype($)>::value>{});
	  T table; int8_t i = -1;
	  ForEachField(&table, [&s, &i](auto& t) {
		s.push_back(34); s += T::$[++i];
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
			<< time->tm_hour << ':' << std::setw(2) << time->tm_min << ':' << std::setw(2) << time->tm_sec << 34; s += os.str();
		} else {
		  s += "\":"; s << t;
		} s.push_back(',');
		}); s[s.size() - 1] = '}';
		std::cout << '\n' << sql_ << " -> " << s;
	};
	template<typename U, typename... K>
	TLinker<T, U>* Join(K&&...k) {
	  static_assert(sizeof...(K) > 0); sql_.push_back(',');
	  (void)std::initializer_list<int>{(useFields(sql_, std::forward<K>(k)), 0)...};
	  sql_.pop_back(); return this;
	};
  private: size_t limit_{ 10 }, offset_{ 0 }; text<0x7ff> sql_; text<0xff> ob_;
  };
  template<class T, class U>
  void TLinker<T, U>::useFields(text<0xbff>& os, const text<0x3f>& v_) {
	const char* c = v_.c_str(); os += c; os += " AS "; os += U::_alias; os.push_back(0x5f); if constexpr (ce_is_pgsql) {
	  while (*++c != 0x22) {}; while (*++c != 0x22)os.push_back(*c);
	} else { while (*++c != 0x60) {}; while (*++c != 0x60)os.push_back(*c); } os.push_back(',');
  };/* SELECT User.id, User.account, User.name, User.photo, Role.id AS Role_id, Role.name AS Role_name
  FROM user User LEFT JOIN user_role UserRole ON UserRole.user_id = User.id LEFT JOIN role Role
  ON Role.id = UserRole.role_id  WHERE User.id>1 ORDER BY User.id */
  template<typename T> struct Sql {
	friend class Table<T>;
	Sql<T>() : sql_("SELECT ") { sql_.reserve(0x1ff); ob_.reserve(0x5f); }
	~Sql<T>() {}
	inline Sql<T>* limit(size_t limit);
	inline Sql<T>* offset(size_t offset);
	inline Sql<T>* orderBy(const text<0x3f>& col, const Sort& ord = Sort::ASC);//default Sort::ASC
	//select <_.`$`>,... from <T> _ WHERE <_.`$`=?> ORDER BY _.`$1` LIMIT 10 OFFSET 0;
	inline Sql<T>* $() { sql_ += T::_ios_; sql_ += " FROM "; sql_ += T::_name; sql_.push_back(' '); sql_ += T::_alias; return this; };
	template <typename... K>
	inline Sql<T>* $(K&&...k);
	template<unsigned short I>
	inline Sql<T>* where(const text<I>& str);
	inline std::vector<T> GetArr(const Sort& ord = Sort::ASC);
	inline T GetOne();
	inline void setFields(std::string& os, const text<0x3f>& v_);//not only but also
	inline void setFields(std::string& os, const char* v_);//v_ only belongs T
	inline decltype(D)::connection_type Query();
	//-------------------------------------DataMapper-------------------------------------
	static void InsertArr(typename T::ptr_arr& t);
	static void InsertArr(std::vector<T>* t);
  private: size_t limit_{ 10 }, offset_{ 0 }; std::string sql_, ob_; bool prepare_{ true };
		 inline void clear() { sql_ = "SELECT "; limit_ = 10; ob_[0] = offset_ = 0; prepare_ = true; }
  };
  template<typename T> Sql<T>* Sql<T>::limit(size_t limit) { limit_ = limit; return this; }
  template<typename T> Sql<T>* Sql<T>::offset(size_t offset) { offset_ = offset; return this; }
  template<typename T> Sql<T>* Sql<T>::orderBy(const text<0x3f>& col, const Sort& ord) {
	ob_ += col.c_str(); ob_ += SORT[static_cast<short>(ord)]; ob_.push_back(','); return this;
  }
  template<typename T>
  void Sql<T>::setFields(std::string& os, const text<0x3f>& v_) { os += v_.c_str(); os.push_back(','); };
  template<typename T>
  void Sql<T>::setFields(std::string& os, const char* v_) {
	os += T::_alias; os.push_back('.'); if constexpr (ce_is_pgsql) {
	  os.push_back(34); os += v_; os.push_back(34);
	} else { os.push_back(96); os += v_; os.push_back(96); } os.push_back(',');
  };
  template<typename T>
  template <typename... K> Sql<T>* Sql<T>::$(K&&...k) {
	(void)std::initializer_list<int>{(setFields(sql_, std::forward<K>(k)), 0)...}; sql_.pop_back();
	sql_ += " FROM "; sql_ += T::_name; sql_.push_back(' '); sql_ += T::_alias; return this;
  };
  template<typename T>
  template<unsigned short I> Sql<T>* Sql<T>::where(const text<I>& v_) { sql_ += " WHERE "; sql_ += v_.c_str(); return this; }
  //Naming beginning with an uppercase letter means that the object returned is not "*this"
  template<typename T> std::vector<T> Sql<T>::GetArr(const Sort& ord)noexcept(false) {
	text<0x3ff> sql(sql_); sql & " ORDER BY "; ob_ += T::_alias; ob_.push_back('.'); if constexpr (ce_is_pgsql) {
	  ob_.push_back(34); ob_ += T::$[0]; ob_.push_back(34);
	} else { ob_.push_back(96); ob_ += T::$[0]; ob_.push_back(96); }
	sql += ob_; sql += SORT[static_cast<short>(ord)]; sql += " LIMIT " + std::to_string(limit_ > MAX_LIMIT ? MAX_LIMIT : limit_);
	sql += " OFFSET " + std::to_string(offset_);// std::cout << sql << '\n';
	this->clear(); return D.conn()(sql.c_str()).template findArray<T>();
  }
  template<typename T> T Sql<T>::GetOne()noexcept(false) {
	std::string sql(sql_); this->clear(); return D.conn()(sql).template findOne<T>();
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
