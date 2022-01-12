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
  template<typename T, typename U>//Table Linker. Perfect programming! Follow the principle of minimum code.
  struct TLinker {
	TLinker(TLinker const&) = delete; TLinker& operator=(TLinker&&) = delete;
	TLinker(TLinker&&) = default; TLinker& operator=(TLinker const&) = delete;
	TLinker<T, U>() : _("SELECT ") {} ~TLinker<T, U>() {}
	//[select all] T.`<$>`,...
	inline TLinker<T, U>* $() { _ += T::_ios; return this; };
	json Get() {
	  constexpr auto v = Tuple<T>(); constexpr auto u = Tuple<U>();
	  constexpr auto $ = std::tuple_cat(v, u); std::string s("{"); s.reserve(0xff);
	  ForEachTuple($, [](auto _) {
		//std::cout << '\n' << typeid(&_).name();
		}, std::make_index_sequence<std::tuple_size<decltype($)>::value>{});
	  //std::cout << _ << '\n';
	  return D.conn()(_.c_str()).JSON(size_, page_);
	};
	//[select all] U.`<$>` AS U_<$>,... FROM <T> T
	template<typename... K> inline TLinker<T, U>* f() {
	  fields(_, U::$[1]); for (uint8_t i = 2; i < U::_size; ++i) { fields(_, U::$[i]); }
	  _ & " FROM "; _ += T::_name; _& T::_alias; return this;
	};
	//[select K] U.`<$>` -> U.`<$>` AS U_<$> FROM <T> T
	template<typename... K> inline TLinker<T, U>* f(K&&...k) {
	  static_assert(sizeof...(K) > 0); _.push_back(','); Exp{ (fields(_, std::forward<K>(k)), 0)... }; _.pop_back();
	  _ & " FROM "; _ += T::_name; _& T::_alias; return this;
	};
	inline TLinker<T, U>* on(const text<0x3f>& t, const text<0x3f>& u) {
	  tId_ = const_cast<text<0x3f>*>(&t); uId_ = const_cast<text<0x3f>*>(&u); return this;
	};
	inline TLinker<T, U>* inner() {
	  _ & " INNER JOIN "; _ += U::_name; _& U::_alias; _ & " ON "; _ += *tId_; _ & " = "; _ += *uId_; return this;
	};
	inline TLinker<T, U>* left() {
	  _ & " LEFT JOIN "; _ += U::_name; _& U::_alias; _ & " ON "; _ += *tId_; _ & " = "; _ += *uId_; return this;
	};
	inline TLinker<T, U>* right() {
	  _ & " RIGHT JOIN "; _ += U::_name; _& U::_alias; _ & " ON "; _ += *tId_; _ & " = "; _ += *uId_; return this;
	};
  private: size_t size_{ 10 }, page_{ 1 }; text<0x7ff> _; text<0xff> __; text<0x3f>* tId_, * uId_;
		 inline void fields(text<0x7ff>& os, const char* c);
		 inline void fields(text<0x7ff>& os, const text<0x3f>& v_);
  };
  template<class T, class U>
  void TLinker<T, U>::fields(text<0x7ff>& os, const char* c) {
	os.push_back(','); os += U::_alias + 1; os & "."; if constexpr (ce_is_pgsql) {
	  os.push_back(0x22); os& c; os & "\" AS";
	} else {
	  os.push_back(0x60); os& c; os & "` AS";
	} os += U::_alias; os.push_back(0x5f); os += c;
  };
  template<class T, class U>
  void TLinker<T, U>::fields(text<0x7ff>& os, const text<0x3f>& v_) {
	const char* c = v_.c_str(); os& c; os & " AS"; os += U::_alias; os.push_back(0x5f); if constexpr (ce_is_pgsql) {
	  while (*++c != 0x22) {}; while (*++c != 0x22)os.push_back(*c);
	} else { while (*++c != 0x60) {}; while (*++c != 0x60)os.push_back(*c); } os.push_back(',');
  };/* SELECT User.id, User.account, User.name, User.photo, Role.id AS Role_id, Role.name AS Role_name
  FROM user User LEFT JOIN user_role UserRole ON UserRole.user_id = User.id LEFT JOIN role Role
  ON Role.id = UserRole.role_id  WHERE User.id>1 ORDER BY User.id*/
  //Naming beginning with an uppercase letter means that the object returned is not "*this"
  template<typename T> struct Sql {
	friend class Table<T>;
	Sql<T>() : _("SELECT ") { _.reserve(0x1ff); __.reserve(0x5f); }
	~Sql<T>() {}
	inline Sql<T>* size(size_t size);
	inline Sql<T>* page(size_t page);
	//Cannot be the first field(Because PgSQL does not sort the primary key)
	inline Sql<T>* orderBy(const text<0x3f>& col, const Sort& ord = Sort::ASC);
	//select <T.`$`>,... from <T> T WHERE <T.`$`=?> ORDER BY T.`$1` LIMIT 10 OFFSET 0;
	inline Sql<T>* $() { _ += T::_ios; _ += " FROM "; _ += T::_name; _ += T::_alias; return this; };
	template <typename... K>
	inline Sql<T>* $(K&&...k);
	template<unsigned short I>
	inline Sql<T>* where(const text<I>& str);
	//Default sort for the first field
	std::vector<T> GetArr(const Sort& ord = Sort::ASC);
	inline T GetOne();
	inline decltype(D)::connection_type DB();
	//-------------------------------------DataMapper-------------------------------------
	static void InsertArr(typename T::ptr_arr& t);
	static void InsertArr(std::vector<T>* t);
  private: size_t size_{ 10 }, page_{ 1 }; std::string _, __; bool ___{ true };
		 inline void clear() { _ = "SELECT "; size_ = 10; __[0] = 0; page_ = 1; ___ = true; }
		 inline void fields(std::string& os, const text<0x3f>& v_);//not only but also
		 inline void fields(std::string& os, const char* v_);//v_ only belongs T
  };
  template<typename T> Sql<T>* Sql<T>::size(size_t size) { size_ = size > MAX_LIMIT ? MAX_LIMIT : size; return this; }
  template<typename T> Sql<T>* Sql<T>::page(size_t page) { page_ = page < 1 ? 1 : page return this; }
  template<typename T> Sql<T>* Sql<T>::orderBy(const text<0x3f>& col, const Sort& ord) {
	__ += col.c_str(); __ += SORT[static_cast<char>(ord)]; __.push_back(','); return this;
  }
  template<typename T>
  void Sql<T>::fields(std::string& os, const text<0x3f>& v_) { os += v_.c_str(); os.push_back(','); };
  template<typename T>
  void Sql<T>::fields(std::string& os, const char* v_) {
	os += T::_alias; os.push_back('.'); if constexpr (ce_is_pgsql) {
	  os.push_back(34); os += v_; os.push_back(34);
	} else { os.push_back(96); os += v_; os.push_back(96); } os.push_back(',');
  };
  template<typename T>
  template <typename... K> Sql<T>* Sql<T>::$(K&&...k) {
	Exp{ (fields(_, std::forward<K>(k)), 0)... }; _.pop_back();
	_ += " FROM "; _ += T::_name; _ += T::_alias; return this;
  };
  template<typename T>
  template<unsigned short I> Sql<T>* Sql<T>::where(const text<I>& v_) { _ += " WHERE "; _ += v_.c_str(); return this; }
  template<typename T> std::vector<T> Sql<T>::GetArr(const Sort& ord)noexcept(false) {
	text<0x3ff> sql(_); sql & " ORDER BY "; __ += T::_alias; __.push_back('.'); if constexpr (ce_is_pgsql) {
	  __.push_back(34); __ += T::$[0]; __.push_back(34);
	} else { __.push_back(96); __ += T::$[0]; __.push_back(96); }
	sql += __; sql += SORT[static_cast<char>(ord)]; sql & " LIMIT "; sql += std::to_string(size_);
	sql & " OFFSET "; sql += std::to_string(page_ * size_ - size_);// std::cout << sql << '\n';
	this->clear(); return D.conn()(sql.c_str()).template findArray<T>();
  }
  template<typename T> T Sql<T>::GetOne()noexcept(false) {
	std::string sql(_); this->clear(); return D.conn()(sql).template findOne<T>();
  };
  template<typename T> decltype(D)::connection_type Sql<T>::DB() { ___ = true; return D.conn(); }
  template<typename T> void Sql<T>::InsertArr(typename T::ptr_arr& input) {
	int8_t i = 0; std::ostringstream os, ov; ov << "VALUES "; os << "INSERT INTO " << T::_name << " (";
	for (; i < T::_size; ++i) {
	  if (T::_[i][0] != 'S' && !(T::_tc[i] & (TC::PRIMARY_KEY | TC::AUTO_INCREMENT))) os << T::$[i] << ',';
	} os.seekp(-1, os.cur);
	for (auto o : *input.get()) {
	  i = -1; ov << '(';
	  ForEachField(dynamic_cast<T*>(&o), [&i, &os, &ov](auto& t) {
		if (!(T::_tc[++i] & (TC::PRIMARY_KEY | TC::AUTO_INCREMENT))) {
		  const char* def = T::_def[i];
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
	int8_t i = 0; std::ostringstream os, ov; ov << "VALUES "; os << "INSERT INTO " << T::_name << " (";
	for (; i < T::_size; ++i) {
	  if (T::_[i][0] != 'S' && !(T::_tc[i] & (TC::PRIMARY_KEY | TC::AUTO_INCREMENT))) os << T::$[i] << ',';
	} os.seekp(-1, os.cur);
	for (auto o : *input) {
	  i = -1; ov << '(';
	  ForEachField(dynamic_cast<T*>(&o), [&i, &os, &ov](auto& t) {
		if (!(T::_tc[++i] & (TC::PRIMARY_KEY | TC::AUTO_INCREMENT))) {
		  const char* def = T::_def[i];
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
