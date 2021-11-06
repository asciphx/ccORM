#pragma once
#include "./base/Initalization.hpp"
#include "./base/s2o.hpp"
#include "./base/so2s.hpp"
struct enable_virtual : std::enable_shared_from_this<enable_virtual> { virtual ~enable_virtual() {} };
template<typename T> struct virtual_shared : virtual enable_virtual {
  std::shared_ptr<T> shared_from_this() {
	return std::dynamic_pointer_cast<T>(std::enable_shared_from_this<enable_virtual>::shared_from_this());
  }
};/** multiple inheritance from std::enable_shared_from_this<T> => needed for
struct A: virtual_shared<A> {}; struct B: virtual_shared<B> {}; struct Z: A, B { };
int main() { std::shared_ptr<Z> z = std::make_shared<Z>(); std::shared_ptr<B> b = z->B::shared_from_this(); } */
namespace orm {
  template<typename T> class Table : public virtual_shared<T> {
	static const std::string _name, _drop_; static const uint8_t _size_; const static char* $[];/*Store key name*/
	static bool _create_need; static uint8_t _idex; static std::string _create_; static const size_t _o$[];/*Store offset*/
#ifdef _WIN32
	friend typename T; const static char* _def_[];/*Store default values*/ static uint8_t _tc_[];/*Store type*/
#endif
	friend typename decltype(D)::db_rs; friend class Sql<T>; static const char* _[];/*Store type character*/
	template <typename N> constexpr N& getIdex(size_t i) {
	  return *reinterpret_cast<N*>(reinterpret_cast<char*>(this) + this->_o$[i]);
	}
	template <typename U> void $et(char i, const U* v) {
	  if constexpr (std::is_same<U, const char*>::value) {
		switch (hack4Str(_[i])) {
		case '4tex':
		case 'text':getIdex<text<>>(i) = *v; break;
		case 'NSt7':
		case "class s"_i:getIdex<std::string>(i) = *v;
		}
	  } else getIdex<U>(i) = *v;
	}
	template <typename U> friend std::string& operator<<(std::string& s, Table<U>* c);//Object serialized as string
	template <typename U> friend std::string& operator<<(std::string& s, std::vector<U> c);//vector<Object> serialized as string
	template <typename U> friend std::ostream& operator<<(std::ostream& o, Table<U>* c);
	template <typename U> friend std::ostream& operator<<(std::ostream& o, std::vector<U> c);
  public:
#ifndef _WIN32
	const static char* _def_[]; static uint8_t _tc_[];
#endif
	using ptr = typename std::shared_ptr<T>; static Sql<T>* __[];
	using ptr_arr = typename std::vector<typename std::shared_ptr<T>>;
	Table& operator=(const Table&) = default; static void Init();//Initalization Table
	Table(); ~Table() {} Table(const Table&) = default;
	template<typename ... Args> static ptr create(Args&& ... args);
	template<typename... U> void set(U... t) {
	  static_assert(_size_ >= sizeof...(U));
	  char idex = -1; (void)std::initializer_list<int>{($et(++idex, &t), 0)...};
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
	long long Insert() {
	  int8_t i = -1; std::ostringstream os, ov; ov << "VALUES ("; os << "INSERT INTO " << _name << " (";
	  ForEachField(dynamic_cast<T*>(this), [&i, &os, &ov](auto& t) {
		if (!(_tc_[++i] & (TC::PRIMARY_KEY | TC::AUTO_INCREMENT))) {
		  if constexpr (std::is_fundamental<std::remove_reference_t<decltype(t)>>::value) {
			if constexpr (std::is_same<bool, std::remove_reference_t<decltype(t)>>::value) {
			  ov << t << ','; os << T::$[i] << ',';
			} else { if (*((char*)&t)) { ov << t << ','; os << T::$[i] << ','; } }
		  } else if constexpr (std::is_same<tm, std::remove_reference_t<decltype(t)>>::value) {
			if (*((char*)&t)) { ov << '\'' << t << "',"; os << T::$[i] << ','; }
		  } else if constexpr (std::is_same<std::string, std::remove_reference_t<decltype(t)>>::value) {
			if (*((char*)&t)) { ov << '\'' << toQuotes(t.c_str()) << "',"; os << T::$[i] << ','; }
		  } else if constexpr (is_text<std::remove_reference_t<decltype(t)>>::value) {
			if (*((char*)&t)) { ov << '\'' << toQuotes(t.c_str()) << "',"; os << T::$[i] << ','; }
		  }
		}
		});
	  os.seekp(-1, os.cur); os << ')'; ov.seekp(-1, ov.cur); ov << ")"; os << ' ' << ov.str() << ";";
	  crow::sql_result<decltype(D)::db_rs>&& rs = Q()->Query()(os.str());
	  if (T::_tc_[0] & TC::PRIMARY_KEY) { return rs.last_insert_id(); } else { return 0LL; }
	}
	//Update the object (The default condition is the value of the frist key)
	void Update() {
	  int8_t i = -1; std::ostringstream os; os << "UPDATE " << _name << " SET ";
	  std::string condition(" WHERE "); condition += $[0]; condition.push_back('=');
	  auto& t = dynamic_cast<T*>(this)->*std::get<0>(Schema<T>());
	  if constexpr (std::is_fundamental<std::remove_reference_t<decltype(t)>>::value) {
		condition += std::to_string(t);
	  } else if constexpr (std::is_same<std::string, std::remove_reference_t<decltype(t)>>::value) {
		condition.push_back('\''); condition += toQuotes(t.c_str()); condition.push_back('\'');
	  }
	  ForEachField(dynamic_cast<T*>(this), [&i, &os, &condition](auto& t) {
		if (++i && !(T::_tc_[i] & TC::AUTO_INCREMENT)) {
		  if constexpr (std::is_fundamental<std::remove_reference_t<decltype(t)>>::value) {
			os << T::$[i] << '=' << t << ',';
		  } else if constexpr (std::is_same<tm, std::remove_reference_t<decltype(t)>>::value) {
			os << T::$[i] << '=' << '\'' << t << "',";
		  } else if constexpr (std::is_same<std::string, std::remove_reference_t<decltype(t)>>::value) {
			os << T::$[i] << '=' << '\'' << toQuotes(t.c_str()) << "',";
		  } else if constexpr (is_text<std::remove_reference_t<decltype(t)>>::value) {
			os << T::$[i] << '=' << '\'' << toQuotes(t.c_str()) << "',";
		  } else { return; }
		}
		});
	  os.seekp(-1, os.cur); os << condition << ";";
	  Q()->Query()(os.str());
	}
	//Delete the object based on this object's frist key
	void Delete() {
	  std::ostringstream os; os << "DELETE FROM " << _name << " WHERE " << $[0] << '=';
	  auto& t = dynamic_cast<T*>(this)->*std::get<0>(Schema<T>());
	  if constexpr (std::is_fundamental<std::remove_reference_t<decltype(t)>>::value) {
		os << t;
	  } else if constexpr (std::is_same<std::string, std::remove_reference_t<decltype(t)>>::value) {
		os << '\'' << toQuotes(t.c_str()) << '\'';
	  } else if constexpr (is_text<std::remove_reference_t<decltype(t)>>::value) {
		os << '\'' << toQuotes(t.c_str()) << '\'';
	  } os << ";";
	  Q()->Query()(os.str());
	}
	static void _addTable() {//Mac is temporarily not supported, adaptation type is needed here
	  if (_create_need) {
		_create_need = false;
		for (uint8_t i = 0; i < _size_; ++i) {//St6vectorI4TypeSaIS1_EE(Linux)
		  if (_[i][0] == 'S') { continue; }
		  TC tc = (TC)_tc_[i]; const char* def = _def_[i];
		  if (i)_create_ += ",\n"; _create_ += $[i];
		  if constexpr (std::is_same<decltype(D)::db_tag, crow::pgsql_tag>::value) {
			if (tc & TC::PRIMARY_KEY || tc & TC::AUTO_INCREMENT) {
			  switch (hack4Str(_[i])) {
			  case 'x':
			  case "__int64"_i: _create_ += " BIGSERIAL PRIMARY KEY"; break;
			  case 's':
			  case "short"_i: _create_ += " SMALLSERIAL PRIMARY KEY"; break;
			  case 'i':
			  case 'int': _create_ += " SERIAL PRIMARY KEY"; break;
			  default:;
			  } continue;
			}
		  }
		  switch (hack4Str(_[i])) {
		  case 'd':
		  case 'f':
		  case "double"_i:
		  case "float"_i: _create_ += " REAL"; goto $;
		  case 'x':
		  case "__int64"_i: _create_ += " BIGINT"; break;//BigInt
		  case 'a':
		  case "signed char"_i: if constexpr (std::is_same_v<decltype(D)::db_tag, crow::pgsql_tag>) {
			_create_ += " SMALLINT";
		  } else { _create_ += " TINYINT"; } goto $;
		  case 'b':
		  case 'bool': _create_ += " BOOLEAN"; if (tc & TC::NOT_NULL) { _create_ += " NOT NULL"; }
					 if constexpr (std::is_same_v<decltype(D)::db_tag, crow::pgsql_tag>) { goto $; }
					 if (tc & TC::DEFAULT && so2s<bool>(def)) {
					   _create_ += " DEFAULT "; _create_.push_back('\''); if (def[0] == 't') {
						 _create_.push_back('1');
					   } else { _create_.push_back('0'); } _create_.push_back('\'');
					 } continue;
		  case 's':
		  case "short"_i: _create_ += " SMALLINT"; break;//SmallInt
		  case 'i':
		  case 'int': _create_ += " INTEGER"; break;
		  case '2tm':
		  case "struct tm"_i:  if constexpr (std::is_same_v<decltype(D)::db_tag, crow::pgsql_tag>) {
			_create_ += " timestamp without time zone";
		  } else { _create_ += " DATETIME"; }
		  if (tc & TC::NOT_NULL) { _create_ += " NOT NULL"; }
		  if (tc & TC::DEFAULT) {
			if (so2s<tm>(def)) { _create_ += " DEFAULT '"; _create_ += def; _create_.push_back('\''); continue; }
			if constexpr (std::is_same<decltype(D)::db_tag, crow::sqlite_tag>::value) {
			  _create_ += " DEFAULT (datetime('now','localtime'))";
			} else if constexpr (std::is_same<decltype(D)::db_tag, crow::pgsql_tag>::value) {
			  _create_ += " DEFAULT now()";
			} else { _create_ += " DEFAULT CURRENT_TIMESTAMP"; }
		  } continue;
		  case '4tex': {_create_ += " VARCHAR("; int c = 8; while (_[i][c] < 58)_create_.push_back(_[i][c++]); _create_.push_back(41); } goto $;
		  case 'text': {_create_ += " VARCHAR("; int c = 5; while (_[i][c] < 58)_create_.push_back(_[i][c++]); _create_.push_back(41); } goto $;
		  case 'NSt7':
		  case "class s"_i: _create_ += " TEXT"; goto $;
		  }
		  if constexpr (std::is_same<decltype(D)::db_tag, crow::sqlite_tag>::value) {
			if (tc & TC::PRIMARY_KEY || (tc & TC::PRIMARY_KEY && tc & TC::AUTO_INCREMENT)) { _create_ += " PRIMARY KEY"; }
		  }
		  if constexpr (std::is_same<decltype(D)::db_tag, crow::mysql_tag>::value) {
			if (tc & TC::PRIMARY_KEY) _create_ += " PRIMARY KEY"; if (tc & TC::AUTO_INCREMENT) _create_ += " AUTO_INCREMENT";
		  }
		$://String type detection system
		  if (tc & TC::NOT_NULL) { _create_ += " NOT NULL"; }
		  if (tc & TC::DEFAULT) {
			switch (hack4Str(_[i])) {
			case 'd':
			case "double"_i: if (!so2s<double>(def)) { break; } goto _;
			case 'f':
			case "float"_i: if (!so2s<float>(def)) { break; } goto _;
			case 'x':
			case "__int64"_i: if (!so2s<long long>(def)) { break; } goto _;
			case 'a':
			case "signed char"_i: if (!so2s<signed char>(def)) { break; } goto _;
			case 'b':
			case 'bool': if (!so2s<bool>(def)) { break; } goto _;
			case 's':
			case "short"_i: if (!so2s<short>(def)) { break; } goto _;
			case 'i':
			case 'int': if (!so2s<int>(def)) { break; } _: _create_ += " DEFAULT ";
			  _create_.push_back('\''); _create_ += def; _create_.push_back('\''); break;
			case '4tex':
			case 'text':
			case 'NSt7':
			case "class s"_i:
			default: _create_ += " DEFAULT "; _create_.push_back('\''); _create_ += toQuotes(def); _create_.push_back('\'');
			}
		  }
		} _create_ += "\n);";
	  }
	  auto&& DbQuery = static_cast<Sql<T>*>(__[0])->Query();
	  if constexpr (std::is_same<decltype(D)::db_tag, crow::pgsql_tag>::value) {
		std::string str_("select count(*) from pg_class where relname = '"); str_ += _name; str_ += "';";
		if (DbQuery(str_).template r__<int>() == 0) {
		  DbQuery(_create_).flush_results();
		}
	  } else {
		try {
		  DbQuery(_create_).flush_results();
		} catch (std::runtime_error e) {
		  std::cerr << "\033[1;4;31mWarning:\033[0m could not create the \033[1;34m[" << _name
			<< "]\033[0m table.\nBecause: \033[4;33m" << e.what() << "\033[0m\n";
		}
	  }
	  std::cout << _create_;
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
		s += "\":\""; std::ostringstream os; const tm* time = &t;
		os << 20 << (time->tm_year - 100) << '-' << std::setfill('0')
		  << std::setw(2) << (time->tm_mon + 1) << '-' << std::setw(2) << time->tm_mday << ' '
		  << std::setw(2) << time->tm_hour << ':' << std::setw(2) << time->tm_min << ':'
		  << std::setw(2) << time->tm_sec << '"'; s += os.str();
	  } else if constexpr (std::is_same<signed char, std::remove_reference_t<decltype(t)>>::value) {
		s += "\":" + std::to_string(t);
	  } else if constexpr (std::is_same<double, std::remove_reference_t<decltype(t)>>::value) {
		s += "\":" + std::to_string(t);
	  } else if constexpr (std::is_same<float, std::remove_reference_t<decltype(t)>>::value) {
		s += "\":" + std::to_string(t);
	  } else if constexpr (std::is_same<bool, std::remove_reference_t<decltype(t)>>::value) {
		s += "\":", s += t == true ? "true" : "false";
	  } else if constexpr (std::is_same<short, std::remove_reference_t<decltype(t)>>::value) {
		s += "\":" + std::to_string(t);
	  } else if constexpr (std::is_same<int, std::remove_reference_t<decltype(t)>>::value) {
		s += "\":" + std::to_string(t);
	  } else if constexpr (std::is_same<long long, std::remove_reference_t<decltype(t)>>::value) {
		s += "\":" + std::to_string(t);
	  } else if constexpr (std::is_same<long, std::remove_reference_t<decltype(t)>>::value) {
		s += "\":" + std::to_string(t);
	  } else if constexpr (std::is_same<string, std::remove_reference_t<decltype(t)>>::value) {
		s += "\":\"" + t + "\"";
	  } else {
		s += "\":"; s << t;
	  } s.push_back(',');
	  }); s[s.size() - 1] = '}'; return s;
  }//Compile into most optimized machine code
  template <typename T> std::string& operator<<(std::string& s, std::vector<T> c) {
	s.push_back('['); size_t l = c.size();
	for (size_t i = 0; i < l; ++i) { s << &c[i], s.push_back(','); }
	l == 0 ? s.push_back(']') : s.pop_back(), s.push_back(']'); return s;
  }
  template <typename T> std::ostream& operator<<(std::ostream& o, Table<T>* c) {
	std::string s; s << dynamic_cast<T*>(c); return o << s;
  };
  template <typename T> std::ostream& operator<<(std::ostream& o, std::vector<T> c) {
	o << '['; size_t l = c.size();
	if (l > 0) { o << &c[0]; }
	for (size_t i = 1; i < l; ++i) { o << ',' << &c[i]; }
	o << ']'; return o;
  }
}
