#pragma once
#include "./base/Initalization.hpp"
#include "./base/s2o.hpp"
#include "./base/so2s.hpp"
#define Struct(T) struct T : orm::Table<T> /*multiple inheritance from std::enable_shared_from_this<T> => needed for
struct A: virtual_shared<A> {}; struct B: virtual_shared<B> {}; struct Z: A, B { };*/
namespace orm {
  struct enable_virtual : std::enable_shared_from_this<enable_virtual> { virtual ~enable_virtual() {} };
  template<typename T> struct virtual_shared : virtual enable_virtual {
	std::shared_ptr<T> shared_from_this() {
	  return std::dynamic_pointer_cast<T>(std::enable_shared_from_this<enable_virtual>::shared_from_this());
	}
  };/*int main() { std::shared_ptr<Z> z = std::make_shared<Z>(); std::shared_ptr<B> b = z->B::shared_from_this(); } */
  template<typename T> class Table : public virtual_shared<T> { /*Store (T.key,)... name*/ /*Store key name[]*/ /*Store _alias*/
	static const std::string _name, _drop_; static const uint8_t _size_; const static char* _ios_, * $[], * _alias;//^
	static bool _created; static unsigned char _idex; static std::string _create_; static const size_t _o$[];/*Store offset[]*/
#ifdef _WIN32
	friend typename T; const static char* _def_[];/*Store default values[]*/static unsigned char _tc_[];/*Store key type[]*/
#endif
	friend typename decltype(D)::db_rs; friend struct Sql<T>; static const char* _[];/*Store type character[]*/
	template <typename U> void $et(int8_t i, const U* v) {
	  if constexpr (std::is_same<U, const char*>::value) {
		switch (hack8Str(_[i])) {
		case "class text"_l:
		case "4textILt"_l:*reinterpret_cast<text<>*>(reinterpret_cast<char*>(this) + this->_o$[i]) = *v; break;
		case "class std::basic_string"_l:
		case "NSt7__cx"_l:*reinterpret_cast<std::string*>(reinterpret_cast<char*>(this) + this->_o$[i]) = *v;
		}
	  } else *reinterpret_cast<U*>(reinterpret_cast<char*>(this) + this->_o$[i]) = *v;
	}
	template <typename U> friend std::string& operator<<(std::string& s, Table<U>* c);//Object serialized as string
	template <typename U> friend std::string& operator<<(std::string& s, std::vector<U> c);//vector<Object> serialized as string
	template <typename U> friend std::ostream& operator<<(std::ostream& o, Table<U>* c);
	template <typename U> friend std::ostream& operator<<(std::ostream& o, std::vector<U> c);
  public:
#ifndef _WIN32
	const static char* _def_[]; static unsigned char _tc_[];
#endif
	using ptr = typename std::shared_ptr<T>; static Sql<T>* __[];
	using ptr_arr = typename std::vector<typename std::shared_ptr<T>>;
	Table& operator=(const Table&) = default; static void Init();//Initalization Table
	Table(); ~Table() {} Table(const Table&) = default;
	template<typename ... Args> static ptr create(Args&& ... args);
	template<typename... U> void set(U... t) {
	  int8_t idex = -1; (void)std::initializer_list<int>{($et(++idex, &t), 0)...};
	  return; /*This code will never arrive*/ *this = T(t...);//detect types
	}
	//Query builder
	static Sql<T>* Q() {
	_: if (++_idex > HARDWARE_CORE) _idex = 0; Sql<T>* q = __[_idex];
	  if (q->prepare_) { q->prepare_ = false; return q; } std::this_thread::yield(); goto _;
	};
	//Object serialized as JSON
	json get() { return json(*dynamic_cast<T*>(this)); };
	//-------------------------------------ActiveRecord-------------------------------------
	//Insert the object (Returns the inserted ID)
	auto Insert() {
	  int8_t i = -1; std::ostringstream os, ov; ov << "VALUES ("; os << "INSERT INTO " << _name << " (";
	  ForEachField(dynamic_cast<T*>(this), [&i, &os, &ov](auto& t) {
		if (!(_tc_[++i] & (TC::PRIMARY_KEY | TC::AUTO_INCREMENT))) {
		  if constexpr (std::is_same<bool, std::remove_reference_t<decltype(t)>>::value) {
			if constexpr (ce_is_pgsql) { ov << (t ? "true" : "false") << ','; } else { ov << t << ','; } os << T::$[i] << ',';
		  } else if constexpr (std::is_fundamental<std::remove_reference_t<decltype(t)>>::value) {
			if (*((char*)&t)) { ov << t << ','; os << T::$[i] << ','; }
		  } else if constexpr (std::is_same<tm, std::remove_reference_t<decltype(t)>>::value) {
			if (*((char*)&t)) { ov << '\'' << t << "',"; os << T::$[i] << ','; }
		  } else if constexpr (std::is_same<std::string, std::remove_reference_t<decltype(t)>>::value) {
			if (*((char*)&t)) { ov << '\'' << toQuotes(t.c_str()) << "',"; os << T::$[i] << ','; }
		  } else if constexpr (is_text<std::remove_reference_t<decltype(t)>>::value) {
			if (*((char*)&t)) { ov << '\'' << toQuotes(t.c_str()) << "',"; os << T::$[i] << ','; }
		  }
		}
		});
	  os.seekp(-1, os.cur); os << ')'; ov.seekp(-1, ov.cur); ov << ")"; os << ' ' << ov.str();
	  if constexpr (ce_is_pgsql) { os << " RETURNING " << T::$[0] << ";"; } else { os << ";"; }
	  crow::sql_result<decltype(D)::db_rs>&& rs = Q()->Query()(os.str());
	  if (T::_tc_[0] & TC::PRIMARY_KEY) { return rs.last_insert_id(); } else if constexpr (ce_is_mysql) { return 0ULL; } else { return 0LL; }
	}
	//Update the object (The default condition is the value of the frist key)
	void Update() {
	  int8_t i = -1; std::ostringstream os; os << "UPDATE " << _name << " SET ";
	  std::string condition(" WHERE "); condition += $[0]; condition.push_back('=');
	  auto& t = dynamic_cast<T*>(this)->*std::get<0>(Schema<T>());
	  if constexpr (std::is_fundamental<std::remove_reference_t<decltype(t)>>::value) {
		condition += std::to_string(t);
	  } else if constexpr (std::is_same<tm, std::remove_reference_t<decltype(t)>>::value) {
		condition.push_back('\''); condition << t; condition.push_back('\'');
	  } else if constexpr (std::is_same<std::string, std::remove_reference_t<decltype(t)>>::value) {
		condition.push_back('\''); condition += toQuotes(t.c_str()); condition.push_back('\'');
	  } else if constexpr (is_text<std::remove_reference_t<decltype(t)>>::value) {
		condition.push_back('\''); condition += toQuotes(t.c_str()); condition.push_back('\'');
	  }
	  ForEachField(dynamic_cast<T*>(this), [&i, &os, &condition](auto& t) {
		if (++i && !(T::_tc_[i] & TC::AUTO_INCREMENT)) {
		  if constexpr (std::is_same<bool, std::remove_reference_t<decltype(t)>>::value) {
			if constexpr (ce_is_pgsql) {
			  os << T::$[i] << '=' << (t ? "true" : "false") << ',';
			} else { os << T::$[i] << '=' << t << ','; }
		  } else if constexpr (std::is_fundamental<std::remove_reference_t<decltype(t)>>::value) {
			os << T::$[i] << '=' << t << ',';
		  } else if constexpr (std::is_same<tm, std::remove_reference_t<decltype(t)>>::value) {
			os << T::$[i] << '=' << '\'' << t << "',";
		  } else if constexpr (std::is_same<std::string, std::remove_reference_t<decltype(t)>>::value) {
			os << T::$[i] << '=' << '\'' << toQuotes(t.c_str()) << "',";
		  } else if constexpr (is_text<std::remove_reference_t<decltype(t)>>::value) {
			os << T::$[i] << '=' << '\'' << toQuotes(t.c_str()) << "',";
		  }
		}
		});
	  os.seekp(-1, os.cur); os << condition << ";";
	  Q()->Query()(os.str());
	}
	//Delete the object based on this object's frist key
	void Delete() {
	  std::ostringstream os; os << "DELETE FROM " << _name << " WHERE " << $[0] << '=';
	  auto& t = dynamic_cast<T*>(this)->*std::get<0>(Schema<T>());
	  if constexpr (std::is_same<bool, std::remove_reference_t<decltype(t)>>::value) {
		if constexpr (ce_is_pgsql) { os << (t ? "true" : "false"); } else { os << t; }
	  } else if constexpr (std::is_fundamental<std::remove_reference_t<decltype(t)>>::value) {
		os << t;
	  } else if constexpr (std::is_same<tm, std::remove_reference_t<decltype(t)>>::value) {
		os << '\'' << t << '\'';
	  } else if constexpr (std::is_same<std::string, std::remove_reference_t<decltype(t)>>::value) {
		os << '\'' << toQuotes(t.c_str()) << '\'';
	  } else if constexpr (is_text<std::remove_reference_t<decltype(t)>>::value) {
		os << '\'' << toQuotes(t.c_str()) << '\'';
	  } os << ";";
	  Q()->Query()(os.str());
	}
	static void _addTable() {//Mac is temporarily not supported, adaptation type is needed here
	  if (_created) {
		_created = false;
		if (_tc_[0] & TC::PRIMARY_KEY || _tc_[0] & TC::AUTO_INCREMENT) {//check sequence key, and it must be number
		  switch (hack8Str(_[0])) {
		  case "signed char"_l: case 'a': case "short"_l: case 's': case 'int': case 'i': case "__int64"_l: case 'x':
		  case "d char"_l: case 'h': case "d short"_l: case 't': case "d int"_l: case 'j': case "d __int64"_l: case 'y':
		  break; default:throw std::runtime_error("\033[1;34m[sequence]\033[31;4m type must be number!\n\033[0m");
		  }
		}
		for (uint8_t i = 0; i < _size_; ++i) {//St6vectorI4TypeSaIS1_EE(Linux)
		  if (_[i][0] == 'S') { continue; }
		  TC tc = (TC)_tc_[i]; const char* def = _def_[i];
		  if (i)_create_ += ",\n";
		  if constexpr (ce_is_pgsql) {
			_create_.push_back('"'); _create_ += $[i]; _create_.push_back('"');
			if (tc & TC::PRIMARY_KEY || tc & TC::AUTO_INCREMENT) { //Compatible layer, unsigned field may be only half the size
			  switch (hack8Str(_[i])) {
			  case "signed char"_l: case 'a':
			  case "d char"_l: case 'h':
			  case "short"_l: case 's':
			  case "d short"_l: case 't': _create_ += " SMALLSERIAL PRIMARY KEY"; break;
			  case 'int': case 'i':
			  case "d int"_l: case 'j': _create_ += " SERIAL PRIMARY KEY"; break;
			  case "__int64"_l: case 'x':
			  case "d __int64"_l: case 'y': _create_ += " BIGSERIAL PRIMARY KEY"; break;
			  } continue;
			}
		  } else if constexpr (ce_is_sqlite) {//AUTOINCREMENT is only allowed on an INTEGER PRIMARY KEY
			_create_.push_back('`'); _create_ += $[i]; _create_.push_back('`');
			if (tc & TC::PRIMARY_KEY && tc & TC::AUTO_INCREMENT) { _create_ += " INTEGER PRIMARY KEY AUTOINCREMENT"; continue; }
		  } else { _create_.push_back('`'); _create_ += $[i]; _create_.push_back('`'); }
		  switch (hack8Str(_[i])) {
		  case 'bool':
		  case 'b': _create_ += " BOOLEAN"; if (tc & TC::NOT_NULL) { _create_ += " NOT NULL"; }
				  if constexpr (ce_is_pgsql) { goto $; }
				  if (tc & TC::DEFAULT && so2s<bool>(def)) {
					_create_ += " DEFAULT "; _create_.push_back('\''); if (def[0] == 't') {
					  _create_.push_back('1');
					} else { _create_.push_back('0'); } _create_.push_back('\'');
				  } continue;
		  case "double"_l://or DECIMAL(16,6), Because the decimal places of double are only 6 in C++
		  case 'd': if constexpr (ce_is_pgsql) { _create_ += " DOUBLE PRECISION"; } else { _create_ += " DOUBLE"; } goto $;
		  case "float"_l:
		  case 'f': _create_ += " REAL"; goto $;
		  case "signed char"_l:
		  case 'a': _create_ += " TINYINT"; break;
		  case "short"_l:
		  case 's': _create_ += " SMALLINT"; break;
		  case 'int':
		  case 'i': _create_ += " INTEGER"; if constexpr (ce_is_sqlite) { if (tc & TC::AUTO_INCREMENT) _create_ += " AUTOINCREMENT"; } break;
		  case "__int64"_l:
		  case 'x': _create_ += " BIGINT"; break;
		  case "struct tm"_l:
		  case '2tm': if constexpr (ce_is_pgsql) {
			_create_ += " timestamp without time zone";
		  } else { _create_ += " DATETIME"; }
		  if (tc & TC::NOT_NULL) { _create_ += " NOT NULL"; }
		  if (tc & TC::DEFAULT) {
			if (so2s<tm>(def)) { _create_ += " DEFAULT '"; _create_ += def; _create_.push_back('\''); continue; }
			if constexpr (ce_is_sqlite) {
			  _create_ += " DEFAULT (datetime('now','localtime'))";
			} else if constexpr (ce_is_pgsql) {
			  _create_ += " DEFAULT now()";
			} else { _create_ += " DEFAULT CURRENT_TIMESTAMP"; }
		  } continue;
		  case "class text"_l: {_create_ += " VARCHAR("; int c = 0xb; while (_[i][c] < 58)_create_.push_back(_[i][c++]); _create_.push_back(41); } goto $;
		  case "4textILt"_l: {_create_ += " VARCHAR("; int c = 8; while (_[i][c] < 58)_create_.push_back(_[i][c++]); _create_.push_back(41); } goto $;
		  case "class std::basic_string"_l:
		  case "NSt7__cx"_l: _create_ += " TEXT"; goto $;
		  case "d char"_l://uint8_t,sqlite not support UNSIGNED, PgSQL can use CREATE DOMAIN
		  case 'h': if constexpr (ce_is_pgsql) {
			_create_ += " UNSIGNED_TINYINT";
		  } else if constexpr (ce_is_mysql) { _create_ += " TINYINT UNSIGNED"; } else { _create_ += " TINYINT"; } break;
		  case "d short"_l:
		  case 't': if constexpr (ce_is_pgsql) {
			_create_ += " UNSIGNED_SMALLINT";
		  } else if constexpr (ce_is_mysql) { _create_ += " SMALLINT UNSIGNED"; } else { _create_ += " SMALLINT"; } break;
		  case "d int"_l:
		  case 'j': if constexpr (ce_is_pgsql) {
			_create_ += " UNSIGNED_INTEGER";
		  } else if constexpr (ce_is_mysql) { _create_ += " INTEGER UNSIGNED"; } else { _create_ += " INTEGER"; } break;
		  case "d __int64"_l://PgSQL,sqlite may be difficult to handle
		  case 'y': if constexpr (ce_is_pgsql) {
			_create_ += " UNSIGNED_BIGINT";
		  } else if constexpr (ce_is_mysql) { _create_ += " BIGINT UNSIGNED"; } else { _create_ += " BIGINT"; } break;
		  }
		  if constexpr (ce_is_sqlite) {
			if (tc & TC::PRIMARY_KEY) _create_ += " PRIMARY KEY";
		  }
		  if constexpr (ce_is_mysql) {
			if (tc & TC::PRIMARY_KEY) _create_ += " PRIMARY KEY"; if (tc & TC::AUTO_INCREMENT) _create_ += " AUTO_INCREMENT";
		  }
		$://String type detection system => unsigned -> d
		  if (tc & TC::NOT_NULL) { _create_ += " NOT NULL"; }
		  if (tc & TC::DEFAULT) {
			switch (hack8Str(_[i])) {
			case "double"_l:
			case 'd': if (!so2s<double>(def)) { break; } goto _;
			case "float"_l:
			case 'f': if (!so2s<float>(def)) { break; } goto _;
			case "__int64"_l:
			case 'x': if (!so2s<long long>(def)) { break; } goto _;
			case "signed char"_l:
			case 'a': if (!so2s<int8_t>(def)) { break; } goto _;
			case 'bool':
			case 'b': if (!so2s<bool>(def)) { break; } goto _;
			case "short"_l:
			case 's': if (!so2s<short>(def)) { break; } goto _;
			case 'int':
			case 'i': if (!so2s<int>(def)) { break; } _: _create_ += " DEFAULT ";
			  _create_.push_back('\''); _create_ += def; _create_.push_back('\''); break;
			case "class text"_l:
			case "4textILt"_l://maybe check length
			case "class std::basic_string"_l:
			case "NSt7__cx"_l: _create_ += " DEFAULT "; _create_.push_back('\''); _create_ += toQuotes(def); _create_.push_back('\''); break;
			case "d char"_l://not check UNSIGNED, may need to be improved
			case 'h':
			case "d short"_l:
			case 't':
			case "d int"_l:
			case 'j':
			case "d __int64"_l:
			case 'y': goto _;
			}
		  }
		} _create_ += "\n);";
	  }
	  auto&& DbQuery = static_cast<Sql<T>*>(__[0])->Query();
	  std::cout << _create_;
	  if constexpr (ce_is_pgsql) {
		std::string str_("select count(*) from pg_class where relname = '"); str_ += _name; str_ += "';";
		if (DbQuery(str_).template r__<int>() == 0) {
		  DbQuery(_create_).flush_results();
		}// else { DbQuery(_drop_); DbQuery(_create_).flush_results(); }
	  } else {
		try {
		  DbQuery(_create_).flush_results();
		} catch (std::runtime_error e) {
		  std::cerr << "\033[1;4;31mWarning:\033[0m could not create the \033[1;34m[" << _name
			<< "]\033[0m table.\nBecause: \033[4;33m" << e.what() << "\033[0m\n";
		}
	  }
	}
	static void _dropTable() { static_cast<Sql<T>*>(__[0])->Query()(_drop_).flush_results(); }
  };
  template<typename T> Table<T>::Table() {}
  template<typename T> template<typename ... Args>
  typename Table<T>::ptr Table<T>::create(Args&& ... args) { return std::make_shared<T>(std::forward<Args>(args)...); }
  template <typename T> std::string& operator<<(std::string& s, Table<T>* c) {
	s.push_back('{'); int8_t i = -1;
	ForEachField(dynamic_cast<T*>(c), [&i, c, &s](auto& t) {
	  s.push_back('"'); s += c->$[++i];
	  if constexpr (std::is_same<tm, std::remove_reference_t<decltype(t)>>::value) {
		s += "\":\""; std::ostringstream os; const tm* time = &t; os << std::setfill('0');
#ifdef _WIN32
		os << std::setw(4) << time->tm_year + 1900;
#else
		int y = time->tm_year / 100; os << std::setw(2) << 19 + y << std::setw(2) << time->tm_year - y * 100;
#endif
		os << '-' << std::setw(2) << (time->tm_mon + 1) << '-' << std::setw(2) << time->tm_mday << ' ' << std::setw(2)
		  << time->tm_hour << ':' << std::setw(2) << time->tm_min << ':' << std::setw(2) << time->tm_sec << '"'; s += os.str();
	  } else if constexpr (std::is_same<bool, std::remove_reference_t<decltype(t)>>::value) {
		s += "\":", s += t == true ? "true" : "false";
	  } else if constexpr (std::is_fundamental<std::remove_reference_t<decltype(t)>>::value) {
		s += "\":" + std::to_string(t);
	  } else if constexpr (std::is_same<std::string, std::remove_reference_t<decltype(t)>>::value) {
		s += "\":\"" + t + "\"";
	  } else {
		s += "\":"; s << t;
	  } s.push_back(',');
	  }); s[s.size() - 1] = '}'; return s;
  }//Compile into most optimized machine code
  template <typename T> std::string& operator<<(std::string& s, std::vector<T> c) {
	s.push_back('['); size_t l = c.size(); if (l > 0) { s << &c[0]; }
	for (size_t i = 1; i < l; ++i) { s.push_back(','), s << &c[i]; }
	s.push_back(']'); return s;
  }
  template <typename T> std::ostream& operator<<(std::ostream& o, Table<T>* c) {
	std::string s; s << dynamic_cast<T*>(c); return o << s;
  };
  template <typename T> std::ostream& operator<<(std::ostream& o, std::vector<T> c) {
	o << '['; size_t l = c.size(); if (l > 0) { o << &c[0]; }
	for (size_t i = 1; i < l; ++i) { o << ',' << &c[i]; }
	o << ']'; return o;
  }
}
