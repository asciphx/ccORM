#pragma once
namespace orm {
  static tm RES_NOW;
  static auto RES_last = std::chrono::steady_clock::now();
  static tm& now() {
	if (std::chrono::steady_clock::now() - RES_last < std::chrono::seconds(1)) return RES_NOW;
	time_t rawtime; std::time(&rawtime);
#if defined(_MSC_VER) || defined(__MINGW32__)
	localtime_s(&RES_NOW, &rawtime);
#else
	localtime_r(&rawtime, &RES_NOW);
#endif
	RES_last = std::chrono::steady_clock::now();
	return RES_NOW;
  }
  template<typename T>
  void regist() {
	unsigned int i = HARDWARE_ASYNCHRONOUS; while (i--) { orm::Table<T>::__[i] = new Sql<T>(); }
	try {
	  T::Init();//Build and check whether the table creation statement of table is correct
	  T::_addTable();//Create table
	} catch (const std::exception& e) {
	  std::cerr << e.what(); return;
	}
  }
  template <typename ...Args>//Create Center
  static void InitializationOrm() {
	time_t RES_ti; std::time(&RES_ti);
#if defined(_MSC_VER) || defined(__MINGW32__)
	system("chcp 65001");
	localtime_s(&RES_NOW, &RES_ti);
#else
	localtime_r(&RES_ti, &RES_NOW);
	std::locale::global(std::locale("en_US.UTF8"));
#endif
	if (ce_is_pgsql) {
	  auto&& DbQuery = D.conn();//auto support tinyint and unsigned fields.
	  if (DbQuery("SELECT count(*) FROM pg_type WHERE typname = 'tinyint';").template r__<int>() == 0) {
		DbQuery("CREATE DOMAIN TINYINT AS int2 CHECK(VALUE > -128 AND VALUE < 128);");
		DbQuery("CREATE DOMAIN UNSIGNED_TINYINT AS int2 CHECK(VALUE >= 0 AND VALUE < 256);");
		DbQuery("CREATE DOMAIN UNSIGNED_SMALLINT AS int4 CHECK(VALUE >= 0 AND VALUE < 65536);");
		DbQuery("CREATE DOMAIN UNSIGNED_INTEGER AS int8 CHECK(VALUE >= 0 AND VALUE < 4294967296);");
		DbQuery("CREATE DOMAIN UNSIGNED_BIGINT AS int8 CHECK(VALUE >= 0 AND VALUE <= 18446744073709551615);").flush_results();
	  }
	}
	(void)std::initializer_list<int>{(regist<Args>(), 0)...};
  }
  static std::string Time2Str(const time_t* t) {
	tm* _v; _v = std::localtime(t); std::ostringstream os; os << std::setfill('0');
#ifdef _WIN32
	os << std::setw(4) << _v->tm_year + 1900;
#else
	int y = _v->tm_year / 100; os << std::setw(2) << 19 + y << std::setw(2) << _v->tm_year - y * 100;
#endif
	os << '-' << std::setw(2) << (_v->tm_mon + 1) << '-' << std::setw(2) << _v->tm_mday << ' ' << std::setw(2)
	  << _v->tm_hour << ':' << std::setw(2) << _v->tm_min << ':' << std::setw(2) << _v->tm_sec; return os.str();
  }
}
