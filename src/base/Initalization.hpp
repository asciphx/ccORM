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
  typename std::enable_if<std::is_fundamental<T>::value, std::string>::type stringify(const T& t) { return std::to_string(t); }
  template<typename T>
  typename std::enable_if<!std::is_fundamental<T>::value, std::string>::type stringify(const T& t) { return t; }
  template<typename T>
  void regist() {
	unsigned int i = HARDWARE_ASYNCHRONOUS; while (i--) { orm::Table<T>::__[i] = new Sql<T>(); }
	try {
	  T::Init();//Build and check whether the table creation statement of table is correct
	} catch (const std::exception&e) {
	  std::cerr << e.what(); return;
	}
	T::_addTable();//Create table
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
	(void)std::initializer_list<int>{(regist<Args>(), 0)...};
  }
  static std::string Time2Str(const time_t* t) {
	tm* _v; _v = std::localtime(t); std::ostringstream os;
	os << 20 << (_v->tm_year - 100) << '-' << std::setfill('0') << std::setw(2)
	  << (_v->tm_mon + 1) << '-' << std::setw(2) << _v->tm_mday << ' ' << std::setw(2)
	  << _v->tm_hour << ':' << std::setw(2) << _v->tm_min << ':' << std::setw(2)
	  << _v->tm_sec; return os.str();
  }
}
