#pragma once
#include <ctime>
#include <string>
#include <iomanip>
#include <cstdlib>
//<Ctrl> + Left mouse button -> Jump to the specified location
tm operator+(tm& t, tm& m);
tm operator-(tm& t, tm& m);
bool operator==(tm& t, tm& m);
bool operator!=(tm& t, tm& m);
bool operator<(tm& t, tm& m);
bool operator>(tm& t, tm& m);
bool operator<=(tm& t, tm& m);
bool operator>=(tm& t, tm& m);
static std::string& toUpperCase(std::string& s);
static std::string toUpperCase(const char* s);
static std::string& toLowerCase(std::string& s);
static std::string toLowerCase(const char* s);
static std::string toSqlCase(const char* s);
template<typename T> const char* ObjName();
static std::string toQuotes(const char* s);
std::ostream& operator<<(std::ostream& os, const tm& _v);
std::string& operator<<(std::string& s, const tm& _v);
#ifdef __cplusplus
extern "C" {
#endif
  static char RES_ASCII[97] = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
  static int strLen(const char* s) { const char* e = s; while (*++e); return e - s; }
  static void strCpy(char* d, const char* s) { while (*s) { *d++ = *s++; }*d = 0; }
  static char* strStr(char* d, const char* s) {
	int i = 0, j = 0, l = strLen(d), k = strLen(s);
	while (i < l&& j < k) if (d[i] == s[j])++i, ++j; else i = i - j + 1, j = 0;
	if (j == k) { return d + i - k; } return (char*)0;
  }
  static int strCmp(const char* c, const char* s) {
	while (*s == *c && *c && *s)++c, ++s; return *c == *s ? 0 : *c > *s ? 1 : -1;
  }
  static int find1CharPosDESC(const char* c, const char d) {
	for (int l = strLen(c), i = l - 1; i > 0; --i) { if (c[i] == d)return l - i; } return -1;
  }
  static int find1CharPosASC(const char* c, const char d) {
	for (int l = strLen(c), i = 0; i < l; ++i) { if (c[i] == d)return i; }return -1;
  }
  //If not safe, it is agreed not to use static. Also means that delete or free is required
  char* subStr(const char* c, int i, int e) {
	if (e < i || i < 0)return (char*)0;
	char* w = (char*)malloc(sizeof(char) * (e - i + 1)); int p = 0; while (i < e)w[p++] = c[i++]; w[p] = 0; return w;
  }
  char* joinS2CInInt(const char* c, int i, const char* s) {
	int j = strLen(s), l = strLen(c), p = 0, n = 0, m = 0;
	char* w = (char*)malloc(sizeof(char) * (l + j));
	--i; while (p < i)w[p++] = c[n++]; while (m < j)w[p++] = s[m++];
	while (n < l) { w[p++] = c[n++]; } w[p] = '\0'; return w;
  }
  char* to8Str(unsigned long long i) {
	int z = 2; for (unsigned long long a = i; a > 0x7f; a -= 0x7f, a /= 0x100, ++z);
	unsigned long long b, t = i / 0x100; b = i - t * 0x100 - 32;
	char* w = (char*)malloc(sizeof(char) * z); w[--z] = '\0';
	while (t > 0x7f) { w[--z] = RES_ASCII[b]; i = t; t = i / 0x100; b = i - t * 0x100 - 32; }
	w[--z] = RES_ASCII[b]; if (z > 0) { t -= 32; w[0] = RES_ASCII[t]; } return w;
  }
  char* to4Str(int i) {
	int t = i / 0x100, b = i - t * 0x100 - 32, z = i > 0x7f7f7f ? 5 : i > 0x7f7f ? 4 : i > 0x7f ? 3 : 2;
	char* w = (char*)malloc(sizeof(char) * z);  w[--z] = '\0';
	while (t > 0x7f) { w[--z] = RES_ASCII[b]; i = t; t = i / 0x100; b = i - t * 0x100 - 32; }
	w[--z] = RES_ASCII[b]; if (z > 0) { t -= 32; w[0] = RES_ASCII[t]; } return w;
  }
  static int hack_4str(const char* oid) {
	int t = 0, i = strLen(oid), j, pow = 1;
	while (i-- > 0) {
	  char* chr = subStr(oid, i, i + 1); j = 0;
	  while (j++ < 0x61) {
		char* s = subStr(RES_ASCII, j - 1, j);
		if (strCmp(chr, s) == 0) {
		  t = t + (j + 0x1f) * pow; pow = pow * 0x100; free(s); break;
		} free(s);
	  } free(chr);
	} return t;
  }
  static inline constexpr unsigned long long hack8Str(const char* s) {
	unsigned long long r = 0; for (signed char i = -1; ++i < 8 && s[i]; r *= 0x100, r += s[i]); return r;
  }//If only the first four digits need to be matched and there is no conflict, it is recommended to use hack4Str to improve efficiency
  static inline constexpr int hack4Str(const char* s) { int r = 0; for (signed char i = -1; ++i < 4 && s[i]; r *= 0x100, r += s[i]); return r; }
  //Hack8str is downward compatible with hack4str, however, it is not compatible with the hackstr method
  static inline constexpr unsigned long long hackStr(const char* s) {
	unsigned long long r = 0; for (unsigned short i = 0xffff; s[++i]; r *= 0x1f, r += s[i] - 0x2f); return r;
  }
  //The following void can only be used for MySQL or certain types(in the "ccORM.hh" file)
  static inline short atos_(char* c) {
	short r = 0; if (*c == '-') { while (*++c) r = r * 10 - *c + 0x30; } else { while (*c) r = r * 10 + *c++ - 0x30; } return r;
  }
  static inline int atoi_(char* c) {
	int r = 0; if (*c == '-') { while (*++c) r = r * 10 - *c + 0x30; } else { while (*c) r = r * 10 + *c++ - 0x30; } return r;
  }
  static inline long long atol_(char* c) {
	long long r = 0; if (*c == '-') { while (*++c) r = r * 10 - *c + 0x30; } else { while (*c) r = r * 10 + *c++ - 0x30; } return r;
  }
  static inline long long aton_(char* c) { long long r = 0; while (*++c) r = r * 10 - *c + 0x30; return r; }//negative
  static inline unsigned long long atou_(char* c) { unsigned long long r = 0; while (*c) r = r * 10 + *c++ - 0x30; return r; }
#ifdef __cplusplus
}  /* extern "C" */
constexpr unsigned long long operator""_l(const char* s, size_t /*len*/) {
  unsigned long long r = 0; for (unsigned long long i = 0; i < 8 && s[i]; r *= 0x100, r += s[i++]); return r;
}
constexpr int operator""_i(const char* s, size_t /*len*/) {
  int r = 0; for (int i = 0; i < 4 && s[i]; r *= 0x100, r += s[i++]); return r;
}
//You can match more strings with hackstr method, but you need to match ""_a used together
constexpr unsigned long long operator""_a(const char* s, size_t /*len*/) {
  unsigned long long r = 0; for (unsigned long long i = 0; s[i]; r *= 0x1f, r += s[i++] - 0x2f); return r;
}
#ifdef _WIN32
#define _IS_WIN 1
#else
#define _IS_WIN 0
#endif
static std::string& toUpperCase(std::string& s) {
  char* c = (char*)s.c_str(); if (*c > 0x60 && *c < 0x7b) { *c &= ~0x20; }
  while (*++c) { if (*c > 0x60 && *c < 0x7b) *c &= ~0x20; } return s;
}
static std::string toUpperCase(const char* s) {
  std::string e; if (*s > 0x60 && *s < 0x7b) { e.push_back(*s - 0x20); }
  while (*++s) { if (*s > 0x60 && *s < 0x7b) { e.push_back(*s - 0x20); } else { e.push_back(*s); } } return e;
}
static std::string& toLowerCase(std::string& s) {
  char* c = (char*)s.c_str(); if (*c > 0x40 && *c < 0x5b) { *c |= 0x20; }
  while (*++c) { if (*c > 0x40 && *c < 0x5b) *c |= 0x20; } return s;
}
static std::string toLowerCase(const char* s) {
  std::string e; if (*s > 0x40 && *s < 0x5b) { e.push_back(*s + 0x20); }
  while (*++s) { if (*s > 0x40 && *s < 0x5b) { e.push_back(*s + 0x20); } else { e.push_back(*s); } } return e;
}
static std::string toSqlCase(const char* s) {
  std::string e; if (*s > 0x40 && *s < 0x5b) { e.push_back(*s + 0x20); }
  while (*++s) { if (*s > 0x40 && *s < 0x5b) { e.push_back(0x5f); e.push_back(*s + 0x20); } else { e.push_back(*s); } } return e;
}
template<typename T> inline const char* ObjName() {
  const char* s = typeid(T).name(); if constexpr (_IS_WIN) {
	while (*++s != 0x20); return ++s;
  } else { while (*s < 0x3a && *s++ != 0x24) {}; return s; }
}
template<typename T, typename U> inline uint64_t ObjLink() {
  const char* s = typeid(T).name(), * c = typeid(U).name(); unsigned long long r = 0; if constexpr (_IS_WIN) {
	while (*++s != 0x20); while (*++c != 0x20); for (; *++s; r *= 0x1f, r += *s - 0x2f); for (; *++c; r *= 0x1f, r += *c - 0x2f);
  } else {
	while (*s < 0x3a && *s++ != 0x24) {}; while (*c < 0x3a && *c++ != 0x24) {};
	for (; *s; r *= 0x1f, r += *s++ - 0x2f); for (; *c; r *= 0x1f, r += *c++ - 0x2f);
  }
  return r;
}
static std::string toQuotes(const char* s) {
  std::string e; while (*s) { if (*s == 0x27) { e.push_back(0x27); e.push_back(0x27); } else { e.push_back(*s); } *++s; } return e;
}
std::ostream& operator<<(std::ostream& os, const tm& _v) {
#ifdef _WIN32
  os << std::setfill('0') << std::setw(4) << _v.tm_year + 1900;
#else
  int y = _v.tm_year / 100; os << std::setfill('0') << std::setw(2) << 19 + y << std::setw(2) << _v.tm_year - y * 100;
#endif
  os << '-' << std::setw(2) << (_v.tm_mon + 1) << '-' << std::setw(2) << _v.tm_mday << ' ' << std::setw(2)
	<< _v.tm_hour << ':' << std::setw(2) << _v.tm_min << ':' << std::setw(2) << _v.tm_sec; return os;
}
std::string& operator<<(std::string& s, const tm& _v) {
  std::ostringstream os;
#ifdef _WIN32
  os << std::setfill('0') << std::setw(4) << _v.tm_year + 1900;
#else
  int y = _v.tm_year / 100; os << std::setfill('0') << std::setw(2) << 19 + y << std::setw(2) << _v.tm_year - y * 100;
#endif
  os << '-' << std::setw(2) << (_v.tm_mon + 1) << '-' << std::setw(2) << _v.tm_mday << ' ' << std::setw(2)
	<< _v.tm_hour << ':' << std::setw(2) << _v.tm_min << ':' << std::setw(2) << _v.tm_sec; s = os.str(); return s;
}
tm operator+(tm& t, tm& m) {
  tm time; memcpy(&time, &t, sizeof(tm)); time.tm_sec += m.tm_sec; time.tm_min += m.tm_min; time.tm_hour += m.tm_hour;
  time.tm_mday += m.tm_mday; time.tm_mon += m.tm_mon; time.tm_year += m.tm_year; time.tm_isdst = 0; return time;
}
tm operator-(tm& t, tm& m) {
  tm time; memcpy(&time, &t, sizeof(tm)); time.tm_sec -= m.tm_sec; time.tm_min -= m.tm_min; time.tm_hour -= m.tm_hour;
  time.tm_mday -= m.tm_mday; time.tm_mon -= m.tm_mon; time.tm_year -= m.tm_year; time.tm_isdst = 0; return time;
}
bool operator==(tm& t, tm& m) { return mktime(&t) == mktime(&m); }
bool operator!=(tm& t, tm& m) { return mktime(&t) != mktime(&m); }
bool operator<(tm& t, tm& m) { return mktime(&t) < mktime(&m); }
bool operator>(tm& t, tm& m) { return mktime(&t) > mktime(&m); }
bool operator<=(tm& t, tm& m) { return mktime(&t) <= mktime(&m); }
bool operator>=(tm& t, tm& m) { return mktime(&t) >= mktime(&m); }
//Basic judgment type characters supported by ccORM (with switch(hack8Str(...)))
#if _IS_WIN
#define T_INT8 "signed char"_l
#define T_UINT8 " char"_l
#define T_INT16 "short"_l
#define T_UINT16 " short"_l
#define T_INT 'int'
#define T_UINT " int"_l
#define T_INT64 "__int64"_l
#define T_UINT64 " __int64"_l
#define T_BOOL 'bool'
#define T_DOUBLE "double"_l
#define T_FLOAT "float"_l
#define T_TM "struct tm"_l
#define T_TEXT "class text"_l
#define T_STRING "class std::basic_string"_l
//#elif __APPLE__
#else
#define T_INT8 'a'
#define T_UINT8 'h'
#define T_INT16 's'
#define T_UINT16 't'
#define T_INT 'i'
#define T_UINT 'j'
#define T_INT64 'x'
#define T_UINT64 'y'
#define T_BOOL 'b'
#define T_DOUBLE 'd'
#define T_FLOAT 'f'
#define T_TM '2tm'
#define T_TEXT "4textILt"_l
#define T_STRING "NSt7__cx"_l
#endif
//Corrected type character sequence
#define T_INT8_ 'a'
#define T_UINT8_ 'h'
#define T_INT16_ 's'
#define T_UINT16_ 't'
#define T_INT_ 'i'
#define T_UINT_ 'j'
#define T_INT64_ 'x'
#define T_UINT64_ 'y'
#define T_BOOL_ 'b'
#define T_DOUBLE_ 'd'
#define T_FLOAT_ 'f'
#define T_TM_ 'm'
#define T_TEXT_ 'e'
#define T_STRING_ 'c'
#define T_POINTER_ '*'
#define T_VECTOR_ '_'
#endif
