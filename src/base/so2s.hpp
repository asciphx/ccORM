#ifndef ORM_SO2S_HPP
#define ORM_SO2S_HPP
#include <string>
#include <ctime>
#include <type_traits>
#include <algorithm>
#include <sstream>
#include "std_s.hpp"
#pragma warning(disable:4834)
//String type detection system
namespace orm {
  template <typename T>
  bool so2s(const char* str);
  template <>
  bool so2s<bool>(const char* str);
  template <>
  bool so2s<int8_t>(const char* str);
  template <>
  bool so2s<short>(const char* str);
  template <>
  bool so2s<int>(const char* str);
  template <>
  bool so2s<unsigned int>(const char* str);
  template <>
  bool so2s<long long>(const char* str);
  template <>
  bool so2s<unsigned long long>(const char* str);
  template <>
  bool so2s<float>(const char* str);
  template <>
  bool so2s<double>(const char* str);
  template <>
  bool so2s<long double>(const char* str);
  template <>
  bool so2s<tm>(const char* str);
  template <typename T>
  bool so2s(const char* str) {
	static_assert(std::is_convertible<const char*, T>::value,
	  "Impossible to convert T from const char*. Please make your own so2s() function");
	return true;
  }
  template <>
  bool so2s<short>(const char* str) {
	try {
	  std::stos(str);
	} catch (...) {
	  return false;
	}
	return true;
  }
  template <>
  bool so2s<int8_t>(const char* str) {
	try {
	  std::stot(str);
	} catch (...) {
	  return false;
	}
	return true;
  }
  template <>
  bool so2s<int>(const char* str) {
	try {
	  std::stoi(str);
	} catch (...) {
	  return false;
	}
	return true;
  }
  template <>
  bool so2s<bool>(const char* str) {
	if (strcmp(str, "true")==0) {
	  return true;
	} else if (strcmp(str, "false")==0) {
	  return true;
	} else {
	  return false;
	}
  }
  template <>
  bool so2s<unsigned int>(const char* str) {
	try {
	  std::stoul(str);
	} catch (...) {
	  return false;
	}
	return true;
  }
  template <>
  bool so2s<long long>(const char* str) {
	try {
	  std::stoll(str);
	} catch (...) {
	  return false;
	}
	return true;
  }
  template <>
  bool so2s<unsigned long long>(const char* str) {
	try {
	  std::stoull(str);
	} catch (...) {
	  return false;
	}
	return true;
  }
  template <>
  bool so2s<float>(const char* str) {
	try {
	  std::stof(str);
	} catch (...) {
	  return false;
	}
	return true;
  }
  template <>
  bool so2s<double>(const char* str) {
	try {
	  std::stod(str);
	} catch (...) {
	  return false;
	}
	return true;
  }
  template <>
  bool so2s<long double>(const char* str) {
	try {
	  std::stold(str);
	} catch (...) {
	  return false;
	}
	return true;
  }
  template <>
  bool so2s<tm>(const char* str) {
	int year = 0, month = 0, day = 0, hour = 0, min = 0, sec = 0;
	if (sscanf(str, RES_DATE_FORMAT, &year, &month, &day, &hour, &min, &sec) != 6) {
	  return false;
	}
	return true;
  }
}
#endif
