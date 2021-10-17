#ifndef ORM_S2O_HPP
#define ORM_S2O_HPP
#include <string>
#include <ctime>
#include <type_traits>
#include <algorithm>
#include <sstream>
#include "std_s.hpp"
//String type setting system(Not used at the moment)
template<typename T>
constexpr typename std::enable_if<std::is_same<T, const char*>::value, std::string>::type str_or_type(const T t) {
  std::string buf = (const char*)t; return buf;
}
template<typename T>
constexpr typename std::enable_if<!std::is_same<T, const char*>::value, T>::type str_or_type(const T t) { return t; }
namespace orm {
  template <typename T>
  bool s2o(const std::string& str, T& value);
  template <>
  bool s2o<bool>(const std::string& str, bool& value);
  template <>
  bool s2o<signed char>(const std::string& str, signed char& value);
  template <>
  bool s2o<short>(const std::string& str, short& value);
  template <>
  bool s2o<int>(const std::string& str, int& value);
  template <>
  bool s2o<unsigned int>(const std::string& str, unsigned int& value);
  template <>
  bool s2o<long long>(const std::string& str, long long& value);
  template <>
  bool s2o<unsigned long long>(const std::string& str, unsigned long long& value);
  template <>
  bool s2o<float>(const std::string& str, float& value);
  template <>
  bool s2o<double>(const std::string& str, double& value);
  template <>
  bool s2o<long double>(const std::string& str, long double& value);
  template <>
  bool s2o<tm>(const std::string& str, tm& value);
  template <typename T>
  bool s2o(const std::string& str, T& value) {
	static_assert(std::is_convertible<std::string, T>::value,
	  "Impossible to convert T from std::string. Please make your own s2o() function");
	value = str;
	return true;
  }
  template <>
  bool s2o<short>(const std::string& str, short& value) {
	try {
	  value = std::stos(str);
	} catch (...) {
	  return false;
	}
	return true;
  }
  template <>
  bool s2o<signed char>(const std::string& str, signed char& value) {
	try {
	  value = std::stot(str);
	} catch (...) {
	  return false;
	}
	return true;
  }
  template <>
  bool s2o<int>(const std::string& str, int& value) {
	try {
	  value = std::stoi(str);
	} catch (...) {
	  return false;
	}
	return true;
  }
  template <>
  bool s2o<bool>(const std::string& str, bool& value) {
	int i = value;
	if (s2o<int>(str, i)) {
	  value = i;
	} else {
	  std::string data = str;
	  std::transform(data.begin(), data.end(), data.begin(), ::tolower);
	  if (str == "true") {
		value = true;
	  } else if (str == "1") {
		value = true;
	  } else {
		return false;
	  }
	}
	return true;
  }
  template <>
  bool s2o<unsigned int>(const std::string& str, unsigned int& value) {
	try {
	  value = std::stoul(str);
	} catch (...) {
	  return false;
	}
	return true;
  }
  template <>
  bool s2o<long long>(const std::string& str, long long& value) {
	try {
	  value = std::stoll(str);
	} catch (...) {
	  return false;
	}
	return true;
  }
  template <>
  bool s2o<unsigned long long>(const std::string& str, unsigned long long& value) {
	try {
	  value = std::stoull(str);
	} catch (...) {
	  return false;
	}
	return true;
  }
  template <>
  bool s2o<float>(const std::string& str, float& value) {
	try {
	  value = std::stof(str);
	} catch (...) {
	  return false;
	}
	return true;
  }
  template <>
  bool s2o<double>(const std::string& str, double& value) {
	try {
	  value = std::stod(str);
	} catch (...) {
	  return false;
	}
	return true;
  }
  template <>
  bool s2o<long double>(const std::string& str, long double& value) {
	try {
	  value = std::stold(str);
	} catch (...) {
	  return false;
	}
	return true;
  }
  template <>
  bool s2o<tm>(const std::string& str, tm& value) {
	int year = 0;
	int month = 0;
	int day = 0;
	int hour = 0;
	int min = 0;
	int sec = 0;
	if (sscanf(str.c_str(), RES_DATE_FORMAT, &year, &month, &day, &hour, &min, &sec) != 6) {
	  return false;
	}
	value.tm_year = year - 1900;
	value.tm_mon = month - 1;
	value.tm_mday = day;
	value.tm_hour = hour;
	value.tm_min = min;
	value.tm_sec = sec;
	return true;
  }
}
#endif