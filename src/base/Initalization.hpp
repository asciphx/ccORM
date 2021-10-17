#pragma once
namespace orm {
  static tm RES_NOW;
  static auto RES_last = std::chrono::steady_clock::now();
  tm& now() {
	if (std::chrono::steady_clock::now() - RES_last < std::chrono::seconds(2)) return RES_NOW;
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
	  T(true);
	} catch (const std::exception&e) {
	  std::cerr << e.what(); return;
	}
	T::_addTable();// std::cout << typeid(T).name()+6 <<"[created!]"<<std::endl;
  }
  template <typename ...Args>//Registration Center
  static void InitializationOrm() {
	(void)std::initializer_list<int>{(regist<Args>(), void(), 0)...};
	time_t RES_ti; std::time(&RES_ti);
#if defined(_MSC_VER) || defined(__MINGW32__)
	system("chcp 65001");
	localtime_s(&RES_NOW, &RES_ti);
#else
	localtime_r(&RES_ti, &RES_NOW);
	std::locale::global(std::locale("en_US.UTF8"));
#endif
  }
  static std::string Time2Str(const time_t* t) {
	tm* _v; _v = std::localtime(t); std::ostringstream os;
	os << std::setfill('0') << std::setw(4) << (_v->tm_year + 1900) << '-' << std::setw(2)
	  << (_v->tm_mon + 1) << '-' << std::setw(2) << _v->tm_mday << ' ' << std::setw(2)
	  << _v->tm_hour << ':' << std::setw(2) << _v->tm_min << ':' << std::setw(2)
	  << _v->tm_sec; return os.str();
  }
}