#pragma once
template<unsigned char I = 255>
struct text {
  ~text() { delete[]_; _ = nullptr; };
  text(const char* c_str = 0) {
	size_t i = strlen(c_str); if (i <= I)l = i; strncpy(_, c_str, I);
  };
  text(const text& str) {
	strcpy(_, str._); l = str.l;
  }
  text(const std::string& str) {
	size_t i = str.length(); strncpy(_, str.c_str(), I); if (i <= I)l = i;
  }
  text& operator = (const char* str) {
	delete[]_; _ = new char[I + 1]; strncpy(_, str, I); size_t n = strlen(str); l = I < n ? I : n; return *this;
  }
  text& operator = (const std::string& str) {
	delete[]_; _ = new char[I + 1]; strncpy(_, str.c_str(), I);
	size_t n = str.length(); l = I < n ? I : n; return *this;
  }
  text& operator = (const text& str) {
	delete[]_; _ = new char[I + 1]; strcpy(_, str._); l = str.l; return *this;
  }
  template<unsigned char L>
  text& operator = (const text<L>& str) {
	delete[]_; _ = new char[I + 1]; strncpy(_, str.c_str(), I); l = str.length(); return *this;
  }
  const char* c_str() const { return _; }
  const short length() const { return l; }
  char& operator[](unsigned char i) { return _[i]; }
  text& operator += (const char* c) {
	while (*c && l < I) { _[l++] = *c++; } _[I] = 0; return *this;
  }
  text& operator += (const text& t) {
	const char* s = t.c_str(); unsigned char i = 0xff;
	while (s[++i] && l < I) { _[l++] = s[i]; } _[I] = 0; return *this;
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
private: char* _ = new char[I + 1]; unsigned char l = I;
};
template<unsigned char I>
text<I> operator+(const text<I>& t, const char* c) {
  unsigned char l = t.length(); char* s = (char*)t.c_str(); while (*c && l < I) { s[l++] = *c++; } s[l] = 0; return t;
}
template<class T>
struct is_text : std::false_type {};
template<class T>
struct is_text<T[]> : std::false_type {};
template<unsigned char N>
struct is_text<text<N>> : std::true_type {};
