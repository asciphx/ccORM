#ifndef MACROS_HPP
#define MACROS_HPP
#include <string>
#include <tuple>
#include <typeinfo>
#include <type_traits>
#include <chrono>
#include <thread>
#include <vector>
#include <ostream>
#include <memory>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cassert>
#include <cstdarg>
#include <stdexcept>
#include "json.hpp"
#include "./utils/str.h"
namespace orm {
  static const unsigned int HARDWARE_ASYNCHRONOUS = 0x6;//It is best to set the maximum number of threads
  template <class T>
  static typename std::enable_if<std::is_same<T, tm>::value, std::string>::type DuckTyping(const T& _v) {
	std::ostringstream os; os << 20 << (_v.tm_year - 100) << '-' << std::setfill('0') << std::setw(2)
	  << (_v.tm_mon + 1) << '-' << std::setw(2) << _v.tm_mday << ' ' << std::setw(2) << _v.tm_hour << ':'
	  << std::setw(2) << _v.tm_min << ':' << std::setw(2) << _v.tm_sec; return os.str();
  }
  template <class T>
  static inline typename std::enable_if<is_text<T>::value, const char*>::type DuckTyping(const T& _v) { return _v.c_str(); }
  template <class T>
  static inline typename std::enable_if<!std::is_same<T, tm>::value && !is_text<T>::value, T>::type DuckTyping(const T& _v) { return _v; }

  template <typename T>
  static typename std::enable_if<std::is_same<T, tm>::value, void>::type OriginalType(T& _v, const char* s, const json& j) {
	std::string d_; try { j.at(s).get_to(d_); } catch (const std::exception&) {} int year = 0, month = 0, day = 0, hour = 0, min = 0, sec = 0;
	if (sscanf(d_.c_str(), RES_DATE_FORMAT, &year, &month, &day, &hour, &min, &sec) == 6) {
	  _v.tm_year = year - 1900; _v.tm_mon = month - 1; _v.tm_mday = day; _v.tm_hour = hour; _v.tm_min = min; _v.tm_sec = sec;
	}
  }
  template <class T>
  static inline typename std::enable_if<is_text<T>::value, void>::type OriginalType(T& _v, const char* s, const json& j) {
	try { _v = j.at(s); } catch (const std::exception&) {}
  }
  template <typename T>
  static inline typename std::enable_if<!std::is_same<T, tm>::value && !is_text<T>::value, void>::type OriginalType(T& _v, const char* s, const json& j) {
	try { j.at(s).get_to(_v); } catch (const std::exception&) {}
  }
  template <typename T, typename Fn, std::size_t... I>
  inline constexpr void ForEachTuple(T& tuple, Fn&& fn,
	std::index_sequence<I...>) {
	using Expander = int[];
	(void)Expander { ((void)fn(std::get<I>(tuple)), 0)... };
  }
  template <typename T>
  inline constexpr auto Schema() { return std::make_tuple(); }
  template <typename T, typename Fn>
  inline constexpr void ForEachField(T* value, Fn&& fn) {
	constexpr const auto schema = Schema<T>();
	ForEachTuple(schema, [value, &fn](auto field) { fn(value->*(field)); },
	  std::make_index_sequence<std::tuple_size<decltype(schema)>::value>{});
  }
  static unsigned int HARDWARE_CORE = HARDWARE_ASYNCHRONOUS - 1;
  enum TC { EMPTY, PRIMARY_KEY, AUTO_INCREMENT, DEFAULT = 4, NOT_NULL = 8 };//protoSpecs
}
#if 1
#define Inject(T, N) (size_t)(&reinterpret_cast<char const volatile&>(((T*)0)->N))
#define EXP(O) O
#ifdef _MSC_VER
static const char* GetRealType(const char* s) {
  if (s[11] == 118) { return "S"; } if (s[11] == 60) { return s + 7; } return s;
}
#define InjectTS(U, T) GetRealType(typeid(U::T).name())
#define ARGS_HELPER(_,_64,_63,_62,_61,_60,_59,_58,_57,_56,_55,_54,_53,_52,_51,_50,_49,_48,_47,_46,_45,_44,_43,_42,_41,_40,_39,_38,_37,_36,_35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1,N,...) N
#define NUM_ARGS(...) EXP(ARGS_HELPER(0, __VA_ARGS__ ,64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0))
#else
inline const char* GetRealType(const char* s, const char* c) {
  uint8_t l = strLen(s); l > 9 ? l += 3 : l += 2; return c + l;
}
#define InjectTS(U, T) GetRealType(#U,typeid(&U::T).name())
#define ARGS_HELPER(_,_64,_63,_62,_61,_60,_59,_58,_57,_56,_55,_54,_53,_52,_51,_50,_49,_48,_47,_46,_45,_44,_43,_42,_41,_40,_39,_38,_37,_36,_35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1,N,...) N
#define NUM_ARGS(...) ARGS_HELPER(0, __VA_ARGS__ ,64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)
#endif
#define PROTO_1(k)      #k
#define PROTO_2(k,...)  #k, EXP(PROTO_1(__VA_ARGS__))
#define PROTO_3(k,...)  #k, EXP(PROTO_2(__VA_ARGS__))
#define PROTO_4(k,...)  #k, EXP(PROTO_3(__VA_ARGS__))
#define PROTO_5(k,...)  #k, EXP(PROTO_4(__VA_ARGS__))
#define PROTO_6(k,...)  #k, EXP(PROTO_5(__VA_ARGS__))
#define PROTO_7(k,...)  #k, EXP(PROTO_6(__VA_ARGS__))
#define PROTO_8(k,...)  #k, EXP(PROTO_7(__VA_ARGS__))
#define PROTO_9(k,...)  #k, EXP(PROTO_8(__VA_ARGS__))
#define PROTO_10(k,...) #k, EXP(PROTO_9(__VA_ARGS__))
#define PROTO_11(k,...) #k, EXP(PROTO_10(__VA_ARGS__))
#define PROTO_12(k,...) #k, EXP(PROTO_11(__VA_ARGS__))
#define PROTO_13(k,...) #k, EXP(PROTO_12(__VA_ARGS__))
#define PROTO_14(k,...) #k, EXP(PROTO_13(__VA_ARGS__))
#define PROTO_15(k,...) #k, EXP(PROTO_14(__VA_ARGS__))
#define PROTO_16(k,...) #k, EXP(PROTO_15(__VA_ARGS__))
#define PROTO_17(k,...) #k, EXP(PROTO_16(__VA_ARGS__))
#define PROTO_18(k,...) #k, EXP(PROTO_17(__VA_ARGS__))
#define PROTO_19(k,...) #k, EXP(PROTO_18(__VA_ARGS__))
#define PROTO_20(k,...) #k, EXP(PROTO_19(__VA_ARGS__))
#define PROTO_21(k,...) #k, EXP(PROTO_20(__VA_ARGS__))
#define PROTO_22(k,...) #k, EXP(PROTO_21(__VA_ARGS__))
#define PROTO_23(k,...) #k, EXP(PROTO_22(__VA_ARGS__))
#define PROTO_24(k,...) #k, EXP(PROTO_23(__VA_ARGS__))
#define PROTO_25(k,...) #k, EXP(PROTO_24(__VA_ARGS__))
#define PROTO_26(k,...) #k, EXP(PROTO_25(__VA_ARGS__))
#define PROTO_27(k,...) #k, EXP(PROTO_26(__VA_ARGS__))
#define PROTO_28(k,...) #k, EXP(PROTO_27(__VA_ARGS__))
#define PROTO_29(k,...) #k, EXP(PROTO_28(__VA_ARGS__))
#define PROTO_30(k,...) #k, EXP(PROTO_29(__VA_ARGS__))
#define PROTO_31(k,...) #k, EXP(PROTO_30(__VA_ARGS__))
#define PROTO_32(k,...) #k, EXP(PROTO_31(__VA_ARGS__))
#define PROTO_N1(N,...) EXP(PROTO_##N(__VA_ARGS__))
#define PROTO_N(N,...) PROTO_N1(N,__VA_ARGS__)
#define TYPE_1(o,k)      InjectTS(o, k)
#define TYPE_2(o,k,...)  InjectTS(o, k), EXP(TYPE_1(o,__VA_ARGS__))
#define TYPE_3(o,k,...)  InjectTS(o, k), EXP(TYPE_2(o,__VA_ARGS__))
#define TYPE_4(o,k,...)  InjectTS(o, k), EXP(TYPE_3(o,__VA_ARGS__))
#define TYPE_5(o,k,...)  InjectTS(o, k), EXP(TYPE_4(o,__VA_ARGS__))
#define TYPE_6(o,k,...)  InjectTS(o, k), EXP(TYPE_5(o,__VA_ARGS__))
#define TYPE_7(o,k,...)  InjectTS(o, k), EXP(TYPE_6(o,__VA_ARGS__))
#define TYPE_8(o,k,...)  InjectTS(o, k), EXP(TYPE_7(o,__VA_ARGS__))
#define TYPE_9(o,k,...)  InjectTS(o, k), EXP(TYPE_8(o,__VA_ARGS__))
#define TYPE_10(o,k,...) InjectTS(o, k), EXP(TYPE_9(o,__VA_ARGS__))
#define TYPE_11(o,k,...) InjectTS(o, k), EXP(TYPE_10(o,__VA_ARGS__))
#define TYPE_12(o,k,...) InjectTS(o, k), EXP(TYPE_11(o,__VA_ARGS__))
#define TYPE_13(o,k,...) InjectTS(o, k), EXP(TYPE_12(o,__VA_ARGS__))
#define TYPE_14(o,k,...) InjectTS(o, k), EXP(TYPE_13(o,__VA_ARGS__))
#define TYPE_15(o,k,...) InjectTS(o, k), EXP(TYPE_14(o,__VA_ARGS__))
#define TYPE_16(o,k,...) InjectTS(o, k), EXP(TYPE_15(o,__VA_ARGS__))
#define TYPE_17(o,k,...) InjectTS(o, k), EXP(TYPE_16(o,__VA_ARGS__))
#define TYPE_18(o,k,...) InjectTS(o, k), EXP(TYPE_17(o,__VA_ARGS__))
#define TYPE_19(o,k,...) InjectTS(o, k), EXP(TYPE_18(o,__VA_ARGS__))
#define TYPE_20(o,k,...) InjectTS(o, k), EXP(TYPE_19(o,__VA_ARGS__))
#define TYPE_21(o,k,...) InjectTS(o, k), EXP(TYPE_20(o,__VA_ARGS__))
#define TYPE_22(o,k,...) InjectTS(o, k), EXP(TYPE_21(o,__VA_ARGS__))
#define TYPE_23(o,k,...) InjectTS(o, k), EXP(TYPE_22(o,__VA_ARGS__))
#define TYPE_24(o,k,...) InjectTS(o, k), EXP(TYPE_23(o,__VA_ARGS__))
#define TYPE_25(o,k,...) InjectTS(o, k), EXP(TYPE_24(o,__VA_ARGS__))
#define TYPE_26(o,k,...) InjectTS(o, k), EXP(TYPE_25(o,__VA_ARGS__))
#define TYPE_27(o,k,...) InjectTS(o, k), EXP(TYPE_26(o,__VA_ARGS__))
#define TYPE_28(o,k,...) InjectTS(o, k), EXP(TYPE_27(o,__VA_ARGS__))
#define TYPE_29(o,k,...) InjectTS(o, k), EXP(TYPE_28(o,__VA_ARGS__))
#define TYPE_30(o,k,...) InjectTS(o, k), EXP(TYPE_29(o,__VA_ARGS__))
#define TYPE_31(o,k,...) InjectTS(o, k), EXP(TYPE_30(o,__VA_ARGS__))
#define TYPE_32(o,k,...) InjectTS(o, k), EXP(TYPE_31(o,__VA_ARGS__))
#define TYPE_N1(o,N,...) EXP(TYPE_##N(o,__VA_ARGS__))
#define TYPE_N(o,N,...) TYPE_N1(o,N,__VA_ARGS__)
#define OFFSET_1(o,k)      Inject(o, k)
#define OFFSET_2(o,k,...)  Inject(o, k), EXP(OFFSET_1(o,__VA_ARGS__))
#define OFFSET_3(o,k,...)  Inject(o, k), EXP(OFFSET_2(o,__VA_ARGS__))
#define OFFSET_4(o,k,...)  Inject(o, k), EXP(OFFSET_3(o,__VA_ARGS__))
#define OFFSET_5(o,k,...)  Inject(o, k), EXP(OFFSET_4(o,__VA_ARGS__))
#define OFFSET_6(o,k,...)  Inject(o, k), EXP(OFFSET_5(o,__VA_ARGS__))
#define OFFSET_7(o,k,...)  Inject(o, k), EXP(OFFSET_6(o,__VA_ARGS__))
#define OFFSET_8(o,k,...)  Inject(o, k), EXP(OFFSET_7(o,__VA_ARGS__))
#define OFFSET_9(o,k,...)  Inject(o, k), EXP(OFFSET_8(o,__VA_ARGS__))
#define OFFSET_10(o,k,...) Inject(o, k), EXP(OFFSET_9(o,__VA_ARGS__))
#define OFFSET_11(o,k,...) Inject(o, k), EXP(OFFSET_10(o,__VA_ARGS__))
#define OFFSET_12(o,k,...) Inject(o, k), EXP(OFFSET_11(o,__VA_ARGS__))
#define OFFSET_13(o,k,...) Inject(o, k), EXP(OFFSET_12(o,__VA_ARGS__))
#define OFFSET_14(o,k,...) Inject(o, k), EXP(OFFSET_13(o,__VA_ARGS__))
#define OFFSET_15(o,k,...) Inject(o, k), EXP(OFFSET_14(o,__VA_ARGS__))
#define OFFSET_16(o,k,...) Inject(o, k), EXP(OFFSET_15(o,__VA_ARGS__))
#define OFFSET_17(o,k,...) Inject(o, k), EXP(OFFSET_16(o,__VA_ARGS__))
#define OFFSET_18(o,k,...) Inject(o, k), EXP(OFFSET_17(o,__VA_ARGS__))
#define OFFSET_19(o,k,...) Inject(o, k), EXP(OFFSET_18(o,__VA_ARGS__))
#define OFFSET_20(o,k,...) Inject(o, k), EXP(OFFSET_19(o,__VA_ARGS__))
#define OFFSET_21(o,k,...) Inject(o, k), EXP(OFFSET_20(o,__VA_ARGS__))
#define OFFSET_22(o,k,...) Inject(o, k), EXP(OFFSET_21(o,__VA_ARGS__))
#define OFFSET_23(o,k,...) Inject(o, k), EXP(OFFSET_22(o,__VA_ARGS__))
#define OFFSET_24(o,k,...) Inject(o, k), EXP(OFFSET_23(o,__VA_ARGS__))
#define OFFSET_25(o,k,...) Inject(o, k), EXP(OFFSET_24(o,__VA_ARGS__))
#define OFFSET_26(o,k,...) Inject(o, k), EXP(OFFSET_25(o,__VA_ARGS__))
#define OFFSET_27(o,k,...) Inject(o, k), EXP(OFFSET_26(o,__VA_ARGS__))
#define OFFSET_28(o,k,...) Inject(o, k), EXP(OFFSET_27(o,__VA_ARGS__))
#define OFFSET_29(o,k,...) Inject(o, k), EXP(OFFSET_28(o,__VA_ARGS__))
#define OFFSET_30(o,k,...) Inject(o, k), EXP(OFFSET_29(o,__VA_ARGS__))
#define OFFSET_31(o,k,...) Inject(o, k), EXP(OFFSET_30(o,__VA_ARGS__))
#define OFFSET_32(o,k,...) Inject(o, k), EXP(OFFSET_31(o,__VA_ARGS__))
#define OFFSET_N1(o,N,...) EXP(OFFSET_##N(o,__VA_ARGS__))
#define OFFSET_N(o,N,...) OFFSET_N1(o,N,__VA_ARGS__)
//REGIST(Tab, id, kg, date);
#endif
#define REGIST(o,...)\
 template<> const uint8_t orm::Table<o>::_size_ = NUM_ARGS(__VA_ARGS__);\
 template<> uint8_t orm::Table<o>::_tc_[NUM_ARGS(__VA_ARGS__)]={};\
 template<> const size_t orm::Table<o>::_o$[NUM_ARGS(__VA_ARGS__)]={ OFFSET_N(o,NUM_ARGS(__VA_ARGS__),__VA_ARGS__) };\
 template<> const char* orm::Table<o>::_def_[NUM_ARGS(__VA_ARGS__)]={};\
 template<> const char* orm::Table<o>::_[NUM_ARGS(__VA_ARGS__)] = { TYPE_N(o,NUM_ARGS(__VA_ARGS__),__VA_ARGS__) };\
 template<> const char* orm::Table<o>::$[NUM_ARGS(__VA_ARGS__)] = { PROTO_N(NUM_ARGS(__VA_ARGS__),__VA_ARGS__) };

#define PRO_1(t,k)      const char* t::$##k = #k;
#define PRO_2(t,k,...)  const char* t::$##k = #k; EXP(PRO_1(t,__VA_ARGS__))
#define PRO_3(t,k,...)  const char* t::$##k = #k; EXP(PRO_2(t,__VA_ARGS__))
#define PRO_4(t,k,...)  const char* t::$##k = #k; EXP(PRO_3(t,__VA_ARGS__))
#define PRO_5(t,k,...)  const char* t::$##k = #k; EXP(PRO_4(t,__VA_ARGS__))
#define PRO_6(t,k,...)  const char* t::$##k = #k; EXP(PRO_5(t,__VA_ARGS__))
#define PRO_7(t,k,...)  const char* t::$##k = #k; EXP(PRO_6(t,__VA_ARGS__))
#define PRO_8(t,k,...)  const char* t::$##k = #k; EXP(PRO_7(t,__VA_ARGS__))
#define PRO_9(t,k,...)  const char* t::$##k = #k; EXP(PRO_8(t,__VA_ARGS__))
#define PRO_10(t,k,...) const char* t::$##k = #k; EXP(PRO_9(t,__VA_ARGS__))
#define PRO_11(t,k,...) const char* t::$##k = #k; EXP(PRO_10(t,__VA_ARGS__))
#define PRO_12(t,k,...) const char* t::$##k = #k; EXP(PRO_11(t,__VA_ARGS__))
#define PRO_13(t,k,...) const char* t::$##k = #k; EXP(PRO_12(t,__VA_ARGS__))
#define PRO_14(t,k,...) const char* t::$##k = #k; EXP(PRO_13(t,__VA_ARGS__))
#define PRO_15(t,k,...) const char* t::$##k = #k; EXP(PRO_14(t,__VA_ARGS__))
#define PRO_16(t,k,...) const char* t::$##k = #k; EXP(PRO_15(t,__VA_ARGS__))
#define PRO_17(t,k,...) const char* t::$##k = #k; EXP(PRO_16(t,__VA_ARGS__))
#define PRO_18(t,k,...) const char* t::$##k = #k; EXP(PRO_17(t,__VA_ARGS__))
#define PRO_19(t,k,...) const char* t::$##k = #k; EXP(PRO_18(t,__VA_ARGS__))
#define PRO_20(t,k,...) const char* t::$##k = #k; EXP(PRO_19(t,__VA_ARGS__))
#define PRO_21(t,k,...) const char* t::$##k = #k; EXP(PRO_20(t,__VA_ARGS__))
#define PRO_22(t,k,...) const char* t::$##k = #k; EXP(PRO_21(t,__VA_ARGS__))
#define PRO_23(t,k,...) const char* t::$##k = #k; EXP(PRO_22(t,__VA_ARGS__))
#define PRO_24(t,k,...) const char* t::$##k = #k; EXP(PRO_23(t,__VA_ARGS__))
#define PRO_25(t,k,...) const char* t::$##k = #k; EXP(PRO_24(t,__VA_ARGS__))
#define PRO_26(t,k,...) const char* t::$##k = #k; EXP(PRO_25(t,__VA_ARGS__))
#define PRO_27(t,k,...) const char* t::$##k = #k; EXP(PRO_26(t,__VA_ARGS__))
#define PRO_28(t,k,...) const char* t::$##k = #k; EXP(PRO_27(t,__VA_ARGS__))
#define PRO_29(t,k,...) const char* t::$##k = #k; EXP(PRO_28(t,__VA_ARGS__))
#define PRO_30(t,k,...) const char* t::$##k = #k; EXP(PRO_29(t,__VA_ARGS__))
#define PRO_31(t,k,...) const char* t::$##k = #k; EXP(PRO_30(t,__VA_ARGS__))
#define PRO_32(t,k,...) const char* t::$##k = #k; EXP(PRO_31(t,__VA_ARGS__))
#define PRO_N(t,N,...) EXP(PRO_##N(t,__VA_ARGS__))
#define PROS(t,N,...) PRO_N(t,N,__VA_ARGS__)

#define STAR_1(o,k)      &o::k
#define STAR_2(o,k,...)  &o::k, EXP(STAR_1(o,__VA_ARGS__))
#define STAR_3(o,k,...)  &o::k, EXP(STAR_2(o,__VA_ARGS__))
#define STAR_4(o,k,...)  &o::k, EXP(STAR_3(o,__VA_ARGS__))
#define STAR_5(o,k,...)  &o::k, EXP(STAR_4(o,__VA_ARGS__))
#define STAR_6(o,k,...)  &o::k, EXP(STAR_5(o,__VA_ARGS__))
#define STAR_7(o,k,...)  &o::k, EXP(STAR_6(o,__VA_ARGS__))
#define STAR_8(o,k,...)  &o::k, EXP(STAR_7(o,__VA_ARGS__))
#define STAR_9(o,k,...)  &o::k, EXP(STAR_8(o,__VA_ARGS__))
#define STAR_10(o,k,...) &o::k, EXP(STAR_9(o,__VA_ARGS__))
#define STAR_11(o,k,...) &o::k, EXP(STAR_10(o,__VA_ARGS__))
#define STAR_12(o,k,...) &o::k, EXP(STAR_11(o,__VA_ARGS__))
#define STAR_13(o,k,...) &o::k, EXP(STAR_12(o,__VA_ARGS__))
#define STAR_14(o,k,...) &o::k, EXP(STAR_13(o,__VA_ARGS__))
#define STAR_15(o,k,...) &o::k, EXP(STAR_14(o,__VA_ARGS__))
#define STAR_16(o,k,...) &o::k, EXP(STAR_15(o,__VA_ARGS__))
#define STAR_17(o,k,...) &o::k, EXP(STAR_16(o,__VA_ARGS__))
#define STAR_18(o,k,...) &o::k, EXP(STAR_17(o,__VA_ARGS__))
#define STAR_19(o,k,...) &o::k, EXP(STAR_18(o,__VA_ARGS__))
#define STAR_20(o,k,...) &o::k, EXP(STAR_19(o,__VA_ARGS__))
#define STAR_21(o,k,...) &o::k, EXP(STAR_20(o,__VA_ARGS__))
#define STAR_22(o,k,...) &o::k, EXP(STAR_21(o,__VA_ARGS__))
#define STAR_23(o,k,...) &o::k, EXP(STAR_22(o,__VA_ARGS__))
#define STAR_24(o,k,...) &o::k, EXP(STAR_23(o,__VA_ARGS__))
#define STAR_25(o,k,...) &o::k, EXP(STAR_24(o,__VA_ARGS__))
#define STAR_26(o,k,...) &o::k, EXP(STAR_25(o,__VA_ARGS__))
#define STAR_27(o,k,...) &o::k, EXP(STAR_26(o,__VA_ARGS__))
#define STAR_28(o,k,...) &o::k, EXP(STAR_27(o,__VA_ARGS__))
#define STAR_29(o,k,...) &o::k, EXP(STAR_28(o,__VA_ARGS__))
#define STAR_30(o,k,...) &o::k, EXP(STAR_29(o,__VA_ARGS__))
#define STAR_31(o,k,...) &o::k, EXP(STAR_30(o,__VA_ARGS__))
#define STAR_32(o,k,...) &o::k, EXP(STAR_31(o,__VA_ARGS__))
#define STARS_N(o,N,...) EXP(STAR_##N(o,__VA_ARGS__))
#define STARS(o,N,...) STARS_N(o,N,__VA_ARGS__)

#define REGISTER_TABLE(o)\
	template<> Sql<o>* orm::Table<o>::__[HARDWARE_ASYNCHRONOUS]={};\
	template<> uint8_t orm::Table<o>::_idex = 0;\
	template<> std::string orm::Table<o>::_create_ = "CREATE TABLE IF NOT EXISTS "#o" (\n";\
	template<> const std::string orm::Table<o>::_drop_ = "DROP TABLE IF EXISTS "+toSqlLowerCase(#o";");\
	template<> const std::string orm::Table<o>::_name = toSqlLowerCase(#o);\
	template<> bool orm::Table<o>::_create_need = true;

#define CONSTRUCT(o,...)\
        PROS(o,NUM_ARGS(__VA_ARGS__),__VA_ARGS__)\
        REGIST(o, __VA_ARGS__)\
        REGISTER_TABLE(o)\
    template <> inline constexpr auto orm::Schema<o>() {\
      return std::make_tuple(STARS(o,NUM_ARGS(__VA_ARGS__), __VA_ARGS__));\
    }
//----------------------------------------------------------
#define PTR_2(k,t,v)      _tc_[k] = t; _def_[k] = v;
#define PTR_4(k,t,v,...)  _tc_[k] = t; _def_[k] = v; EXP(PTR_2(k+1,__VA_ARGS__))
#define PTR_6(k,t,v,...)  _tc_[k] = t; _def_[k] = v; EXP(PTR_4(k+1,__VA_ARGS__))
#define PTR_8(k,t,v,...)  _tc_[k] = t; _def_[k] = v; EXP(PTR_6(k+1,__VA_ARGS__))
#define PTR_10(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_8(k+1,__VA_ARGS__))
#define PTR_12(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_10(k+1,__VA_ARGS__))
#define PTR_14(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_12(k+1,__VA_ARGS__))
#define PTR_16(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_14(k+1,__VA_ARGS__))
#define PTR_18(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_16(k+1,__VA_ARGS__))
#define PTR_20(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_18(k+1,__VA_ARGS__))
#define PTR_22(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_20(k+1,__VA_ARGS__))
#define PTR_24(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_22(k+1,__VA_ARGS__))
#define PTR_26(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_24(k+1,__VA_ARGS__))
#define PTR_28(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_26(k+1,__VA_ARGS__))
#define PTR_30(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_28(k+1,__VA_ARGS__))
#define PTR_32(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_30(k+1,__VA_ARGS__))
#define PTR_34(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_32(k+1,__VA_ARGS__))
#define PTR_36(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_34(k+1,__VA_ARGS__))
#define PTR_38(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_36(k+1,__VA_ARGS__))
#define PTR_40(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_38(k+1,__VA_ARGS__))
#define PTR_42(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_40(k+1,__VA_ARGS__))
#define PTR_44(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_42(k+1,__VA_ARGS__))
#define PTR_46(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_44(k+1,__VA_ARGS__))
#define PTR_48(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_46(k+1,__VA_ARGS__))
#define PTR_50(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_48(k+1,__VA_ARGS__))
#define PTR_52(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_50(k+1,__VA_ARGS__))
#define PTR_54(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_52(k+1,__VA_ARGS__))
#define PTR_56(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_54(k+1,__VA_ARGS__))
#define PTR_58(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_56(k+1,__VA_ARGS__))
#define PTR_60(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_58(k+1,__VA_ARGS__))
#define PTR_62(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_60(k+1,__VA_ARGS__))
#define PTR_64(k,t,v,...) _tc_[k] = t; _def_[k] = v; EXP(PTR_62(k+1,__VA_ARGS__))
#define PTRS_N(N,...) EXP(PTR_##N(0,__VA_ARGS__))
#define PTRS(N,...) PTRS_N(N,__VA_ARGS__)
//`1;`->加粗，`4`->下划线，`0`->还原,`m`<=>`\033[`
#define RGB_BLACK  	 "\033[30m"
#define RGB_RED  	 "\033[31m"
#define RGB_GREEN    "\033[32m"
#define RGB_YELLOW   "\033[33m"
#define RGB_BLUE   	 "\033[34m"
#define RGB_MAGENTA  "\033[35m"
#define RGB_AZURE    "\033[36m"
#define RGB_NULL 	 "\033[0m"
//regist PROPERTY,主键规定只能在第一个位置，同时于此也允许没有主键（不然不好处理）
#define REGIST_PROTO(o,...)\
  template<> void orm::Table<o>::Init(){ PTRS(NUM_ARGS(__VA_ARGS__), __VA_ARGS__)\
    bool b=true;if(_tc_[0] & TC::PRIMARY_KEY){b=false;}for(char i=1;i<NUM_ARGS(__VA_ARGS__);++i){\
       if(_tc_[i] & TC::PRIMARY_KEY){ if(b){b=false;\
throw std::runtime_error(std::string("\033[1;34m["#o"]\033[31;4m primary key must be in the first position!\n\033[0m"));}\
else{ throw std::runtime_error(std::string("\033[1;34m["#o"]\033[31;4m can't have multiple primary keys!\n\033[0m"));} }}}

#define FIELD_1(k)      static const char* $##k;
#define FIELD_2(k,...)  static const char* $##k; EXP(FIELD_1(__VA_ARGS__))
#define FIELD_3(k,...)  static const char* $##k; EXP(FIELD_2(__VA_ARGS__))
#define FIELD_4(k,...)  static const char* $##k; EXP(FIELD_3(__VA_ARGS__))
#define FIELD_5(k,...)  static const char* $##k; EXP(FIELD_4(__VA_ARGS__))
#define FIELD_6(k,...)  static const char* $##k; EXP(FIELD_5(__VA_ARGS__))
#define FIELD_7(k,...)  static const char* $##k; EXP(FIELD_6(__VA_ARGS__))
#define FIELD_8(k,...)  static const char* $##k; EXP(FIELD_7(__VA_ARGS__))
#define FIELD_9(k,...)  static const char* $##k; EXP(FIELD_8(__VA_ARGS__))
#define FIELD_10(k,...) static const char* $##k; EXP(FIELD_9(__VA_ARGS__))
#define FIELD_11(k,...) static const char* $##k; EXP(FIELD_10(__VA_ARGS__))
#define FIELD_12(k,...) static const char* $##k; EXP(FIELD_11(__VA_ARGS__))
#define FIELD_13(k,...) static const char* $##k; EXP(FIELD_12(__VA_ARGS__))
#define FIELD_14(k,...) static const char* $##k; EXP(FIELD_13(__VA_ARGS__))
#define FIELD_15(k,...) static const char* $##k; EXP(FIELD_14(__VA_ARGS__))
#define FIELD_16(k,...) static const char* $##k; EXP(FIELD_15(__VA_ARGS__))
#define FIELD_17(k,...) static const char* $##k; EXP(FIELD_16(__VA_ARGS__))
#define FIELD_18(k,...) static const char* $##k; EXP(FIELD_17(__VA_ARGS__))
#define FIELD_19(k,...) static const char* $##k; EXP(FIELD_18(__VA_ARGS__))
#define FIELD_20(k,...) static const char* $##k; EXP(FIELD_19(__VA_ARGS__))
#define FIELD_21(k,...) static const char* $##k; EXP(FIELD_20(__VA_ARGS__))
#define FIELD_22(k,...) static const char* $##k; EXP(FIELD_21(__VA_ARGS__))
#define FIELD_23(k,...) static const char* $##k; EXP(FIELD_22(__VA_ARGS__))
#define FIELD_24(k,...) static const char* $##k; EXP(FIELD_23(__VA_ARGS__))
#define FIELD_25(k,...) static const char* $##k; EXP(FIELD_24(__VA_ARGS__))
#define FIELD_26(k,...) static const char* $##k; EXP(FIELD_25(__VA_ARGS__))
#define FIELD_27(k,...) static const char* $##k; EXP(FIELD_26(__VA_ARGS__))
#define FIELD_28(k,...) static const char* $##k; EXP(FIELD_27(__VA_ARGS__))
#define FIELD_29(k,...) static const char* $##k; EXP(FIELD_28(__VA_ARGS__))
#define FIELD_30(k,...) static const char* $##k; EXP(FIELD_29(__VA_ARGS__))
#define FIELD_31(k,...) static const char* $##k; EXP(FIELD_30(__VA_ARGS__))
#define FIELD_32(k,...) static const char* $##k; EXP(FIELD_31(__VA_ARGS__))
#define FIELD_N1(N,...) EXP(FIELD_##N(__VA_ARGS__))
#define FIELD_N(N,...) FIELD_N1(N,__VA_ARGS__)

#define COL_1(o,k)      j[#k].operator=(orm::DuckTyping(o.k));
#define COL_2(o,k,...)  j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_1(o,__VA_ARGS__))
#define COL_3(o,k,...)  j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_2(o,__VA_ARGS__))
#define COL_4(o,k,...)  j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_3(o,__VA_ARGS__))
#define COL_5(o,k,...)  j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_4(o,__VA_ARGS__))
#define COL_6(o,k,...)  j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_5(o,__VA_ARGS__))
#define COL_7(o,k,...)  j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_6(o,__VA_ARGS__))
#define COL_8(o,k,...)  j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_7(o,__VA_ARGS__))
#define COL_9(o,k,...)  j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_8(o,__VA_ARGS__))
#define COL_10(o,k,...) j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_9(o,__VA_ARGS__))
#define COL_11(o,k,...) j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_10(o,__VA_ARGS__))
#define COL_12(o,k,...) j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_11(o,__VA_ARGS__))
#define COL_13(o,k,...) j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_12(o,__VA_ARGS__))
#define COL_14(o,k,...) j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_13(o,__VA_ARGS__))
#define COL_15(o,k,...) j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_14(o,__VA_ARGS__))
#define COL_16(o,k,...) j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_15(o,__VA_ARGS__))
#define COL_17(o,k,...) j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_16(o,__VA_ARGS__))
#define COL_18(o,k,...) j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_17(o,__VA_ARGS__))
#define COL_19(o,k,...) j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_18(o,__VA_ARGS__))
#define COL_20(o,k,...) j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_19(o,__VA_ARGS__))
#define COL_21(o,k,...) j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_20(o,__VA_ARGS__))
#define COL_22(o,k,...) j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_21(o,__VA_ARGS__))
#define COL_23(o,k,...) j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_22(o,__VA_ARGS__))
#define COL_24(o,k,...) j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_23(o,__VA_ARGS__))
#define COL_25(o,k,...) j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_24(o,__VA_ARGS__))
#define COL_26(o,k,...) j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_25(o,__VA_ARGS__))
#define COL_27(o,k,...) j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_26(o,__VA_ARGS__))
#define COL_28(o,k,...) j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_27(o,__VA_ARGS__))
#define COL_29(o,k,...) j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_28(o,__VA_ARGS__))
#define COL_30(o,k,...) j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_29(o,__VA_ARGS__))
#define COL_31(o,k,...) j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_30(o,__VA_ARGS__))
#define COL_32(o,k,...) j[#k].operator=(orm::DuckTyping(o.k)), EXP(COL_31(o,__VA_ARGS__))
#define COL_N1(o,N,...) EXP(COL_##N(o,__VA_ARGS__))
#define COL_N(o,N,...) COL_N1(o,N,__VA_ARGS__)

#define ATTR_1(o,k)      orm::OriginalType(o.k,#k,j);
#define ATTR_2(o,k,...)  orm::OriginalType(o.k,#k,j), EXP(ATTR_1(o,__VA_ARGS__))
#define ATTR_3(o,k,...)  orm::OriginalType(o.k,#k,j), EXP(ATTR_2(o,__VA_ARGS__))
#define ATTR_4(o,k,...)  orm::OriginalType(o.k,#k,j), EXP(ATTR_3(o,__VA_ARGS__))
#define ATTR_5(o,k,...)  orm::OriginalType(o.k,#k,j), EXP(ATTR_4(o,__VA_ARGS__))
#define ATTR_6(o,k,...)  orm::OriginalType(o.k,#k,j), EXP(ATTR_5(o,__VA_ARGS__))
#define ATTR_7(o,k,...)  orm::OriginalType(o.k,#k,j), EXP(ATTR_6(o,__VA_ARGS__))
#define ATTR_8(o,k,...)  orm::OriginalType(o.k,#k,j), EXP(ATTR_7(o,__VA_ARGS__))
#define ATTR_9(o,k,...)  orm::OriginalType(o.k,#k,j), EXP(ATTR_8(o,__VA_ARGS__))
#define ATTR_10(o,k,...) orm::OriginalType(o.k,#k,j), EXP(ATTR_9(o,__VA_ARGS__))
#define ATTR_11(o,k,...) orm::OriginalType(o.k,#k,j), EXP(ATTR_10(o,__VA_ARGS__))
#define ATTR_12(o,k,...) orm::OriginalType(o.k,#k,j), EXP(ATTR_11(o,__VA_ARGS__))
#define ATTR_13(o,k,...) orm::OriginalType(o.k,#k,j), EXP(ATTR_12(o,__VA_ARGS__))
#define ATTR_14(o,k,...) orm::OriginalType(o.k,#k,j), EXP(ATTR_13(o,__VA_ARGS__))
#define ATTR_15(o,k,...) orm::OriginalType(o.k,#k,j), EXP(ATTR_14(o,__VA_ARGS__))
#define ATTR_16(o,k,...) orm::OriginalType(o.k,#k,j), EXP(ATTR_15(o,__VA_ARGS__))
#define ATTR_17(o,k,...) orm::OriginalType(o.k,#k,j), EXP(ATTR_16(o,__VA_ARGS__))
#define ATTR_18(o,k,...) orm::OriginalType(o.k,#k,j), EXP(ATTR_17(o,__VA_ARGS__))
#define ATTR_19(o,k,...) orm::OriginalType(o.k,#k,j), EXP(ATTR_18(o,__VA_ARGS__))
#define ATTR_20(o,k,...) orm::OriginalType(o.k,#k,j), EXP(ATTR_19(o,__VA_ARGS__))
#define ATTR_21(o,k,...) orm::OriginalType(o.k,#k,j), EXP(ATTR_20(o,__VA_ARGS__))
#define ATTR_22(o,k,...) orm::OriginalType(o.k,#k,j), EXP(ATTR_21(o,__VA_ARGS__))
#define ATTR_23(o,k,...) orm::OriginalType(o.k,#k,j), EXP(ATTR_22(o,__VA_ARGS__))
#define ATTR_24(o,k,...) orm::OriginalType(o.k,#k,j), EXP(ATTR_23(o,__VA_ARGS__))
#define ATTR_25(o,k,...) orm::OriginalType(o.k,#k,j), EXP(ATTR_24(o,__VA_ARGS__))
#define ATTR_26(o,k,...) orm::OriginalType(o.k,#k,j), EXP(ATTR_25(o,__VA_ARGS__))
#define ATTR_27(o,k,...) orm::OriginalType(o.k,#k,j), EXP(ATTR_26(o,__VA_ARGS__))
#define ATTR_28(o,k,...) orm::OriginalType(o.k,#k,j), EXP(ATTR_27(o,__VA_ARGS__))
#define ATTR_29(o,k,...) orm::OriginalType(o.k,#k,j), EXP(ATTR_28(o,__VA_ARGS__))
#define ATTR_30(o,k,...) orm::OriginalType(o.k,#k,j), EXP(ATTR_29(o,__VA_ARGS__))
#define ATTR_31(o,k,...) orm::OriginalType(o.k,#k,j), EXP(ATTR_30(o,__VA_ARGS__))
#define ATTR_32(o,k,...) orm::OriginalType(o.k,#k,j), EXP(ATTR_31(o,__VA_ARGS__))
#define ATTR_N1(o,N,...) EXP(ATTR_##N(o,__VA_ARGS__))
#define ATTR_N(o,N,...) ATTR_N1(o,N,__VA_ARGS__)
#define ATTRS(o,...)\
friend void to_json(json& j, const o& f) { COL_N(f,NUM_ARGS(__VA_ARGS__),__VA_ARGS__) }\
friend void from_json(const json& j, o& f) { ATTR_N(f,NUM_ARGS(__VA_ARGS__),__VA_ARGS__) }

#define FIELD(o,...) ATTRS(o, __VA_ARGS__)\
public: FIELD_N(NUM_ARGS(__VA_ARGS__),__VA_ARGS__)
#endif
