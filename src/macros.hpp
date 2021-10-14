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
#include <map>
#include <memory>
#include <cstdarg>
#include "json.hpp"
#include "./utils/str.h"
namespace orm {
  static const unsigned int HARDWARE_ASYNCHRONOUS = 0x6;//It is best to set the maximum number of threads
  template <class T>
  static typename std::enable_if<std::is_same<T, tm>::value, std::string>::type DuckTyping(const T& _v) {
    std::ostringstream os; os << std::setfill('0') << std::setw(4) << (_v.tm_year + 1900)
      << '-' << std::setw(2) << (_v.tm_mon + 1) << '-' << std::setw(2) << _v.tm_mday << ' ' << std::setw(2)
      << _v.tm_hour << ':' << std::setw(2) << _v.tm_min << ':' << std::setw(2) << _v.tm_sec;
#ifdef _WIN32
    return os.str();
#else
    return os.str().replace(1, 1, "");
#endif
  }
  template <class T>
  static inline typename std::enable_if<!std::is_same<T, tm>::value, T>::type DuckTyping(const T& _v) { return _v; }

  template <typename T>
  static typename std::enable_if<std::is_same<T, tm>::value, void>::type OriginalType(T& _v, const char* str, const json& j) {
    std::string d_; j.at(str).get_to(d_); int year = 0, month = 0, day = 0, hour = 0, min = 0, sec = 0;
    if (sscanf(d_.c_str(), RES_DATE_FORMAT, &year, &month, &day, &hour, &min, &sec) == 6) {
      _v.tm_year = year - 1900; _v.tm_mon = month - 1; _v.tm_mday = day; _v.tm_hour = hour; _v.tm_min = min; _v.tm_sec = sec;
    }
  }
  template <typename T>
  static inline typename std::enable_if<!std::is_same<T, tm>::value, void>::type OriginalType(T& _v, const char* str, const json& j) {
    j.at(str).get_to(_v);
  }
  template <typename Fn, typename Tuple, std::size_t... I>
  inline constexpr void ForEachTuple(Tuple& tuple, Fn& fn,
    std::index_sequence<I...>) {
    using Expander = int[];
    (void)Expander { 0, ((void)fn(std::get<I>(tuple)), 0)... };
  }
  template <typename Fn, typename Tuple>
  inline constexpr void ForEachTuple(Tuple&& tuple, Fn&& fn) {
    ForEachTuple(tuple, fn,
      std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>::value>{});
  }
  template <typename T>
  inline constexpr auto Schema() { return std::make_tuple(); }
  template <typename T, typename Fn>
  inline constexpr void ForEachField(T* value, Fn&& fn) {
    constexpr auto struct_schema = Schema<std::decay_t<T>>();
    static_assert(std::tuple_size<decltype(struct_schema)>::value != 0,
      "Schema tuples, like ((&T::field, field_name), ...)");
    ForEachTuple(struct_schema, [value, &fn](auto field_schema) {
      fn(value->*(std::get<0>(field_schema)), std::get<1>(field_schema));
      });
  }
  static unsigned int HARDWARE_CORE = HARDWARE_ASYNCHRONOUS - 1;
  enum TC { EMPTY, PRIMARY_KEY, AUTO_INCREMENT, DEFAULT = 4, NOT_NULL = 8 };//protoSpecs
  static bool is_PRIMARY_KEY(TC specs) { return (specs & TC::PRIMARY_KEY); }
  static bool is_NOT_NULL(TC specs) { return (specs & TC::NOT_NULL); }
  static bool is_DEFAULT(TC specs) { return (specs & TC::DEFAULT); }
  static bool is_AUTOINCREMENT(TC specs) { return (specs & TC::AUTO_INCREMENT); }
}
std::string formattedString(const char* f, ...) {
  std::string s(128, 0); va_list vl, backup; va_start(vl, f); va_copy(backup, vl);
  auto r = vsnprintf((char*)s.data(), s.size(), f, backup); va_end(backup);
  if ((r >= 0) && ((std::string::size_type)r < s.size())) s.resize(r); else while (true) {
    if (r < 0) s.resize(s.size() * 2); else s.resize(r + 1);
    va_copy(backup, vl); auto r = vsnprintf((char*)s.data(), s.size(), f, backup); va_end(backup);
    if ((r >= 0) && ((std::string::size_type)r < s.size())) { s.resize(r); break; }
  } va_end(vl); return s;
}
#if 1
#define Inject(T, N) (size_t)(&reinterpret_cast<char const volatile&>(((T*)0)->N))
#define EXP(O) O
#ifdef _MSC_VER
#define InjectTS(U, T) typeid(U::T).name()[11]==118?"vector":typeid(U::T).name()//(#U[3] == 0x3a?#U+5:#U)
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
 template<> Sql<o>* orm::Table<o>::QB[HARDWARE_ASYNCHRONOUS]={};\
 template<> uint8_t orm::Table<o>::_idex = 0;\
 template<> const size_t orm::Table<o>::_[NUM_ARGS(__VA_ARGS__)]={ OFFSET_N(o,NUM_ARGS(__VA_ARGS__),__VA_ARGS__) };\
 template<> bool orm::Table<o>::_create_need = true;\
 template<> uint8_t orm::Table<o>::_tc_[NUM_ARGS(__VA_ARGS__)]={};\
 template<> const char* orm::Table<o>::_def_[NUM_ARGS(__VA_ARGS__)]={}; \
 template<> const char* orm::Table<o>::_T_[NUM_ARGS(__VA_ARGS__)] = { TYPE_N(o,NUM_ARGS(__VA_ARGS__),__VA_ARGS__) };\
 template<> const char* orm::Table<o>::$[NUM_ARGS(__VA_ARGS__)] = { PROTO_N(NUM_ARGS(__VA_ARGS__),__VA_ARGS__) };\
//

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
static void to_json(json& j, const o& f) { COL_N(f,NUM_ARGS(__VA_ARGS__),__VA_ARGS__) }\
static void from_json(const json& j, o& f) { ATTR_N(f,NUM_ARGS(__VA_ARGS__),__VA_ARGS__) }

#define STAR_1(o,k)      std::make_tuple(&o::k, #k)
#define STAR_2(o,k,...)  std::make_tuple(&o::k, #k), EXP(STAR_1(o,__VA_ARGS__))
#define STAR_3(o,k,...)  std::make_tuple(&o::k, #k), EXP(STAR_2(o,__VA_ARGS__))
#define STAR_4(o,k,...)  std::make_tuple(&o::k, #k), EXP(STAR_3(o,__VA_ARGS__))
#define STAR_5(o,k,...)  std::make_tuple(&o::k, #k), EXP(STAR_4(o,__VA_ARGS__))
#define STAR_6(o,k,...)  std::make_tuple(&o::k, #k), EXP(STAR_5(o,__VA_ARGS__))
#define STAR_7(o,k,...)  std::make_tuple(&o::k, #k), EXP(STAR_6(o,__VA_ARGS__))
#define STAR_8(o,k,...)  std::make_tuple(&o::k, #k), EXP(STAR_7(o,__VA_ARGS__))
#define STAR_9(o,k,...)  std::make_tuple(&o::k, #k), EXP(STAR_8(o,__VA_ARGS__))
#define STAR_10(o,k,...) std::make_tuple(&o::k, #k), EXP(STAR_9(o,__VA_ARGS__))
#define STAR_11(o,k,...) std::make_tuple(&o::k, #k), EXP(STAR_10(o,__VA_ARGS__))
#define STAR_12(o,k,...) std::make_tuple(&o::k, #k), EXP(STAR_11(o,__VA_ARGS__))
#define STAR_13(o,k,...) std::make_tuple(&o::k, #k), EXP(STAR_12(o,__VA_ARGS__))
#define STAR_14(o,k,...) std::make_tuple(&o::k, #k), EXP(STAR_13(o,__VA_ARGS__))
#define STAR_15(o,k,...) std::make_tuple(&o::k, #k), EXP(STAR_14(o,__VA_ARGS__))
#define STAR_16(o,k,...) std::make_tuple(&o::k, #k), EXP(STAR_15(o,__VA_ARGS__))
#define STAR_17(o,k,...) std::make_tuple(&o::k, #k), EXP(STAR_16(o,__VA_ARGS__))
#define STAR_18(o,k,...) std::make_tuple(&o::k, #k), EXP(STAR_17(o,__VA_ARGS__))
#define STAR_19(o,k,...) std::make_tuple(&o::k, #k), EXP(STAR_18(o,__VA_ARGS__))
#define STAR_20(o,k,...) std::make_tuple(&o::k, #k), EXP(STAR_19(o,__VA_ARGS__))
#define STAR_21(o,k,...) std::make_tuple(&o::k, #k), EXP(STAR_20(o,__VA_ARGS__))
#define STAR_22(o,k,...) std::make_tuple(&o::k, #k), EXP(STAR_21(o,__VA_ARGS__))
#define STAR_23(o,k,...) std::make_tuple(&o::k, #k), EXP(STAR_22(o,__VA_ARGS__))
#define STAR_24(o,k,...) std::make_tuple(&o::k, #k), EXP(STAR_23(o,__VA_ARGS__))
#define STAR_25(o,k,...) std::make_tuple(&o::k, #k), EXP(STAR_24(o,__VA_ARGS__))
#define STAR_26(o,k,...) std::make_tuple(&o::k, #k), EXP(STAR_25(o,__VA_ARGS__))
#define STAR_27(o,k,...) std::make_tuple(&o::k, #k), EXP(STAR_26(o,__VA_ARGS__))
#define STAR_28(o,k,...) std::make_tuple(&o::k, #k), EXP(STAR_27(o,__VA_ARGS__))
#define STAR_29(o,k,...) std::make_tuple(&o::k, #k), EXP(STAR_28(o,__VA_ARGS__))
#define STAR_30(o,k,...) std::make_tuple(&o::k, #k), EXP(STAR_29(o,__VA_ARGS__))
#define STAR_31(o,k,...) std::make_tuple(&o::k, #k), EXP(STAR_30(o,__VA_ARGS__))
#define STAR_32(o,k,...) std::make_tuple(&o::k, #k), EXP(STAR_31(o,__VA_ARGS__))
/** Single init **///not use
#define STARS_N(o,N,...) EXP(STAR_##N(o,__VA_ARGS__))
#define STARS(o,N,...) STARS_N(o,N,__VA_ARGS__)

#define REGISTER_TABLE(o)\
    template<> std::string orm::Table<o>::_create_ = "CREATE TABLE IF NOT EXISTS "#o" (\n";\
    template<> const char* orm::Table<o>::_drop_ = "DROP TABLE IF EXISTS "#o";";\
    template<> const char* orm::Table<o>::_name = #o;
#define CONSTRUCT(o,...)\
        ATTRS(o,__VA_ARGS__)\
        REGIST(o, __VA_ARGS__)\
        REGISTER_TABLE(o)\
    template <> inline constexpr auto orm::Schema<o>() {\
      return std::make_tuple(STARS(o,NUM_ARGS(__VA_ARGS__), __VA_ARGS__));\
    }

//----------------------------------------------------------
#define PTR_2(a,o,k,v)      _tc_[a] = k; _def_[a] = v;
#define PTR_4(a,o,k,v,...)  _tc_[a] = k; _def_[a] = v; EXP(PTR_2(a+1,o,__VA_ARGS__))
#define PTR_6(a,o,k,v,...)  _tc_[a] = k; _def_[a] = v; EXP(PTR_4(a+1,o,__VA_ARGS__))
#define PTR_8(a,o,k,v,...)  _tc_[a] = k; _def_[a] = v; EXP(PTR_6(a+1,o,__VA_ARGS__))
#define PTR_10(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_8(a+1,o,__VA_ARGS__))
#define PTR_12(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_10(a+1,o,__VA_ARGS__))
#define PTR_14(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_12(a+1,o,__VA_ARGS__))
#define PTR_16(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_14(a+1,o,__VA_ARGS__))
#define PTR_18(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_16(a+1,o,__VA_ARGS__))
#define PTR_20(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_18(a+1,o,__VA_ARGS__))
#define PTR_22(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_20(a+1,o,__VA_ARGS__))
#define PTR_24(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_22(a+1,o,__VA_ARGS__))
#define PTR_26(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_24(a+1,o,__VA_ARGS__))
#define PTR_28(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_26(a+1,o,__VA_ARGS__))
#define PTR_30(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_28(a+1,o,__VA_ARGS__))
#define PTR_32(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_30(a+1,o,__VA_ARGS__))
#define PTR_34(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_32(a+1,o,__VA_ARGS__))
#define PTR_36(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_34(a+1,o,__VA_ARGS__))
#define PTR_38(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_36(a+1,o,__VA_ARGS__))
#define PTR_40(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_38(a+1,o,__VA_ARGS__))
#define PTR_42(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_40(a+1,o,__VA_ARGS__))
#define PTR_44(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_42(a+1,o,__VA_ARGS__))
#define PTR_46(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_44(a+1,o,__VA_ARGS__))
#define PTR_48(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_46(a+1,o,__VA_ARGS__))
#define PTR_50(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_48(a+1,o,__VA_ARGS__))
#define PTR_52(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_50(a+1,o,__VA_ARGS__))
#define PTR_54(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_52(a+1,o,__VA_ARGS__))
#define PTR_56(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_54(a+1,o,__VA_ARGS__))
#define PTR_58(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_56(a+1,o,__VA_ARGS__))
#define PTR_60(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_58(a+1,o,__VA_ARGS__))
#define PTR_62(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_60(a+1,o,__VA_ARGS__))
#define PTR_64(a,o,k,v,...) _tc_[a] = k; _def_[a] = v; EXP(PTR_62(a+1,o,__VA_ARGS__))
#define PTRS_N(o,N,...) EXP(PTR_##N(0,o,__VA_ARGS__))
#define PTRS(o,N,...) PTRS_N(o,N,__VA_ARGS__)
//regist PROPERTY
#define REGIST_PROTO(o,...)\
  o::o(bool b){ PTRS(o, NUM_ARGS(__VA_ARGS__), __VA_ARGS__)  }
        //
#endif
