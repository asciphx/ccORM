#pragma once
#include <string>
#include <stdexcept>
namespace std {
  inline short stos(const string& _Str) {
	const char* _Ptr = _Str.c_str(); int _Base = 10;
	char* _Eptr;
	const long _Ans = ::strtol(_Ptr, &_Eptr, _Base);
	if (_Ptr == _Eptr) {
	  throw std::runtime_error("invalid stos argument");
	}
	if (_Ans < -32768 || 32767 < _Ans) {
	  throw std::runtime_error("stos argument out of range");
	}
	return static_cast<short>(_Ans);
  }
  inline signed char stot(const string& _Str) {
	const char* _Ptr = _Str.c_str(); int _Base = 10;
	char* _Eptr;
	const long _Ans = ::strtol(_Ptr, &_Eptr, _Base);
	if (_Ptr == _Eptr) {
	  throw std::runtime_error("invalid stos argument");
	}
	if (_Ans < -128 || 127 < _Ans) {
	  throw std::runtime_error("stos argument out of range");
	}
	return static_cast<signed char>(_Ans);
  }
}
