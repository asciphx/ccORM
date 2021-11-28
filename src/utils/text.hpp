#pragma once
template<unsigned short I = 255>//Max 65535, Min 1
struct text {
  ~text() { delete[]_; _ = nullptr; };
  text(const char* c_str = 0) {
	static_assert(I != 0);
	size_t i = strlen(c_str); if (i < I)l = i; strncpy(_, c_str, I); _[l] = 0;
  };
  text(const text& str) {
	strcpy(_, str._); l = str.l;
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
  const char* c_str() const { return _; }
  const short length() const { return l; }
  char& operator[](unsigned short i) { return _[i]; }
  text& operator += (const char* c) {
	while (*c && l < I) { _[l++] = *c++; } _[I] = 0; return *this;
  }
  text& operator += (const text& t) {
	const char* s = t.c_str();
	if (&t == this) {
	  short i = 2 * l, k = -1; if (i > I)i = I; while (l < i) { _[l++] = s[++k]; } _[i] = 0;
	} else {
	  unsigned short i = 0xffff; while (s[++i] && l < I) { _[l++] = s[i]; } _[I] = 0;
	}
	return *this;
  }
  template<unsigned short L>
  text& operator += (const text<L>& t) {
	const char* s = t.c_str(); unsigned short i = 0xffff; while (s[++i] && l < I) { _[l++] = s[i]; } _[I] = 0; return *this;
  }
  text& operator += (const std::string& t) {
	unsigned short i = 0xffff; while (t[++i] && l < I) { _[l++] = t[i]; } _[I] = 0; return *this;
  }
  text& push_back(const char c) {
	if (l < I) _[l++] = c; return *this;
  }
  text& pop_back() {
	_[--l] = 0; return *this;
  }
  friend std::string& operator<<(std::string& s, text<I>& c) {
	s.push_back('"'); s += c.c_str(); s.push_back('"'); return s;
  };
  friend std::ostream& operator<<(std::ostream& s, text<I>& c) {
	return s << c.c_str();
  };
private: char* _ = new char[I + 1]; unsigned short l = I;
};
template<unsigned short I>
text<I> operator+(const text<I>& t, const char* c) {
  unsigned short l = t.length() - 1; char* s = (char*)t.c_str(); while (*c && ++l < I) { s[l] = *c++; } s[++l] = 0; return t;
}
template<unsigned short I>
text<I> operator+(const text<I>& t, const std::string& $) {
  unsigned short l = t.length() - 1; char* s = (char*)t.c_str(), * c = (char*)$.c_str(); while (*c && ++l < I) { s[l] = *c++; } s[++l] = 0; return t;
}
template<unsigned short I, unsigned short K>
text<I> operator+(const text<I>& t, const text<K>& $) {
  unsigned short l = t.length() - 1; char* s = (char*)t.c_str(), * c = (char*)$.c_str(); while (*c && ++l < I) { s[l] = *c++; } s[++l] = 0; return t;
}
template<unsigned short I>
std::string operator+(std::string& t, const text<I>& $) {
  char* c = (char*)$.c_str(); while (*c) { t.push_back(*c); *++c; } t.push_back(0); return t;
}
template<unsigned short I>
text<I> operator+(const char* c, text<I>& t) {
  text<I> f(t); unsigned short r = strlen(c), l = t.length(), i = 0; while (r < I) { t[r++] = f[i]; ++i; ++l; }
  char* s = (char*)t.c_str(); i = 0; while (*c) { s[i] = *c++; ++i; } s[l] = 0; return t;
}
template<class T>
struct is_text : std::false_type {};
template<class T>
struct is_text<T[]> : std::false_type {};
template<unsigned short N>
struct is_text<text<N>> : std::true_type {};
