#pragma once
#include "./Sql.hh"
#include "./base/Initalization.hh"
#include "./base/s2o.hh"
#include "./base/so2s.hh"
#define Struct(T) struct T : orm::Table<T>/*multiple inheritance from std::enable_shared_from_this<T> => needed for
struct A: virtual_shared<A> {}; struct B: virtual_shared<B> {}; struct Z: A, B { };*/
namespace orm {
  struct enable_virtual : std::enable_shared_from_this<enable_virtual> { virtual ~enable_virtual() {} };
  template<typename T> struct virtual_shared : virtual enable_virtual {
	std::shared_ptr<T> shared_from_this() {
	  return std::dynamic_pointer_cast<T>(std::enable_shared_from_this<enable_virtual>::shared_from_this());
	}
  };/*int main() { std::shared_ptr<Z> z = std::make_shared<Z>(); std::shared_ptr<B> b = z->B::shared_from_this(); } */
  template<typename T> class Table : public virtual_shared<T> { /*struct size---proto length----Store alias*//*AS [alias]_<k>*/
	static const std::string _name, _drop, _low, _mT; static const uint8_t _size, _len; const static char* _alias, * _as_alia;//^
	static bool _created; static unsigned char _idex; static std::string _create; static const size_t _o$[];/*Store offset[]*/
	static const std::string_view $[], _ios;/*Store <k> name[]*//*Store (T.<k>,)...*//*_low -> lower name*//*_mT -> M_TABLE name*/
#ifdef _WIN32
	friend typename T; const static char* _def[];/*Store default values[]*/static unsigned char _tc[];/*Store key type[]*/
#endif
	template<typename U, typename V> friend struct TLinker; friend typename decltype(D)::db_rs;
	friend struct Sql<T>; static const char* _[];/*Store type character[]*/static int _r, _r1;/*Prepare Run Serialization*/
	template <typename U> void $et(int8_t i, const U* v) {
	  if constexpr (std::is_same<U, const char*>::value) {
		switch (*_[i]) {
		case T_TEXT_:*reinterpret_cast<text<>*>(RUST_CAST(this) + this->_o$[i]) = *v; break;
		case T_STRING_:*reinterpret_cast<std::string*>(RUST_CAST(this) + this->_o$[i]) = *v;
		}
	  } else *reinterpret_cast<U*>(RUST_CAST(this) + this->_o$[i]) = *v;
	}
	template <typename U> friend typename std::enable_if<li::is_vector<U>::value, void>::type FuckJSON(const U& u, const char* s, json& j);
	template <typename U> friend typename std::enable_if<li::is_ptr<U>::value &&
	  !std::is_fundamental<U>::value, void>::type FuckJSON(const U& u, const char* c, json& j);
	template <typename U> friend std::string& operator<<(std::string& s, Table<U>* c);//<T> serialized as string
	template <typename U> friend std::ostream& operator<<(std::ostream& o, Table<U>* c);
	template <typename U> friend std::ostream& operator<<(std::ostream& o, Table<U>& c);
	template <typename U> friend std::string& operator<<(std::string& s, std::vector<U> c);//vector<T> serialized as string
	template <typename U> friend std::ostream& operator<<(std::ostream& o, std::vector<U> c);
	template <typename U> friend std::string& operator<<(std::string& s, std::vector<U>* c);//vector<T>* serialized as string
	template <typename U> friend std::ostream& operator<<(std::ostream& o, std::vector<U>* c);
  public:
#ifndef _WIN32
	const static char* _def[]; static unsigned char _tc[];
#endif
	using ptr = typename std::shared_ptr<T>; static Sql<T>* __[];
	using arr = typename std::vector<typename std::shared_ptr<T>>;
	Table& operator=(const Table&) = default; static int Init();//Initalization Table
	Table(); ~Table() {} Table(const Table&) = default; static void Dev();//Dev Mode, need orm::FastestDev = true
	template<typename ... Args> static ptr create(Args&& ... args);
	template<typename... U> void set(U... t) {
	  int8_t idex = -1; Exp{ ($et(++idex, &t), 0)... }; return;/*This code will never arrive && detect types*/ *this = T(t...);
	}
	//Query builder
	static Sql<T>& Q() {
	_: if (++_idex > HARDWARE_CORE) _idex = 0; Sql<T>* q = __[_idex];
	  if (q->___) { q->___ = false; return *q; } std::this_thread::yield(); goto _;
	};
	//<T> serialized as JSON with std::vector, includes empty std::vector
	json get() { return json(*dynamic_cast<T*>(this)); }
	//-------------------------------------ActiveRecord-------------------------------------
	Table& operator=(Table&& t) {
	  if (this != &t) {}return *this;
	}
	//
	inline bool is_null() { return *((char*)(RUST_CAST(this) + this->_o$[0])) == 0; }
	//Insert the object (Returns the inserted ID)
	auto Insert() {
	  int8_t i = -1; std::ostringstream os, ov; ov << "VALUES ("; os << "INSERT INTO " << _name << " (";
	  ForEachField(dynamic_cast<T*>(this), [&i, &os, &ov](auto& t) {
		using Y = std::remove_reference_t<decltype(t)>;
		if (!(_tc[++i] & (TC::PRIMARY_KEY | TC::AUTO_INCREMENT))) {
		  if constexpr (std::is_same<bool, Y>::value) {
			if constexpr (pgsqL) {
			  ov << (t ? "true" : "false") << ','; os << '"' << T::$[i] << "\",";
			} else { ov << t << ','; os << T::$[i] << ','; }
		  } else if constexpr (std::is_fundamental<Y>::value) {
			if (*((char*)&t)) {
			  ov << t << ','; if constexpr (pgsqL) { os << '"' << T::$[i] << "\","; } else { os << T::$[i] << ','; }
			}
		  } else if constexpr (std::is_same<tm, Y>::value) {
			if (*((char*)&t)) {
			  ov << '\'' << t << "',";
			  if constexpr (pgsqL) { os << '"' << T::$[i] << "\","; } else { os << T::$[i] << ','; }
			}
		  } else if constexpr (std::is_same<std::string, Y>::value) {
			if (*((char*)&t)) {
			  ov << '\'' << toQuotes(t.c_str()) << "',";
			  if constexpr (pgsqL) { os << '"' << T::$[i] << "\","; } else { os << T::$[i] << ','; }
			}
		  } else if constexpr (is_text<Y>::value) {
			if (*((char*)&t)) {
			  ov << '\'' << toQuotes(t.c_str()) << "',";
			  if constexpr (pgsqL) { os << '"' << T::$[i] << "\","; } else { os << T::$[i] << ','; }
			}
		  }
		}
		});
	  os.seekp(-1, os.cur); os << ')'; ov.seekp(-1, ov.cur); ov << ")"; os << ' ' << ov.str();
	  if constexpr (pgsqL) { os << " RETURNING \"" << T::$[0] << "\";"; } else { os << ";"; }
	  li::sql_result<decltype(D)::db_rs>&& rs = D.conn()(os.str());
	  if (T::_tc[0] & TC::AUTO_INCREMENT) {
		return rs.last_insert_id();
	  } else if constexpr (mysqL) { return 0ULL; } else { return 0LL; }
	}
	//Update the object (The default condition is the value of the frist key)
	void Update() {
	  int8_t i = -1; std::ostringstream os; os << "UPDATE " << _name << " SET "; std::string cd(" WHERE ");
	  if constexpr (pgsqL) {
		cd.push_back('"'); cd += $[0]; cd.push_back('"'); cd.push_back('=');
	  } else { cd += $[0]; cd.push_back('='); }
	  auto& t = dynamic_cast<T*>(this)->*std::get<0>(li::Tuple<T>());
	  using Y = std::remove_reference_t<decltype(t)>;
	  if constexpr (std::is_fundamental<Y>::value) {
		cd += std::to_string(t);
	  } else if constexpr (std::is_same<tm, Y>::value) {
		cd.push_back('\''); cd << t; cd.push_back('\'');
	  } else if constexpr (std::is_same<std::string, Y>::value) {
		cd.push_back('\''); cd += toQuotes(t.c_str()); cd.push_back('\'');
	  } else if constexpr (is_text<Y>::value) {
		cd.push_back('\''); cd += toQuotes(t.c_str()); cd.push_back('\'');
	  }
	  ForEachField(dynamic_cast<T*>(this), [&i, &os](auto& t) {
		using Z = std::remove_reference_t<decltype(t)>;
		if (++i && !(T::_tc[i] & TC::AUTO_INCREMENT)) {
		  if constexpr (std::is_same<bool, Z>::value) {
			if constexpr (pgsqL) { os << '"' << T::$[i] << "\"=" << (t ? "true" : "false") << ','; } else { os << T::$[i] << '=' << t << ','; }
		  } else if constexpr (std::is_fundamental<Z>::value) {
			if constexpr (pgsqL) { os << '"' << T::$[i] << '"'; } else { os << T::$[i]; } os << '=' << t << ',';
		  } else if constexpr (std::is_same<tm, Z>::value) {
			if constexpr (pgsqL) { os << '"' << T::$[i] << '"'; } else { os << T::$[i]; } os << '=' << '\'' << t << "',";
		  } else if constexpr (std::is_same<std::string, Z>::value) {
			if constexpr (pgsqL) { os << '"' << T::$[i] << '"'; } else { os << T::$[i]; } os << '=' << '\'' << toQuotes(t.c_str()) << "',";
		  } else if constexpr (is_text<Z>::value) {
			if constexpr (pgsqL) { os << '"' << T::$[i] << '"'; } else { os << T::$[i]; } os << '=' << '\'' << toQuotes(t.c_str()) << "',";
		  }
		}
		});
	  os.seekp(-1, os.cur); os << cd << ";";//std::cout<<os.str();
	  D.conn()(os.str());
	}
	//Delete the object based on this object's frist key
	void Delete() {
	  std::ostringstream os; os << "DELETE FROM " << _name;
	  if constexpr (pgsqL) {
		os << " WHERE \"" << $[0] << "\"=";
	  } else { os << " WHERE " << $[0] << '='; }
	  auto& t = dynamic_cast<T*>(this)->*std::get<0>(li::Tuple<T>());
	  using Y = std::remove_reference_t<decltype(t)>;
	  if constexpr (std::is_same<bool, Y>::value) {
		if constexpr (pgsqL) { os << (t ? "true" : "false"); } else { os << t; }
	  } else if constexpr (std::is_fundamental<Y>::value) {
		os << t;
	  } else if constexpr (std::is_same<tm, Y>::value) {
		os << '\'' << t << '\'';
	  } else if constexpr (std::is_same<std::string, Y>::value) {
		os << '\'' << toQuotes(t.c_str()) << '\'';
	  } else if constexpr (is_text<Y>::value) {
		os << '\'' << toQuotes(t.c_str()) << '\'';
	  } os << ";";
	  D.conn()(os.str());
	}
	static int _addTable() {//MAC system is not necessarily supported because I don't have physical machine test
	  if (_created) {
		_created = false; bool wrong_order = false; try {
		  for (uint8_t i = 0; i < _size; ++i) {
			if (*_[i] == 0x2a || *_[i] == 0x5f) { if (i < _len) { wrong_order = true; } continue; }
			if (wrong_order) { throw std::runtime_error("\033[1;34m*/[]\033[31;4m type should be at the end!\n\033[0m" __FILE__); }
			if (_tc[i] & TC::AUTO_INCREMENT) {//check sequence key, and it must be number
			  switch (*_[i]) {
			  case T_INT8_: case T_INT16_: case T_INT_: case T_INT64_: case T_UINT8_: case T_UINT16_: case T_UINT_: case T_UINT64_:
			  break; default:throw std::runtime_error("\033[1;34m[sequence]\033[31;4m type must be number!\n\033[0m");
			  }
			}
			TC tc = (TC)_tc[i]; const char* def = _def[i]; if (i)_create += ",\n";
			if constexpr (pgsqL) {
			  _create.push_back('"'); _create += $[i]; _create.push_back('"');
			  if (tc & TC::PRIMARY_KEY || tc & TC::AUTO_INCREMENT) {//Compatible layer, unsigned field may be only half the size
				switch (*_[i]) {
				case T_INT8_: case T_UINT8_:
				case T_INT16_: case T_UINT16_: _create += " SMALLSERIAL PRIMARY KEY"; break;
				case T_INT_: case T_UINT_: _create += " SERIAL PRIMARY KEY"; break;
				case T_INT64_: case T_UINT64_: _create += " BIGSERIAL PRIMARY KEY"; break;
				} continue;
			  }
			} else if constexpr (sqlitE) {//AUTOINCREMENT is only allowed on an INTEGER PRIMARY KEY
			  _create.push_back('`'); _create += $[i]; _create.push_back('`');
			  if (tc & TC::PRIMARY_KEY && tc & TC::AUTO_INCREMENT) { _create += " INTEGER PRIMARY KEY AUTOINCREMENT"; continue; }
			} else { _create.push_back('`'); _create += $[i]; _create.push_back('`'); }
			switch (*_[i]) {
			case T_BOOL_: _create += " BOOLEAN"; if (tc & TC::NOT_NULL) { _create += " NOT NULL"; }
						if constexpr (pgsqL) { goto $; }
						if (tc & TC::DEFAULT && so2s<bool>(def)) {
						  _create += " DEFAULT "; _create.push_back('\''); if (def[0] == 't') {
							_create.push_back('1');
						  } else { _create.push_back('0'); } _create.push_back('\'');
						} continue;//or DECIMAL(16,6), Because the decimal places of double are only 6 in C++
			case T_DOUBLE_: if constexpr (pgsqL) { _create += " DOUBLE PRECISION"; } else { _create += " DOUBLE"; } goto $;
			case T_FLOAT_: _create += " REAL"; goto $;
			case T_INT8_: _create += " TINYINT"; break;
			case T_INT16_: _create += " SMALLINT"; break;
			case T_INT_: _create += " INTEGER"; if constexpr (sqlitE) { if (tc & TC::AUTO_INCREMENT) _create += " AUTOINCREMENT"; } break;
			case T_INT64_: _create += " BIGINT"; break;
			case T_TM_: if constexpr (pgsqL) {
			  _create += " timestamp without time zone";
			} else { _create += " DATETIME"; }
			if (tc & TC::NOT_NULL) { _create += " NOT NULL"; }
			if (tc & TC::DEFAULT) {
			  if (so2s<tm>(def)) { _create += " DEFAULT '"; _create += def; _create.push_back('\''); continue; }
			  if constexpr (sqlitE) {
				_create += " DEFAULT (datetime('now','localtime'))";
			  } else if constexpr (pgsqL) {
				_create += " DEFAULT now()";
			  } else { _create += " DEFAULT CURRENT_TIMESTAMP"; }
			} continue;
			case T_TEXT_: {_create += " VARCHAR("; _create += _[i] + 1; _create.push_back(')'); } goto $;
			case T_STRING_: _create += " TEXT"; goto $;
			case T_UINT8_: if constexpr (pgsqL) {
			  _create += " UNSIGNED_TINYINT";
			} else if constexpr (mysqL) { _create += " TINYINT UNSIGNED"; } else { _create += " TINYINT"; } break;
			case T_UINT16_: if constexpr (pgsqL) {
			  _create += " UNSIGNED_SMALLINT";
			} else if constexpr (mysqL) { _create += " SMALLINT UNSIGNED"; } else { _create += " SMALLINT"; } break;
			case T_UINT_: if constexpr (pgsqL) {
			  _create += " UNSIGNED_INTEGER";
			} else if constexpr (mysqL) { _create += " INTEGER UNSIGNED"; } else { _create += " INTEGER"; } break;
			case T_UINT64_: if constexpr (pgsqL) {
			  _create += " UNSIGNED_BIGINT";
			} else if constexpr (mysqL) { _create += " BIGINT UNSIGNED"; } else { _create += " BIGINT"; } break;
			}
			if constexpr (sqlitE) {
			  if (tc & TC::PRIMARY_KEY) _create += " PRIMARY KEY";
			}
			if constexpr (mysqL) {
			  if (tc & TC::PRIMARY_KEY) _create += " PRIMARY KEY"; if (tc & TC::AUTO_INCREMENT) _create += " AUTO_INCREMENT";
			}
		  $://String type detection system => unsigned -> d
			if (tc & TC::NOT_NULL) { _create += " NOT NULL"; }
			if (tc & TC::DEFAULT) {
			  switch (*_[i]) {
			  case T_DOUBLE_: if (!so2s<double>(def)) { break; } goto _;
			  case T_FLOAT_: if (!so2s<float>(def)) { break; } goto _;
			  case T_INT64_: if (!so2s<long long>(def)) { break; } goto _;
			  case T_INT8_: if (!so2s<int8_t>(def)) { break; } goto _;
			  case T_BOOL_: if (!so2s<bool>(def)) { break; } goto _;
			  case T_INT16_: if (!so2s<short>(def)) { break; } goto _;
			  case T_INT_: if (!so2s<int>(def)) { break; } _: _create += " DEFAULT ";
				_create.push_back('\''); _create += def; _create.push_back('\''); break;
			  case T_TEXT_:
			  case T_STRING_: _create += " DEFAULT "; _create.push_back('\''); _create += toQuotes(def); _create.push_back('\''); break;
			  case T_UINT8_:
			  case T_UINT16_:
			  case T_UINT_:
			  case T_UINT64_: goto _;
			  }
			}
		  } _create += "\n);\n";
		} catch (const std::exception& e) { std::cerr << e.what(); exit(0); }
	  }
	  auto DbQuery = D.conn();
	  std::cout << _create;
	  if constexpr (pgsqL) {
		std::string str_("select count(*) from pg_class where relname = '"); str_ += _name.c_str() + 1; str_.pop_back(); str_ += "';";
		if (DbQuery(str_).template r__<int>() == 0) {
		  DbQuery(_create); Dev(); return 1;
		}// else { DbQuery(_drop); DbQuery(_create).flush_results(); }
	  } else {
		try {
		  DbQuery(_create); Dev(); return 1;
		} catch (std::runtime_error e) {
		  std::cerr << "\033[1;4;31mWarning:\033[0m could not create the \033[1;34m[" << _name
			<< "]\033[0m table.\nBecause: \033[4;33m" << e.what() << "\033[0m\n"; return 0;
		}
	  } return 0;
	}
	static void _dropTable() { D.conn()(_drop); }
  };
  template<typename T> Table<T>::Table() {}
  template<typename T> template<typename ... Args>
  typename Table<T>::ptr Table<T>::create(Args&& ... args) { return std::make_shared<T>(std::forward<Args>(args)...); }

  template <typename T> std::string& operator<<(std::string& s, Table<T>* c) {
	if (c == nullptr || *((char*)(RUST_CAST(c) + c->_o$[0])) == 0) { s = "null"; return s; }
	s.push_back('{'); int8_t i = -1;
	ForEachField(dynamic_cast<T*>(c), [&i, c, &s](auto& t) {
	  using Y = std::remove_reference_t<decltype(t)>;
	  if constexpr (!li::is_vector<Y>::value) { s.push_back('"'); s += c->$[++i]; } else { ++i; }
	  if constexpr (std::is_same<tm, Y>::value) {
		s += "\":\""; std::ostringstream os; const tm* time = &t; os << std::setfill('0');
#ifdef _WIN32
		os << std::setw(4) << time->tm_year + 1900;
#else
		int y = time->tm_year / 100; os << std::setw(2) << 19 + y << std::setw(2) << time->tm_year - y * 100;
#endif
		os << '-' << std::setw(2) << (time->tm_mon + 1) << '-' << std::setw(2) << time->tm_mday << ' ' << std::setw(2)
		  << time->tm_hour << ':' << std::setw(2) << time->tm_min << ':' << std::setw(2) << time->tm_sec << '"'; s += os.str();
	  } else if constexpr (std::is_same<bool, Y>::value) {
		s += "\":", s += t == true ? "true" : "false";
	  } else if constexpr (std::is_fundamental<Y>::value) {
		s += "\":" + std::to_string(t);
	  } else if constexpr (std::is_same<std::string, Y>::value) {
		s += "\":\"" + t + "\"";
	  } else if constexpr (li::is_vector<Y>::value) {
		size_t l = t.size(); if (l) { s.push_back('"'); s += c->$[i]; s += "\":"; s << &t; } else { s.pop_back(); }
	  } else if constexpr (li::is_ptr<Y>::value) {
		s += "\":"; t == nullptr ? s += "null" : s << t;
	  } else if constexpr (is_text<std::remove_reference_t<decltype(t)>>::value) {
		s += "\":"; s << t;
	  } else {
		s += "\":"; s << &t;
	  } s.push_back(',');
	  }); s[s.size() - 1] = '}'; return s;
  }//Filter empty std::vector
  template <typename T> std::ostream& operator<<(std::ostream& o, Table<T>* c) {
	std::string s; s << dynamic_cast<T*>(c); return o << s;
  };
  template <typename T> std::ostream& operator<<(std::ostream& o, Table<T>& c) {
	std::string s; s << dynamic_cast<T*>(&c); return o << s;
  };//Compile into most optimized machine code
  template <typename T> std::string& operator<<(std::string& s, std::vector<T> c) {
	s.push_back('['); size_t l = c.size(); if (l > 0) {
	  s << &c[0];
	  for (size_t i = 1; i < l; ++i) { s.push_back(','), s << &c[i]; }
	} s.push_back(']'); return s;
  }
  template <typename T> std::ostream& operator<<(std::ostream& o, std::vector<T> c) {
	o << '['; size_t l = c.size(); if (l > 0) {
	  o << &c[0];
	  for (size_t i = 1; i < l; ++i) { o << ',' << &c[i]; }
	} o << ']'; return o;
  }
  template <typename T> std::string& operator<<(std::string& s, std::vector<T>* c) {
	s.push_back('['); size_t l = c->size(); if (l > 0) {
	  s << &c->at(0);
	  for (size_t i = 1; i < l; ++i) { s.push_back(','), s << &c->at(i); }
	} s.push_back(']'); return s;
  }
  template <typename T> std::ostream& operator<<(std::ostream& o, std::vector<T>* c) {
	o << '['; size_t l = c->size(); if (l > 0) {
	  o << &c->at(0);
	  for (size_t i = 1; i < l; ++i) { o << ',' << &c->at(i); }
	} o << ']'; return o;
  }
}
