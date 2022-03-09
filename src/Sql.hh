#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<stdarg.h>
#include"macros.hh"
#define N0TYPE(A) A::_[i][0]!=0x2a&&A::_[i][0]!=0x5f
namespace orm {
  enum class Sort { ASC, DESC }; static const char* SORT[2] = { "", " DESC" };
  template<typename T> class Table;//eg: T::Q()->$(T::$id, T::$name, T::$date)->where(T::$id == 1)->GetOne();
  template<class T, class U>//Table Linker(Default INNER JOIN) Perfect programming! Follow the principle of minimum code.
  struct TLinker {
	TLinker(TLinker const&) = delete; TLinker& operator=(TLinker&&) = delete;
	TLinker(TLinker&&) = default; TLinker& operator=(TLinker const&) = delete;
	~TLinker<T, U>() {}
	//max to three conditions -> eg:(Tab::$id == Type::$id) && (Type::name == Tab::$name) && (Type::age == Tab::age)
	template<unsigned short I>
	TLinker<T, U>(const text<I>& v_) : _(T::_ios), ___(v_), w_("") {
	  f(_, U::$[0]); for (uint8_t i = 1; i < U::_size && N0TYPE(U); ++i) { f(_, U::$[i]); }
	  _ += " FROM "; _ += T::_name; _ += T::_alias;
	}
	//Default on() conditions -> Associate the primary key of two tables => `Tab`.`id`=`Type`.`id`
	TLinker<T, U>() : _(T::_ios), w_("") {
	  f(_, U::$[0]); for (uint8_t i = 1; i < U::_size && N0TYPE(U); ++i) { f(_, U::$[i]); }_ += " FROM ";
	  _ += T::_name; _ += T::_alias; std::string s; F<T>(s, T::$[0]); s += " = "; F<U>(s, U::$[0]); ___ = s;
	}
	inline TLinker<T, U>& size(uint8_t size) { size_ = size < 1 ? 1 : size; return *this; }
	inline TLinker<T, U>& page(size_t page) { page_ = page < 1 ? 1 : page; return *this; }
	template<unsigned short I>
	inline TLinker<T, U>& where(const text<I>& str) { w_ += " WHERE "; w_ += str.c_str(); return *this; }
	inline TLinker<T, U>& orderBy(const text<0x3f>& col, const Sort& ord = Sort::ASC) {
	  __ += col.c_str(); __ += SORT[static_cast<char>(ord)]; __.push_back(','); return *this;
	};
	//The properties of the first parameter passed in will be changed
	T GetOne(U* u, const Sort& ord = Sort::ASC) {
	  _ += " INNER JOIN "; _ += U::_name; _ += U::_alias; _ += " ON "; _ += ___.c_str(); if (w_[0]) _ += w_;
	  _ += " ORDER BY "; __ += T::_alias; __.push_back('.'); if constexpr (pgsqL) {
		__.push_back(34); __ += T::$[0]; __.push_back(34);
	  } else { __.push_back(96); __ += T::$[0]; __.push_back(96); } _ += __; _ += SORT[static_cast<char>(ord)];
	  _ += " LIMIT 1"; return D.conn()(_).template findOne<T>(u);
	};
	//If the first parameter passed in contains a value, it will be cleared.
	std::vector<T> GetArr(std::vector<U>* vu, const Sort& ord = Sort::ASC) {
	  _ += " INNER JOIN "; _ += U::_name; _ += U::_alias; _ += " ON "; _ += ___.c_str(); if (w_[0]) _ += w_;
	  _ += " ORDER BY "; __ += T::_alias; __.push_back('.'); if constexpr (pgsqL) {
		__.push_back(34); __ += T::$[0]; __.push_back(34);
	  } else { __.push_back(96); __ += T::$[0]; __.push_back(96); } _ += __; _ += SORT[static_cast<char>(ord)];
	  _ += " LIMIT "; _ += std::to_string(size_); _ += " OFFSET "; _ += std::to_string((page_ - 1) * size_);
	  return D.conn()(_).template findArray<T>(vu);
	};
	//Implementing many-to-many
	inline void leftJoin() {
	  //_ += " LEFT JOIN "; _ += U::_name; _ += U::_alias; _ += " ON "; _ += ___.c_str();
	  //_ += " LIMIT "; _ += std::to_string(size_); _ += " OFFSET "; _ += std::to_string((page_ - 1) * size_);
	};
	//The principle is similar to the reflector of Rust language, but closer to the machine code
	json GetJson() {
	  constexpr auto $ = std::tuple_cat(li::Tuple<T>(), li::Tuple<U>()); T t; U u; int8_t z = -1, y = -1;
	  ForEachTuple($, [&t, &u, &z, &y](auto& _) {
		//std::cout << getObjectName<decltype(li::ExT(_))>() << "->";
		if constexpr (std::is_same<decltype(li::ExT(_)), T>::value) {
		  T* o = dynamic_cast<T*>(&t); ++z; using Z = std::remove_reference_t<decltype(o->*_)>;
		  if constexpr (li::is_vector<Z>::value) {
			std::vector<li::vector_pack_t<Z>> v_t = o->*_;
			std::cout << T::$[z] << ":vector<" << typeid(li::vector_pack_t<Z>).name() << ">, ";
		  } else if constexpr (li::is_ptr<Z>::value) {
			std::cout << T::$[z] << ":*" << typeid(li::ptr_pack_t<Z>).name() << ", ";
		  } else {
			std::cout << T::$[z] << ':' << typeid(o->*_).name() << ", ";
		  }
		} else {
		  U* o = dynamic_cast<U*>(&u); ++y; using Y = std::remove_reference_t<decltype(o->*_)>;
		  if constexpr (li::is_vector<Y>::value) {
			std::cout << U::$[y] << ":vector<" << typeid(li::vector_pack_t<Y>).name() << ">";
		  } else if constexpr (li::is_ptr<Y>::value) {
			std::cout << U::$[y] << ":*" << typeid(li::ptr_pack_t<Y>).name();
		  } else {
			std::cout << U::$[y] << ':' << typeid(o->*_).name() << ", ";
		  }
		}
		//std::cout << '\n' << typeid(li::ExP<T>(li::ExP(_))).name();
		}, std::make_index_sequence<std::tuple_size<decltype($)>::value>{}); std::cout << '\n';
	  return json{ "Only stars, flowers and applause can satisfy my arrogance and greatness" };
	};
  private: uint8_t size_{ 10 }; size_t page_{ 1 }; std::string _, __, w_; text<0x188> ___;
		 template<typename V> inline void F(std::string& os, const std::string_view& c);//&os += `V`.`id`
		 inline void f(std::string& os, const std::string_view& c);//_ += `U`.`id` AS U_id
  };
  template<class T, class U>
  template<typename V>
  inline void TLinker<T, U>::F(std::string& os, const std::string_view& c) {
	os += V::_alias + 1; os.push_back('.'); if constexpr (pgsqL) {
	  os.push_back('"'); os += c; os.push_back('"');
	} else { os.push_back('`'); os += c; os.push_back('`'); }
  }
  template<class T, class U>
  inline void TLinker<T, U>::f(std::string& os, const std::string_view& c) {
	os.push_back(','); os += U::_alias + 1; os.push_back('.'); if constexpr (pgsqL) {
	  os.push_back('"'); os += c; os.push_back('"');
	} else { os.push_back('`'); os += c; os.push_back('`'); } os += U::_as_alia; os += c;
  };
  //Naming beginning with an uppercase letter means that the object returned is not "*this"
  template<typename T> struct Sql {
	friend class Table<T>;
	Sql<T>() : _(T::_ios) { _.reserve(0x1ff); __.reserve(0x5f); _ += " FROM "; _ += T::_name; _ += T::_alias; }
	~Sql<T>() {}
	inline Sql<T>& size(uint8_t size);
	inline Sql<T>& page(size_t page);
	//Cannot be the first field(Because PgSQL does not sort the primary key)
	inline Sql<T>& orderBy(const text<0x3f>& col, const Sort& ord = Sort::ASC);
	//select <T.`$`>,... from <T> T WHERE <T.`$`=?> ORDER BY T.`$1` LIMIT 10 OFFSET 0;
	template<unsigned short I>
	inline Sql<T>& where(const text<I>& str);
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
  template<typename T> Sql<T>& Sql<T>::size(uint8_t size) { size_ = size < 1 ? 1 : size; return *this; }
  template<typename T> Sql<T>& Sql<T>::page(size_t page) { page_ = page < 1 ? 1 : page; return *this; }
  template<typename T> Sql<T>& Sql<T>::orderBy(const text<0x3f>& col, const Sort& ord) {
	__ += col.c_str(); __ += SORT[static_cast<char>(ord)]; __.push_back(','); return *this;
  }
  template<typename T>
  template<unsigned short I> Sql<T>& Sql<T>::where(const text<I>& v_) { _ += " WHERE "; _ += v_.c_str(); return *this; }
  template<typename T> std::vector<T> Sql<T>::GetArr(const Sort& ord)noexcept(false) {
	std::string sql(_); sql += " ORDER BY "; __ += T::_alias; __.push_back('.'); if constexpr (pgsqL) {
	  __.push_back(34); __ += T::$[0]; __.push_back(34);
	} else { __.push_back(96); __ += T::$[0]; __.push_back(96); }
	sql += __; sql += SORT[static_cast<char>(ord)]; sql += " LIMIT "; sql += std::to_string(size_);
	sql += " OFFSET "; sql += std::to_string((page_ - 1) * size_);// std::cout << sql << '\n';
	this->clear(); return D.conn()(sql).template findArray<T>();
  }
  template<typename T> T Sql<T>::GetOne()noexcept(false) {
	std::string sql(_); sql += " LIMIT 1"; this->clear(); return D.conn()(sql).template findOne<T>();
  };
  template<typename T> decltype(D)::connection_type Sql<T>::DB() { ___ = true; return D.conn(); }
  template<typename T> void Sql<T>::InsertArr(typename T::arr& input) {
	int8_t i = 0; std::ostringstream os, ov; ov << "VALUES "; os << "INSERT INTO " << T::_name << " (";
	for (; i < T::_size; ++i) {
	  if (!(T::_tc[i] & (TC::PRIMARY_KEY | TC::AUTO_INCREMENT)) && N0TYPE(T)) {
		if constexpr (pgsqL) { os << '"' << T::$[i] << "\","; } else { os << T::$[i] << ','; }
	  }
	} os.seekp(-1, os.cur);
	for (auto o : *input.get()) {
	  i = -1; ov << '(';
	  ForEachField(dynamic_cast<T*>(&o), [&i, &os, &ov](auto& t) {
		if (!(T::_tc[++i] & (TC::PRIMARY_KEY | TC::AUTO_INCREMENT))) {
		  const char* def = T::_def[i]; using Y = std::remove_reference_t<decltype(t)>;
		  if constexpr (std::is_same<bool, Y>::value) {
			if constexpr (pgsqL) { ov << (t ? "true" : "false") << ','; } else { ov << t << ','; }
		  } else if constexpr (std::is_fundamental<Y>::value) {
			if (!*((char*)&t) && def[0]) { ov << def << ','; } else { ov << t << ','; }
		  } else if constexpr (std::is_same<tm, Y>::value) {
			if (!*((char*)&t)) ov << '\'' << (def[0] ? def : "0000-00-00 00:00:00") << "',"; else ov << '\'' << t << "',";
		  } else if constexpr (std::is_same<std::string, Y>::value) {
			if (!*((char*)&t)) { ov << '\'' << toQuotes(def) << "',"; } else ov << '\'' << toQuotes(t.c_str()) << "',";
		  } else if constexpr (is_text<Y>::value) {
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
	  if (!(T::_tc[i] & (TC::PRIMARY_KEY | TC::AUTO_INCREMENT)) && N0TYPE(T)) {
		if constexpr (pgsqL) { os << '"' << T::$[i] << "\","; } else { os << T::$[i] << ','; }
	  }
	} os.seekp(-1, os.cur);
	for (auto o : *input) {
	  i = -1; ov << '(';
	  ForEachField(dynamic_cast<T*>(&o), [&i, &os, &ov](auto& t) {
		if (!(T::_tc[++i] & (TC::PRIMARY_KEY | TC::AUTO_INCREMENT))) {
		  const char* def = T::_def[i]; using Y = std::remove_reference_t<decltype(t)>;
		  if constexpr (std::is_same<bool, Y>::value) {
			if constexpr (pgsqL) { ov << (t ? "true" : "false") << ','; } else { ov << t << ','; }
		  } else if constexpr (std::is_fundamental<Y>::value) {
			if (!*((char*)&t) && def[0]) { ov << def << ','; } else { ov << t << ','; }
		  } else if constexpr (std::is_same<tm, Y>::value) {
			if (!*((char*)&t)) ov << '\'' << (def[0] ? def : "0000-00-00 00:00:00") << "',"; else ov << '\'' << t << "',";
		  } else if constexpr (std::is_same<std::string, Y>::value) {
			if (!*((char*)&t)) { ov << '\'' << toQuotes(def) << "',"; } else ov << '\'' << toQuotes(t.c_str()) << "',";
		  } else if constexpr (is_text<Y>::value) {
			if (!*((char*)&t)) { ov << '\'' << toQuotes(def) << "',"; } else ov << '\'' << toQuotes(t.c_str()) << "',";
		  } else { return; }
		}
		}); ov.seekp(-1, ov.cur); ov << "),";
	}
	os << ')'; os << ' ' << ov.str(); os.seekp(-1, os.cur); os << ";"; D.conn()(os.str());
  }
}//chrono::milliseconds(100)microseconds
