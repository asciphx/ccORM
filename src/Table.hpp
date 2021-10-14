#pragma once
#include "./base/Initalization.hpp"
#include "./base/s2o.hpp"
#include "./base/so2s.hpp"
//#pragma warning(disable:4715)
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
	const static char* _name, * _drop_, * $[]; static const uint8_t _size_;
	static bool _create_need; static uint8_t _idex; static std::string _create_;
#ifdef _WIN32
	friend typename T; const static char* _def_[]; static uint8_t _tc_[];
#endif
	static const char* _T_[]; static const size_t _[];
	template <typename N> constexpr N& getIdex(size_t i) {
	  return *reinterpret_cast<N*>(reinterpret_cast<char*>(this) + this->_[i]);
	}
	template <typename U> void $et(uint8_t idex, const U val) {
	  if constexpr (std::is_same<U, tm>::value)	getIdex<tm>(idex) = val;
	  else if constexpr (std::is_same<U, float>::value)	getIdex<float>(idex) = val;
	  else if constexpr (std::is_same<U, double>::value) getIdex<double>(idex) = val;
	  else if constexpr (std::is_same<U, bool>::value) getIdex<bool>(idex) = val;
	  else if constexpr (std::is_same<U, short>::value)	getIdex<short>(idex) = val;
	  else if constexpr (std::is_same<U, signed char>::value) getIdex<signed char>(idex) = val;
	  else if constexpr (std::is_same<U, unsigned int>::value) getIdex<unsigned int>(idex) = val;
	  else if constexpr (std::is_same<U, int>::value) getIdex<int>(idex) = val;
	  else if constexpr (std::is_same<U, long long>::value) getIdex<long long>(idex) = val;
	  else if constexpr (std::is_same<U, std::string>::value) getIdex<std::string>(idex) = val;
	  else if constexpr (std::is_same<U, const char*>::value) getIdex<std::string>(idex) = val;
	}
	template <typename U> friend std::string& operator<<(std::string& s, Table<U>* c);
	template <typename U> friend std::string& operator<<(std::string& s, std::vector<U> c);
	template <typename U> friend std::ostream& operator<<(std::ostream& o, Table<U>* c);
	template <typename U> friend std::ostream& operator<<(std::ostream& o, std::vector<U> c);
  public:
#ifndef _WIN32
	const static char* _def_[]; static uint8_t _tc_[];
#endif
	using ptr = typename std::shared_ptr<T>; static Sql<T>* QB[];//QueryBuilder
	using ptr_arr = typename std::vector<typename std::shared_ptr<T>>;
	Table& operator=(const Table&) = default;
	Table(); ~Table() {} //Table(const Table&) = delete;
	template<typename ... Args> static ptr create(Args&& ... args);
	ptr _asPointer() { return this->shared_from_this(); }
	template<typename... U> void set(U&&... t) {
	  assert(_size_ >= sizeof...(U));
	  uint8_t colBuf = 0; (void)std::initializer_list<int>{($et(colBuf++, std::forward<U>(t)), void(), 0)...};
	}
	static Sql<T>* Q() {
	_: if (++_idex > HARDWARE_CORE) _idex = 0; Sql<T>* q = QB[_idex];
	  if (q->prepare_) { q->prepare_ = false; return q; }
	  std::this_thread::yield(); goto _;
	};
	json get() { return json(*dynamic_cast<T*>(this)); };
	static void _add() {
	  if (_create_need) {
		_create_need = false;
		for (uint8_t i = 0; i < _size_; ++i) {//St6vectorI4TypeSaIS1_EE
		  if (_T_[i][0] == 'v' || _T_[i][3] == 'v') continue; TC tc = (TC)_tc_[i]; const char* def = _def_[i];
		  if (i)_create_ += ",\n"; _create_ += $[i];
		  if constexpr (std::is_same<decltype(D)::db_tag, crow::pgsql_tag>::value) {
			if (is_PRIMARY_KEY(tc) || is_AUTOINCREMENT(tc)) {
			  switch (hack4Str(_T_[i])) {
			  case "x"_i:
			  case "__int64"_i: _create_ += " BIGSERIAL PRIMARY KEY"; break;
			  case "s"_i:
			  case "short"_i: _create_ += " SMALLSERIAL PRIMARY KEY"; break;
			  case "i"_i:
			  case "int"_i: _create_ += " SERIAL PRIMARY KEY"; break;
			  } continue;
			}
		  }
		  switch (hack4Str(_T_[i])) {
		  case "d"_i:
		  case "f"_i:
		  case "double"_i:
		  case "float"_i: _create_ += " REAL"; goto $;
		  case "x"_i:
		  case "__int64"_i: _create_ += " BIGINT"; break;//BigInt
		  case "a"_i:
		  case "signed char"_i: if constexpr (std::is_same_v<decltype(D)::db_tag, crow::pgsql_tag>) {
			_create_ += " SMALLINT";
		  } else { _create_ += " TINYINT"; } goto $;
		  case "b"_i:
		  case "bool"_i: _create_ += " BOOLEAN"; if (is_NOT_NULL(tc)) { _create_ += " NOT NULL"; }
					   if constexpr (std::is_same_v<decltype(D)::db_tag, crow::sqlite_tag>) {
						 if (is_DEFAULT(tc) && so2s<bool>(def)) {
						   _create_ += " DEFAULT "; _create_.push_back('\''); if (def[0] == 't') {
							 _create_.push_back('1');
						   } else { _create_.push_back('0'); } _create_.push_back('\'');
						 } continue;
					   } else { goto $; }
		  case "s"_i:
		  case "short"_i: _create_ += " SMALLINT"; break;//SmallInt
		  case "i"_i:
		  case "int"_i: _create_ += " INTEGER"; break;
		  case "2tm"_i:
		  case "struct tm"_i:  if constexpr (std::is_same_v<decltype(D)::db_tag, crow::pgsql_tag>) {
			_create_ += " timestamp without time zone";
		  } else { _create_ += " DATETIME"; }
		  if (is_NOT_NULL(tc)) { _create_ += " NOT NULL"; }
		  if (is_DEFAULT(tc)) {
			if (so2s<tm>(def)) { _create_ += " DEFAULT '"; _create_ += def; _create_.push_back('\''); continue; }
			if constexpr (std::is_same<decltype(D)::db_tag, crow::sqlite_tag>::value) {
			  _create_ += " DEFAULT (datetime('now','localtime'))";
			} else if constexpr (std::is_same<decltype(D)::db_tag, crow::pgsql_tag>::value) {
			  _create_ += " DEFAULT now()";
			} else { _create_ += " DEFAULT CURRENT_TIMESTAMP"; }
		  } continue;
		  case "NSt7"_i:
		  case "class s"_i: _create_ += " VARCHAR(255)"; goto $;
		  }
		  if constexpr (std::is_same<decltype(D)::db_tag, crow::sqlite_tag>::value) {
			if (is_PRIMARY_KEY(tc) || is_AUTOINCREMENT(tc)) { _create_ += " PRIMARY KEY"; }
		  }
		  if constexpr (std::is_same<decltype(D)::db_tag, crow::mysql_tag>::value) {
			if (is_PRIMARY_KEY(tc))  _create_ += " PRIMARY KEY"; if (is_AUTOINCREMENT(tc))_create_ += " AUTO_INCREMENT";
		  }
		$:
		  if (is_NOT_NULL(tc)) { _create_ += " NOT NULL"; }
		  if (is_DEFAULT(tc)) {
			switch (hack4Str(_T_[i])) {
			case "d"_i:
			case "double"_i: if (!so2s<double>(def)) { break; }
			case "f"_i:
			case "float"_i: if (!so2s<float>(def)) { break; }
			case "x"_i:
			case "__int64"_i: if (!so2s<long long>(def)) { break; }
			case "a"_i:
			case "signed char"_i: if (!so2s<signed char>(def)) { break; }
			case "s"_i:
			case "short"_i: if (!so2s<short>(def)) { break; }
			case "i"_i:
			case "int"_i: if (!so2s<int>(def)) { break; }
			case "NSt7"_i:
			case "class s"_i: if (!so2s<std::string>(def)) { break; }
			default: _create_ += " DEFAULT "; _create_.push_back('\''); _create_ += def; _create_.push_back('\'');
			}
		  }
		} _create_ += "\n);";
	  }
	  auto DbQuery = static_cast<Sql<T>*>(QB[0])->Query();
	  if constexpr (std::is_same<decltype(D)::db_tag, crow::pgsql_tag>::value) {
		std::string str_("select count(*) from pg_class where relname = '"); str_ += toSqlLowerCase(_name); str_ += "';";
		if (DbQuery(str_).template r__<int>() == 0) {
		  DbQuery(_create_.c_str()).flush_results();
		}
	  } else {
		try {
		  DbQuery(_create_.c_str()).flush_results();
		} catch (std::runtime_error e) {
		  std::cerr << "Warning: Lithium::sql could not create the " << static_cast<Sql<T>*>(QB[0])->table_ << " sql table.\n"
			<< "You can ignore this message if the table already exists." << "The sql error is: " << e.what() << std::endl;
		}
	  }
	  //std::cout << _create_.c_str();
	}
	static void _drop() { auto DbQuery = static_cast<Sql<T>*>(QB[0])->Query(); DbQuery(_drop_).flush_results(); }
  };
  template<typename T> Table<T>::Table() {}
  template<typename T> template<typename ... Args>
  typename Table<T>::ptr Table<T>::create(Args&& ... args) { return std::make_shared<T>(std::forward<Args>(args)...); }
  template <typename T> std::string& operator<<(std::string& s, Table<T>* c) {
	s.push_back('{'); bool b = true;
	ForEachField(dynamic_cast<T*>(c), [&s, &b](auto& t, auto& k) {
	  if (b) { s.push_back('\"'), b = false; } else { s += ",\""; } s += k;
	  if constexpr (std::is_same<tm, std::remove_reference_t<decltype(t)>>::value) {
		std::ostringstream os; const tm* time = &t;
		os << std::setfill('0') << std::setw(4) << (time->tm_year + 1900) << '-'
		  << std::setw(2) << (time->tm_mon + 1) << '-' << std::setw(2) << time->tm_mday << ' '
		  << std::setw(2) << time->tm_hour << ':' << std::setw(2) << time->tm_min << ':'
		  << std::setw(2) << time->tm_sec << '"'; s += "\":\"";
#ifdef _WIN32
		s += os.str();
#else
		s += os.str().replace(1, 1, "");
#endif
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
	  }
	  }); s.push_back('}'); return s;
  }
  template <typename T> std::string& operator<<(std::string& s, std::vector<T> c) {
	s.push_back('['); bool b = true;
	for (auto t : c) {
	  if (b) s << &t, b = false; else s.push_back(','), s << &t;
	}
	s.push_back(']'); return s;
  }
  template <typename T> std::ostream& operator<<(std::ostream& o, Table<T>* c) {
	std::string s; s << dynamic_cast<T*>(c); return o << s;
  };
  template <typename T> std::ostream& operator<<(std::ostream& o, std::vector<T> c) {
	o << '['; bool b = true;
	for (auto t : c) {
	  if (b) o << &t, b = false; else o << ',' << &t;
	} o << ']'; return o;
  }
}
