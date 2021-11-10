#pragma once
#include <ctime>
#include <string>
#include <iomanip>
#include <cstdlib>
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
	if (c[0] == 0 || s[0] == 0) { return -1; } while (*s == *c && *c && *s)c++, s++;
	if (*c == *s) { return 0; } if (*c > *s)return 1; return -1;
  }
  static int find1CharPosDESC(const char* c, const char d) {
	for (int l = strLen(c), i = l - 1; i > 0; --i) { if (c[i] == d)return l - i; } return -1;
  }
  static int find1CharPosASC(const char* c, const char d) {
	for (int l = strLen(c), i = 0; i < l; ++i) { if (c[i] == d)return i; }return -1;
  }
  //other not safe
  static char* subStr(const char* c, int i, int e) {
	if (e < i || i < 0)return (char*)0;
	char* w = (char*)malloc(sizeof(char) * (e - i + 1)); int p = 0; while (i < e)w[p++] = c[i++]; w[p] = 0; return w;
  }
  static char* joinS2CInInt(const char* c, int i, const char* s) {
	int j = strLen(s), l = strLen(c), p = 0, n = 0, m = 0;
	char* w = (char*)malloc(sizeof(char) * (l + j));
	--i; while (p < i)w[p++] = c[n++]; while (m < j)w[p++] = s[m++];
	while (n < l) { w[p++] = c[n++]; } w[p] = '\0'; return w;
  }
  static char* to8Str(unsigned long long i) {
	int z = 2; for (unsigned long long a = i; a > 0x7f; a -= 0x7f, a /= 0x100, ++z);
	unsigned long long b, t = i / 0x100; b = i - t * 0x100 - 32;
	char* w = (char*)malloc(sizeof(char) * z); w[--z] = '\0';
	while (t > 0x7f) {
	  w[--z] = RES_ASCII[b]; i = t; t = i / 0x100; b = i - t * 0x100 - 32;
	}
	w[--z] = RES_ASCII[b]; if (z > 0) { t -= 32; w[0] = RES_ASCII[t]; }
	return w;
  }
  static char* to4Str(int i) {
	int t = i / 0x100, b = i - t * 0x100 - 32, z = i > 0x7f7f7f ? 5 : i > 0x7f7f ? 4 : i > 0x7f ? 3 : 2;
	char* w = (char*)malloc(sizeof(char) * (z)); w[--z] = '\0';
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
  constexpr unsigned long long hack8Str(const char* s) {
	unsigned long long r = 0; for (int i = 0; i < 8 && s[i]; r *= 0x100, r += s[i++]); return r;
  }//If only the first four digits need to be matched and there is no conflict, it is recommended to use hack4Str to improve efficiency
  constexpr int hack4Str(const char* s) { int r = 0; for (int i = 0; i < 4 && s[i]; r *= 0x100, r += s[i++]); return r; }
  //Hack8str is downward compatible with hack4str, however, it is not compatible with the hackallstr method
  constexpr unsigned long long hackAllStr(const char* s) {
	unsigned long long r = 0; for (int i = 0; s[i]; r *= 0x1f, r += s[i++]); return r;
  }
#ifdef __cplusplus
}  /* extern "C" */
constexpr unsigned long long operator""_l(const char* s, size_t /*len*/) {
  unsigned long long r = 0; for (unsigned long long i = 0; i < 8 && s[i]; r *= 0x100, r += s[i++]); return r;
}
constexpr int operator""_i(const char* s, size_t /*len*/) {
  int r = 0; for (int i = 0; i < 4 && s[i]; r *= 0x100, r += s[i++]); return r;
}
//You can match more strings with hackallstr method, but you need to match ""_a used together
constexpr unsigned long long operator""_a(const char* s, size_t /*len*/) {
  unsigned long long r = 0; for (unsigned long long i = 0; s[i]; r *= 0x1f, r += s[i++]); return r;
}

static std::string toLowerCase(const char* s) {
  std::string e; while (*s) {
	if (*s > 0x40 && *s < 0x5b) {
	  e += *s + 0x20;
	} else { e += *s; }
	*++s;
  } return e;
}
static std::string toSqlLowerCase(const char* s) {
  std::string e;
  if (*s > 0x40 && *s < 0x5b) { e += *s + 0x20; }
  while (*++s) {
	if (*s > 0x40 && *s < 0x5b) {
	  e += 0x5f; e += *s + 0x20;
	} else { e += *s; }
  } return e;
}
template<typename T> const char* getObjectName() {
  const char* s = typeid(T).name();
#if _WIN32
  while (*++s != 0x20); return ++s;
#else
  while (*s < 0x3a && *s++ != 0x24) {}; return s;
#endif
}
static std::string toQuotes(const char* s) {
  std::string e; while (*s) {
	if (*s == 0x27) {
	  e += 0x27; e += 0x27;
	} else { e += *s; }
	*++s;
  } return e;
}
std::ostream& operator<<(std::ostream& os, const tm& time) {
  os << 20 << (time.tm_year - 100) << '-' << std::setfill('0')
	<< std::setw(2) << (time.tm_mon + 1) << '-' << std::setw(2) << time.tm_mday
	<< ' ' << std::setw(2) << time.tm_hour << ':' << std::setw(2) << time.tm_min
	<< ':' << std::setw(2) << time.tm_sec; return os;
}
std::string operator&& (const std::string& o, const std::string& c) {
  std::ostringstream os; os << '(' << o << " AND " << c << ')'; return os.str();
};
std::string operator|| (const std::string& o, const std::string& c) {
  std::ostringstream os; os << '(' << o << " OR " << c << ')'; return os.str();
};
template<typename T>
std::string operator<(const std::string& o, const T& v) {
  std::ostringstream os; os << o << '<' << v; return os.str();
}
template<typename T>
std::string operator<=(const std::string& o, const T& v) {
  std::ostringstream os; os << o << "<=" << v; return os.str();
}
template<typename T>
std::string operator>=(const std::string& o, const T& v) {
  std::ostringstream os; os << o << ">=" << v; return os.str();
}
template<typename T>
std::string operator>(const std::string& o, const T& v) {
  std::ostringstream os; os << o << '>' << v; return os.str();
}
#endif
