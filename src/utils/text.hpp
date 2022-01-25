#pragma once
#include <iomanip>
#include <string.h>
#include <assert.h>
#include <iostream>
#include <iosfwd>
#include "./str.h"//If it is utf8, please set three times the length
template<unsigned short I = 255>//Max [65535(char),21845(utf8)], Min 1, default 255.
class text {//It is similar to a dynamic std::string_view with a fixed maximum length
  unsigned short l = I; char* _ = new char[I + 1];
  friend std::string& operator<<(std::string& s, text<I>& c) {
	s.push_back('"'); s += c.c_str(); s.push_back('"'); return s;
  };
  friend std::string& operator<<(std::string& s, const text<I>& c) {
	s.push_back('"'); s += c.c_str(); s.push_back('"'); return s;
  };
  friend std::ostream& operator<<(std::ostream& s, text<I>& c) {
	return s << c.c_str();
  };
public:
  ~text() { delete[]_; _ = nullptr; };
  text() { static_assert(I != 0); _[0] = l = 0; };
  text(const char* c) {
	static_assert(I != 0); size_t i = strlen(c); if (i < I)l = i; strncpy(_, c, I); _[l] = 0;
  };
  text(const text& str) { strcpy(_, str._); l = str.l; }
  template<unsigned short L>
  text(const text<L>& str) {
	static_assert(I >= L); strcpy(_, str.c_str()); l = str.length();
  }
  text(const std::string& str) {
	size_t i = str.length(); strncpy(_, str.c_str(), I); if (i < I)l = i; _[l] = 0;
  }
  text& operator = (const char* str) {
	delete[]_; _ = new char[I + 1]; strncpy(_, str, I); size_t n = strlen(str); l = I < n ? I : n; _[l] = 0; return *this;
  }
  text& operator = (const std::string& str) {
	delete[]_; _ = new char[I + 1]; strncpy(_, str.c_str(), I); size_t n = str.length(); l = I < n ? I : n; _[l] = 0; return *this;
  }
  text& operator = (const text& str) {
	delete[]_; _ = new char[I + 1]; strcpy(_, str._); l = str.l; return *this;
  }
  template<unsigned short L>
  text& operator = (const text<L>& str) {
	delete[]_; _ = new char[I + 1]; strncpy(_, str.c_str(), I); l = str.length(); _[l] = 0; return *this;
  }
  inline const char* c_str() const { return _; }//Same as std::string
  inline const unsigned short length() const { return l; }//Same as std::string
  inline char& operator[](unsigned short i) { return _[i]; }//Same as std::string
  inline void operator +=(const char* c) { while (*c && l < I) { _[l++] = *c++; } _[l] = 0; }//Safe, like std::string
  inline void operator & (const char* c) { while (*c) { _[l++] = *c++; } }//Fast, but not safe. Unless you're sure it's enough(maybe need .end())
  inline void operator +=(char c) { _[l++] = c; }//Incomplete safety, but it's generally safe and fast(maybe need .end())
  void operator += (const text& t) {
	const char* s = t.c_str();
	if (&t == this) {
	  int i = 2 * l, k = -1; if (i > I)i = I; while (l < i) { _[l++] = s[++k]; } _[i] = 0;
	} else {
	  unsigned short i = 0xffff; while (s[++i] && l < I) { _[l++] = s[i]; } _[l] = 0;
	}
  }
  template<unsigned short L>
  void operator += (const text<L>& t) {
	const char* s = t.c_str(); unsigned short i = 0xffff; while (s[++i] && l < I) { _[l++] = s[i]; } _[l] = 0;
  }
  void operator += (const std::string& t) {
	unsigned short i = 0xffff; while (t[++i] && l < I) { _[l++] = t[i]; } _[l] = 0;
  }
  inline void push_back(const char c) { if (l < I) _[l++] = c; }
  inline void pop_back() { _[--l] = 0; }
  inline void push_begin(const char c) { unsigned short i = l; while (i) { _[i] = _[i - 1]; --i; } _[++l] = 0; _[0] = c; }
  inline void end() { _[l] = 0; }
  inline void clear() { _[0] = l = 0; }
  inline void slice(unsigned char i) { assert(i < I); _[i] = 0; l = i; }
};
template<unsigned short I>
std::ostream& operator<<(std::ostream& s, text<I> c) {
  return s << c.c_str();
}
template<unsigned short I>
text<I> operator+(text<I>& t, const char* c) {
  unsigned short& l = *((unsigned short*)(&t)); while (*c && l < I) { t[l] = *c++; ++l; } t[l] = 0; return t;
}
template<unsigned short I>
text<I> operator+(text<I>& t, const std::string& $) {
  unsigned short& l = *((unsigned short*)(&t)); char* c = (char*)$.c_str(); while (*c && l < I) { t[l] = *c++; ++l; } t[l] = 0; return t;
}
template<unsigned short I, unsigned short K>
text<I> operator+(text<I>& t, const text<K>& $) {
  unsigned short& l = *((unsigned short*)(&t)); char* c = (char*)$.c_str(); while (*c && l < I) { t[l] = *c++; ++l; } t[l] = 0; return t;
}
template<unsigned short I>
text<I> operator+(const char* c, text<I>& t) {
  text<I> f(t); unsigned short r = strlen(c) - 1, & l = *((unsigned short*)(&t)), i = 0; while (f[i] && r < I) { t[++r] = f[i]; ++i; ++l; }
  i = 0; while (*c) { t[i] = *c++; ++i; } t[++l] = 0; return t;
}
template<unsigned short I>
text<I> operator+(const char c, text<I>& t) {
  unsigned short& l = *((unsigned short*)(&t)), i = l; while (i) { t[i] = t[i - 1]; --i; } t[++l] = 0; t[0] = c; return t;
}
template<unsigned short I>
std::string operator+(std::string& t, const text<I>& $) {
  char* c = (char*)$.c_str(); while (*c) { t.push_back(*c); *++c; } t.push_back(0); return t;
}
template<class T>
struct is_text : std::false_type {};
template<class T>
struct is_text<T[]> : std::false_type {};
template<unsigned short N>
struct is_text<text<N>> : std::true_type {};
inline const std::string textify(const char* t) { return toQuotes(t); }
inline const std::string textify(const std::string& t) { return toQuotes(t.c_str()); }
inline const std::string textify(const tm& _v) {
  std::ostringstream os; os << std::setfill('0');
#ifdef _WIN32
  os << std::setw(4) << _v.tm_year + 1900;
#else
  int y = _v.tm_year / 100; os << std::setw(2) << 19 + y << std::setw(2) << _v.tm_year - y * 100;
#endif
  os << '-' << std::setw(2) << (_v.tm_mon + 1) << '-' << std::setw(2) << _v.tm_mday << ' ' << std::setw(2)
	<< _v.tm_hour << ':' << std::setw(2) << _v.tm_min << ':' << std::setw(2) << _v.tm_sec; return os.str();
}
template<unsigned short I>
inline const std::string textify(const text<I>& t) { return toQuotes(t.c_str()); }
template<typename T>
inline typename std::enable_if<std::is_fundamental<T>::value, const std::string>::type textify(const T& t) { return std::to_string(t); }
template<unsigned short I, typename T>
text<I * 2> operator!=(const text<I>& o, const T& v) {
  text<I * 2> x(o); x += '<'; x += '>';
  if constexpr (std::is_same<T, std::string>::value || std::is_same<T, tm>::value || is_text<T>::value) {
	x += '\''; x += textify(v); x += '\'';
  } else {
	x += textify(v);
  } x += (char)0; return x;
}
template<unsigned short I, typename T>
text<I * 2> operator==(const text<I>& o, const T& v) {
  text<I * 2> x(o); x += '=';
  if constexpr (std::is_same<T, std::string>::value || std::is_same<T, tm>::value || is_text<T>::value) {
	x += '\''; x += textify(v); x += '\'';
  } else {
	x += textify(v);
  } x += (char)0; return x;
}
template<unsigned short I, typename T>
text<I * 2> operator<(const text<I>& o, const T& v) {
  text<I * 2> x(o); x += '<';
  if constexpr (std::is_same<T, std::string>::value || std::is_same<T, tm>::value || is_text<T>::value) {
	x += '\''; x += textify(v); x += '\'';
  } else {
	x += textify(v);
  } x += (char)0; return x;
}
template<unsigned short I, typename T>
text<I * 2> operator<=(const text<I>& o, const T& v) {
  text<I * 2> x(o); x += '<'; x += '=';
  if constexpr (std::is_same<T, std::string>::value || std::is_same<T, tm>::value || is_text<T>::value) {
	x += '\''; x += textify(v); x += '\'';
  } else {
	x += textify(v);
  } x += (char)0; return x;
}
template<unsigned short I, typename T>
text<I * 2> operator>=(const text<I>& o, const T& v) {
  text<I * 2> x(o); x += '>'; x += '=';
  if constexpr (std::is_same<T, std::string>::value || std::is_same<T, tm>::value || is_text<T>::value) {
	x += '\''; x += textify(v); x += '\'';
  } else {
	x += textify(v);
  } x += (char)0; return x;
}
template<unsigned short I, typename T>
text<I * 2> operator>(const text<I>& o, const T& v) {
  text<I * 2> x(o); x += '>';
  if constexpr (std::is_same<T, std::string>::value || std::is_same<T, tm>::value || is_text<T>::value) {
	x += '\''; x += textify(v); x += '\'';
  } else {
	x += textify(v);
  } x += (char)0; return x;
}
template<unsigned short I, unsigned short L>
text<I + L + 7> operator&&(text<I> o, const text<L>& c) {
  text<I + L + 7> x("("); x += o; x += " AND "; x += c; x += ')'; x.end(); return x;
};
template<unsigned short I, unsigned short L>
text<I + L + 6> operator||(text<I> o, const text<L>& c) {
  text<I + L + 6> x("("); x += o; x += " OR "; x += c; x += ')'; x.end(); return x;
};
