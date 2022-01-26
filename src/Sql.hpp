#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<stdarg.h>
#include"macros.hpp"
namespace orm {
  enum class Sort { ASC, DESC }; static const char* SORT[2] = { "", " DESC" };
  template<typename T> class Table;//eg: T::Q()->where(T::$id == 1)->GetOne();
  template<typename T, typename U>//Table Linker. Perfect programming! Follow the principle of minimum code.
  struct TLinker {
	TLinker(TLinker const&) = delete; TLinker& operator=(TLinker&&) = delete;
	TLinker(TLinker&&) = default; TLinker& operator=(TLinker const&) = delete;
	//[select all] T.`<$>`,...
	TLinker<T, U>() : _(T::_ios) {} ~TLinker<T, U>() {}
	inline TLinker<T, U>* size(uint8_t size) { size_ = size; return this; }
	inline TLinker<T, U>* page(size_t page) { page_ = page; return this; }
	json Get() {
	  std::cout << _ << '\n';
	  return D.conn()(_.c_str()).JSON(size_, page_);
	};
	//[select all] U.`<$>` AS U_<$>,... FROM <T> T
	template<typename... K> inline TLinker<T, U>* $() {
	  f(_, U::$[1]); for (uint8_t i = 2; i < U::_size && U::_[i][0] != 'S'; ++i) { f(_, U::$[i]); } _ & " FROM "; _ += T::_name; _& T::_alias; return this;
	};
	//[select K] U.`<$>` -> U.`<$>` AS U_<$> FROM <T> T
	template<typename... K> inline TLinker<T, U>* $(K&&...k) {
	  Exp{ (f(_, std::forward<K>(k)), 0)... }; _ & " FROM "; _ += T::_name; _& T::_alias; return this;
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
  private: uint8_t size_{ 10 }; size_t page_{ 1 }; text<0x6ff> _; text<0xff> __; text<0x3f>* tId_, * uId_;
		 inline void f(text<0x6ff>& os, const char* c);
		 inline void f(text<0x6ff>& os, const text<0x3f>& v_);
  };
  template<class T, class U>
  void TLinker<T, U>::f(text<0x6ff>& os, const char* c) {
	os.push_back(','); os += U::_alias + 1; os & "."; if constexpr (pgsqL) {
	  os.push_back(0x22); os& c; os & "\"";
	} else {
	  os.push_back(0x60); os& c; os & "`";
	} os += U::_as_alia; os += c;
  };
  template<class T, class U>
  void TLinker<T, U>::f(text<0x6ff>& os, const text<0x3f>& v_) {
	os.push_back(','); const char* c = v_.c_str(); os& c; os += U::_as_alia;
	if constexpr (pgsqL) {
	  while (*++c != 0x22) {}; while (*++c != 0x22)os.push_back(*c);
	} else { while (*++c != 0x60) {}; while (*++c != 0x60)os.push_back(*c); }
  };
  template<typename T> struct Sql {
	friend class Table<T>;
	Sql<T>() : _(T::_ios) { _.reserve(0x1ff); __.reserve(0x5f); _ += " FROM "; _ += T::_name; _ += T::_alias; }
	~Sql<T>() {}
	inline Sql<T>* size(uint8_t size);
	inline Sql<T>* page(size_t page);
	//Cannot be the first field(Because PgSQL does not sort the primary key)
	inline Sql<T>* orderBy(const text<0x3f>& col, const Sort& ord = Sort::ASC);
	//select <T.`$`>,... from <T> T WHERE <T.`$`=?> ORDER BY T.`$1` LIMIT 10 OFFSET 0;
	template<unsigned short I>
	inline Sql<T>* where(const text<I>& str);
	//Default ASC for the first field
	std::vector<T> GetArr(const Sort& ord = Sort::ASC);
	inline T GetOne();
	inline decltype(D)::connection_type DB();
	//-------------------------------------DataMapper-------------------------------------
	static void InsertArr(typename T::arr& t);
	static void InsertArr(std::vector<T>* t);
  private: uint8_t size_{ 10 }; size_t page_{ 1 }; std::string _, __; bool ___{ true };
		 inline void clear() {
		   _ = T::_ios; _ += " FROM "; _ += T::_name; _ += T::_alias; size_ = 10; __[0] = 0; page_ = 1; ___ = true;
		 }
  };
  template<typename T> Sql<T>* Sql<T>::size(uint8_t size) { size_ = size; return this; }
  template<typename T> Sql<T>* Sql<T>::page(size_t page) { page_ = page; return this; }
  template<typename T> Sql<T>* Sql<T>::orderBy(const text<0x3f>& col, const Sort& ord) {
	__ += col.c_str(); __ += SORT[static_cast<char>(ord)]; __.push_back(','); return this;
  }
  template<typename T>
  template<unsigned short I> Sql<T>* Sql<T>::where(const text<I>& v_) { _ += " WHERE "; _ += v_.c_str(); return this; }
  template<typename T> std::vector<T> Sql<T>::GetArr(const Sort& ord)noexcept(false) {
	std::string sql(_); sql += " ORDER BY "; __ += T::_alias; __.push_back('.'); if constexpr (pgsqL) {
	  __.push_back(34); __ += T::$[0]; __.push_back(34);
	} else { __.push_back(96); __ += T::$[0]; __.push_back(96); }
	sql += __; sql += SORT[static_cast<char>(ord)]; sql += " LIMIT "; sql += std::to_string(size_);
	sql += " OFFSET "; sql += std::to_string((page_ - 1) * size_);// std::cout << sql << '\n';
	this->clear(); return D.conn()(sql).template findArray<T>();
  }
  template<typename T> T Sql<T>::GetOne()noexcept(false) {
	std::string sql(_); this->clear(); return D.conn()(sql).template findOne<T>();
  };
  template<typename T> decltype(D)::connection_type Sql<T>::DB() { ___ = true; return D.conn(); }
  template<typename T> void Sql<T>::InsertArr(typename T::arr& input) {
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
			if constexpr (pgsqL) { ov << (t ? "true" : "false") << ','; } else { ov << t << ','; }
		  } else if constexpr (std::is_fundamental<std::remove_reference_t<decltype(t)>>::value) {
			if (!*((char*)&t) && def[0]) { ov << def << ','; } else { ov << t << ','; }
		  } else if constexpr (std::is_same<tm, std::remove_reference_t<decltype(t)>>::value) {
			if (!*((char*)&t)) ov << '\'' << (def[0] ? def : "0000-00-00 00:00:00") << "',"; else ov << '\'' << t << "',";
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
			if constexpr (pgsqL) { ov << (t ? "true" : "false") << ','; } else { ov << t << ','; }
		  } else if constexpr (std::is_fundamental<std::remove_reference_t<decltype(t)>>::value) {
			if (!*((char*)&t) && def[0]) { ov << def << ','; } else { ov << t << ','; }
		  } else if constexpr (std::is_same<tm, std::remove_reference_t<decltype(t)>>::value) {
			if (!*((char*)&t)) ov << '\'' << (def[0] ? def : "0000-00-00 00:00:00") << "',"; else ov << '\'' << t << "',";
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
