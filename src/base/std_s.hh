#pragma once
#include <string>
#include <stdexcept>
namespace std {
  inline short stos(const string& _Str) {
	// convert string to short
	const char* _Ptr = _Str.c_str();
	char* _Eptr;
	const long _Ans = ::strtol(_Ptr, &_Eptr, 10);
	if (_Ptr == _Eptr) {
	  throw std::runtime_error("invalid stos argument");
	}
	if (_Ans < -32768 || 32767 < _Ans) {
	  throw std::runtime_error("stos argument out of range");
	}
	return static_cast<short>(_Ans);
  }
  inline int8_t stot(const string& _Str) {
	// convert string to tiny int
	const char* _Ptr = _Str.c_str();
	char* _Eptr;
	const long _Ans = ::strtol(_Ptr, &_Eptr, 10);
	if (_Ptr == _Eptr) {
	  throw std::runtime_error("invalid stot argument");
	}
	if (_Ans < -128 || 127 < _Ans) {
	  throw std::runtime_error("stot argument out of range");
	}
	return static_cast<int8_t>(_Ans);
  }
  //If it is lower than C++ 17, it maybe needs to be implemented
#if _WIN32
#if _HAS_CXX17==0
  struct string_view {
	string_view(const char*, unsigned long long);
	[[nodiscard]] size_t length() const;
	[[nodiscard]] size_t size() const;
	[[nodiscard]] inline const char* data() const;
  };
  struct nullopt_t {
	explicit constexpr nullopt_t(int) {}
  };
  template <class T>
  struct optional {
	using value_type = T;
	constexpr optional() noexcept {}
	constexpr optional(nullopt_t) noexcept {}
  };
  struct any {
	any(void* v); any& operator=(void*);
  };
#endif
#elif !defined(_HAS_CXX17)

#endif
}
