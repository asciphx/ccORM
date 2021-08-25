#pragma once
#define SYS_IS_UTF8 1           //------ use GBK or UTF8 ------
#define MaxSyncConnections 32   //---- MaxSync Connections ----
#include <tuple>
#include <vector>
#include <any>
#include <string>
#include <cassert>
#include <cstring>
#include <deque>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <thread>
#include <unordered_map>

#include <boost/lexical_cast.hpp>
#include <string_view>

#include <mysql/mysql.h>
#include <libpq-fe.h>
#include <sqlite3.h>

#include <chrono>
#include <atomic>
#include <stdarg.h>
#include <stdexcept>
#include <functional>
#include <assert.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
#if __APPLE__
#include <machine/endian.h>
#endif
#if __APPLE__ 
#include <libkern/OSByteOrder.h>
#define be64toh(x) OSSwapBigToHostInt64(x)
#elif _WIN32
#define be64toh(x) _byteswap_uint64(x)
#endif
#define INT8OID 20
#define INT2OID 21
#define INT4OID 23
namespace crow {

  template <typename... E, typename F> constexpr void apply_each(F&& f, E&&... e) {
	(void)std::initializer_list<int>{((void)f(std::forward<E>(e)), 0)...};
  }

  template <typename... E, typename F, typename R>
  constexpr auto tuple_map_reduce_impl(F&& f, R&& reduce, E&&... e) {
	return reduce(f(std::forward<E>(e))...);
  }

  template <typename T, typename F> constexpr void tuple_map(T&& t, F&& f) {
	return std::apply([&](auto&&... e) { apply_each(f, std::forward<decltype(e)>(e)...); },
	  std::forward<T>(t));
  }

  template <typename T, typename F> constexpr auto tuple_reduce(T&& t, F&& f) {
	return std::apply(std::forward<F>(f), std::forward<T>(t));
  }

  template <typename T, typename F, typename R>
  decltype(auto) tuple_map_reduce(T&& m, F map, R reduce) {
	auto fun = [&](auto... e) { return tuple_map_reduce_impl(map, reduce, e...); };
	return std::apply(fun, m);
  }

  template <typename F> constexpr inline std::tuple<> tuple_filter_impl() { return std::make_tuple(); }

  template <typename F, typename... M, typename M1> constexpr auto tuple_filter_impl(M1 m1, M... m) {
	if constexpr (std::is_same<M1, F>::value)
	  return tuple_filter_impl<F>(m...);
	else
	  return std::tuple_cat(std::make_tuple(m1), tuple_filter_impl<F>(m...));
  }

  template <typename F, typename... M> constexpr auto tuple_filter(const std::tuple<M...>& m) {

	auto fun = [](auto... e) { return tuple_filter_impl<F>(e...); };
	return std::apply(fun, m);
  }

  template <typename... T> struct typelist {};

  template <typename... T1, typename... T2>
  constexpr auto typelist_cat(typelist<T1...> t1, typelist<T2...> t2) {
	return typelist<T1..., T2...>();
  }

  template <typename T> struct typelist_to_tuple {};

  template <typename... T> struct typelist_to_tuple<typelist<T...>> {
	typedef std::tuple<T...> type;
  };

  template <typename T> struct tuple_to_typelist {};

  template <typename... T> struct tuple_to_typelist<std::tuple<T...>> {
	typedef typelist<T...> type;
  };

  template <typename T> using typelist_to_tuple_t = typename typelist_to_tuple<T>::type;
  template <typename T> using tuple_to_typelist_t = typename tuple_to_typelist<T>::type;

  template <typename T, typename U> struct typelist_embeds : public std::false_type {};

  template <typename... T, typename U>
  struct typelist_embeds<typelist<T...>, U>
	: public std::integral_constant<bool, count_first_falses(std::is_same<T, U>::value...) !=
	sizeof...(T)> {};

  template <typename T, typename E> struct typelist_embeds_any_ref_of : public std::false_type {};

  template <typename U, typename... T>
  struct typelist_embeds_any_ref_of<typelist<T...>, U>
	: public typelist_embeds<typelist<std::decay_t<T>...>, std::decay_t<U>> {};
#ifdef _WIN32
  inline char* UnicodeToUtf8(const char* str) {
	LPCSTR pszSrc = str;
	int nLen = MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, NULL, 0);
	if (nLen == 0) return "";
	wchar_t* pwszDst = new wchar_t[nLen];
	MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, pwszDst, nLen);
	std::wstring wstr(pwszDst); delete[] pwszDst; pwszDst = NULL;
	const wchar_t* unicode = wstr.c_str();
	nLen = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, NULL, 0, NULL, NULL);
	char* szUtf8 = (char*)malloc(nLen + 1);
	memset(szUtf8, 0, nLen + 1);
	WideCharToMultiByte(CP_UTF8, 0, unicode, -1, szUtf8, nLen, NULL, NULL);
	return szUtf8;
  }
#else
  inline char* UnicodeToUtf8(const char* str) {
	if (NULL == str) return NULL;
	size_t destlen = mbstowcs(0, str, 0);
	size_t size = destlen + 1;
	wchar_t* pw = new wchar_t[size];
	mbstowcs(pw, str, size);
	size = wcslen(pw) * sizeof(wchar_t);
	char* pc = (char*)malloc(size + 1); memset(pc, 0, size + 1);
	destlen = wcstombs(pc, pw, size + 1);
	pc[size] = 0; delete[] pw; pw = NULL; return pc;
  }
#endif
  constexpr int count_first_falses() { return 0; }

  template <typename... B> constexpr int count_first_falses(bool b1, B... b) {
	if (b1)
	  return 0;
	else
	  return 1 + count_first_falses(b...);
  }

  template <typename E, typename... T> decltype(auto) arg_get_by_type_(void*, E* a1, T&&... args) {
	return std::forward<E*>(a1);
  }

  template <typename E, typename... T>
  decltype(auto) arg_get_by_type_(void*, const E* a1, T&&... args) {
	return std::forward<const E*>(a1);
  }

  template <typename E, typename... T> decltype(auto) arg_get_by_type_(void*, E& a1, T&&... args) {
	return std::forward<E&>(a1);
  }

  template <typename E, typename... T>
  decltype(auto) arg_get_by_type_(void*, const E& a1, T&&... args) {
	return std::forward<const E&>(a1);
  }

  template <typename E, typename T1, typename... T>
  decltype(auto) arg_get_by_type_(std::enable_if_t<!std::is_same<E, std::decay_t<T1>>::value>*,
	T1&&, T&&... args) {
	return arg_get_by_type_<E>((void*)0, std::forward<T>(args)...);
  }

  template <typename E, typename... T> decltype(auto) arg_get_by_type(T&&... args) {
	return arg_get_by_type_<std::decay_t<E>>(0, args...);
  }

  template <typename E, typename... T> decltype(auto) tuple_get_by_type(std::tuple<T...>& tuple) {
	typedef std::decay_t<E> DE;
	return std::get<count_first_falses((std::is_same<std::decay_t<T>, DE>::value)...)>(tuple);
  }

  template <typename E, typename... T> decltype(auto) tuple_get_by_type(std::tuple<T...>&& tuple) {
	typedef std::decay_t<E> DE;
	return std::get<count_first_falses((std::is_same<std::decay_t<T>, DE>::value)...)>(tuple);
  }

  template <typename T, typename U> struct tuple_embeds : public std::false_type {};

  template <typename... T, typename U>
  struct tuple_embeds<std::tuple<T...>, U>
	: public std::integral_constant<bool, count_first_falses(std::is_same<T, U>::value...) !=
	sizeof...(T)> {};

  template <typename U, typename... T> struct tuple_embeds_any_ref_of : public std::false_type {};
  template <typename U, typename... T>
  struct tuple_embeds_any_ref_of<std::tuple<T...>, U>
	: public tuple_embeds<std::tuple<std::decay_t<T>...>, std::decay_t<U>> {};

  template <typename T> struct tuple_remove_references;
  template <typename... T> struct tuple_remove_references<std::tuple<T...>> {
	typedef std::tuple<std::remove_reference_t<T>...> type;
  };

  template <typename T> using tuple_remove_references_t = typename tuple_remove_references<T>::type;

  template <typename T> struct tuple_remove_references_and_const;
  template <typename... T> struct tuple_remove_references_and_const<std::tuple<T...>> {
	typedef std::tuple<std::remove_const_t<std::remove_reference_t<T>>...> type;
  };

  template <typename T>
  using tuple_remove_references_and_const_t = typename tuple_remove_references_and_const<T>::type;

  template <typename T, typename U, typename E> struct tuple_remove_element2;

  template <typename... T, typename... U, typename E1>
  struct tuple_remove_element2<std::tuple<E1, T...>, std::tuple<U...>, E1>
	: public tuple_remove_element2<std::tuple<T...>, std::tuple<U...>, E1> {};

  template <typename... T, typename... U, typename T1, typename E1>
  struct tuple_remove_element2<std::tuple<T1, T...>, std::tuple<U...>, E1>
	: public tuple_remove_element2<std::tuple<T...>, std::tuple<U..., T1>, E1> {};

  template <typename... U, typename E1>
  struct tuple_remove_element2<std::tuple<>, std::tuple<U...>, E1> {
	typedef std::tuple<U...> type;
  };

  template <typename T, typename E>
  struct tuple_remove_element : public tuple_remove_element2<T, std::tuple<>, E> {};

  template <typename T, typename... E> struct tuple_remove_elements;

  template <typename... T, typename E1, typename... E>
  struct tuple_remove_elements<std::tuple<T...>, E1, E...> {
	typedef typename tuple_remove_elements<typename tuple_remove_element<std::tuple<T...>, E1>::type,
	  E...>::type type;
  };

  template <typename... T> struct tuple_remove_elements<std::tuple<T...>> {
	typedef std::tuple<T...> type;
  };

  template <typename A, typename B> struct tuple_minus;

  template <typename... T, typename... R> struct tuple_minus<std::tuple<T...>, std::tuple<R...>> {
	typedef typename tuple_remove_elements<std::tuple<T...>, R...>::type type;
  };

  template <typename T, typename... E>
  using tuple_remove_elements_t = typename tuple_remove_elements<T, E...>::type;

  template <typename F, size_t... I, typename... T>
  inline F tuple_map(std::tuple<T...>& t, F f, std::index_sequence<I...>) {
	return (void)std::initializer_list<int>{((void)f(std::get<I>(t)), 0)...}, f;
  }

  template <typename F, typename... T> inline void tuple_map(std::tuple<T...>& t, F f) {
	tuple_map(t, f, std::index_sequence_for<T...>{});
  }

  template <typename F, size_t... I, typename T>
  inline decltype(auto) tuple_transform(T&& t, F f, std::index_sequence<I...>) {
	return std::make_tuple(f(std::get<I>(std::forward<T>(t)))...);
  }

  template <typename F, typename T> inline decltype(auto) tuple_transform(T&& t, F f) {
	return tuple_transform(std::forward<T>(t), f,
	  std::make_index_sequence < std::tuple_size<std::decay_t<T>>{} > {});
  }

  template <template <class> class F, typename T, typename I, typename R, typename X = void>
  struct tuple_filter_sequence;

  template <template <class> class F, typename... T, typename R>
  struct tuple_filter_sequence<F, std::tuple<T...>, std::index_sequence<>, R> {
	using ret = R;
  };

  template <template <class> class F, typename T1, typename... T, size_t I1, size_t... I, size_t... R>
  struct tuple_filter_sequence<F, std::tuple<T1, T...>, std::index_sequence<I1, I...>,
	std::index_sequence<R...>, std::enable_if_t<F<T1>::value>> {
	using ret = typename tuple_filter_sequence<F, std::tuple<T...>, std::index_sequence<I...>,
	  std::index_sequence<R..., I1>>::ret;
  };

  template <template <class> class F, typename T1, typename... T, size_t I1, size_t... I, size_t... R>
  struct tuple_filter_sequence<F, std::tuple<T1, T...>, std::index_sequence<I1, I...>,
	std::index_sequence<R...>, std::enable_if_t<!F<T1>::value>> {
	using ret = typename tuple_filter_sequence<F, std::tuple<T...>, std::index_sequence<I...>,
	  std::index_sequence<R...>>::ret;
  };

  template <std::size_t... I, typename T>
  decltype(auto) tuple_filter_impl(std::index_sequence<I...>, T&& t) {
	return std::make_tuple(std::get<I>(t)...);
  }

  template <template <class> class F, typename T> decltype(auto) tuple_filter(T&& t) {
	using seq = typename tuple_filter_sequence<
	  F, std::decay_t<T>, std::make_index_sequence<std::tuple_size<std::decay_t<T>>::value>,
	  std::index_sequence<>>::ret;
	return tuple_filter_impl(seq{}, t);
  }
  namespace internal {
	template <typename T> struct has_parenthesis_operator {
	  template <typename C> static char test(decltype(&C::operator()));
	  template <typename C> static int test(...);
	  static const bool value = sizeof(test<T>(0)) == 1;
	};
  }

  template <typename F, typename X = void> struct callable_traits {
	typedef std::false_type is_callable;
	static const int arity = 0;
	typedef std::tuple<> arguments_tuple;
	typedef typelist<> arguments_list;
	typedef void return_type;
  };

  template <typename F, typename X> struct callable_traits<F&, X> : public callable_traits<F, X> {};
  template <typename F, typename X> struct callable_traits<F&&, X> : public callable_traits<F, X> {};
  template <typename F, typename X>
  struct callable_traits<const F&, X> : public callable_traits<F, X> {};

  template <typename F>
  struct callable_traits<F, std::enable_if_t<internal::has_parenthesis_operator<F>::value>> {
	typedef callable_traits<decltype(&F::operator())> super;
	typedef std::true_type is_callable;
	static const int arity = super::arity;
	typedef typename super::arguments_tuple arguments_tuple;
	typedef typename super::arguments_list arguments_list;
	typedef typename super::return_type return_type;
  };

  template <typename C, typename R, typename... ARGS>
  struct callable_traits<R(C::*)(ARGS...) const> {
	typedef std::true_type is_callable;
	static const int arity = sizeof...(ARGS);
	typedef std::tuple<ARGS...> arguments_tuple;
	typedef typelist<ARGS...> arguments_list;
	typedef R return_type;
  };

  template <typename C, typename R, typename... ARGS> struct callable_traits<R(C::*)(ARGS...)> {
	typedef std::true_type is_callable;
	static const int arity = sizeof...(ARGS);
	typedef std::tuple<ARGS...> arguments_tuple;
	typedef typelist<ARGS...> arguments_list;
	typedef R return_type;
  };

  template <typename R, typename... ARGS> struct callable_traits<R(ARGS...)> {
	typedef std::true_type is_callable;
	static const int arity = sizeof...(ARGS);
	typedef std::tuple<ARGS...> arguments_tuple;
	typedef typelist<ARGS...> arguments_list;
	typedef R return_type;
  };

  template <typename R, typename... ARGS> struct callable_traits<R(*)(ARGS...)> {
	typedef std::true_type is_callable;
	static const int arity = sizeof...(ARGS);
	typedef std::tuple<ARGS...> arguments_tuple;
	typedef typelist<ARGS...> arguments_list;
	typedef R return_type;
  };

  template <typename F>
  using callable_arguments_tuple_t = typename callable_traits<F>::arguments_tuple;
  template <typename F> using callable_arguments_list_t = typename callable_traits<F>::arguments_list;
  template <typename F> using callable_return_type_t = typename callable_traits<F>::return_type;

  template <typename F> struct is_callable : public callable_traits<F>::is_callable {};

  template <typename F, typename... A> struct callable_with {
	template <typename G, typename... B>
	static char test(int x,
	  std::remove_reference_t<decltype(std::declval<G>()(std::declval<B>()...))>* = 0);
	template <typename G, typename... B> static int test(...);
	static const bool value = sizeof(test<F, A...>(0)) == 1;
  };

  template <typename V>
  struct type_hashmap {

	template <typename E, typename F> E& get_cache_entry(int& hash, F) {
	  if (hash == -1) {
		std::lock_guard lock(mutex_);
		if (hash == -1)
		  hash = ++counter_;
	  }
	  if (hash >= values_.size() or !values_[hash].has_value()) {
		if (values_.size() < hash + 1)
		  values_.resize(hash + 1);
		values_[hash] = E();
	  }

	  return std::any_cast<E&>(values_[hash]);
	}
	template <typename K, typename F> V& operator()(F f, K key) {
	  static int hash = -1;
	  return this->template get_cache_entry<std::unordered_map<K, V>>(hash, f)[key];
	}

	template <typename F> V& operator()(F f) {
	  static int hash = -1;
	  return this->template get_cache_entry<V>(hash, f);
	}

  private:
	static std::mutex mutex_;
	static int counter_;
	std::vector<std::any> values_;
  };

  template <typename V>
  std::mutex type_hashmap<V>::mutex_;
  template <typename V>
  int type_hashmap<V>::counter_ = 0;

  template <typename T> struct is_tuple_after_decay : std::false_type {};
  template <typename... T> struct is_tuple_after_decay<std::tuple<T...>> : std::true_type {};

  template <typename T> struct is_tuple : is_tuple_after_decay<std::decay_t<T>> {};
  template <typename T> struct unconstref_tuple_elements {};
  template <typename... T> struct unconstref_tuple_elements<std::tuple<T...>> {
	typedef std::tuple<std::remove_const_t<std::remove_reference_t<T>>...> ret;
  };

  struct sql_blob : public std::string {
	using std::string::string;
	using std::string::operator=;

	sql_blob() : std::string() {}
  };

  struct sql_null_t {};

  template <unsigned SIZE> struct sql_varchar : public std::string {
	using std::string::string;
	using std::string::operator=;

	sql_varchar() : std::string() {}
  };

  template <typename I> struct sql_result {
	I impl_;
	sql_result() = delete;
	sql_result& operator=(sql_result&) = delete;
	sql_result(const sql_result&) = delete;
	sql_result(I&& impl) : impl_(std::forward<I>(impl)) {}

	inline ~sql_result() { this->flush_results(); }

	inline void flush_results() { impl_.flush_results(); }


	long long int last_insert_id() { return impl_.last_insert_id(); }


	template <typename T1, typename... T> auto r__();

	template <typename T1, typename... T> auto read_optional();


	template <typename T1, typename... T> bool r__(T1&& t1, T&... tail);
	template <typename T> void r__(std::optional<T>& o);


	template <typename F> void map(F f);
	nlohmann::json JSON();//or any other json lib
  };

  template <typename B> nlohmann::json sql_result<B>::JSON() {
	nlohmann::json t; uint32_t result = impl_.readJson(std::forward<nlohmann::json>(t));
	//if (!result) throw std::runtime_error("sql_result::r__: error: Trying to read a request that did not return any data.");
	return nlohmann::json{ {"num",result},{"result",t} };
  }
  template <typename B>
  template <typename T1, typename... T>
  bool sql_result<B>::r__(T1&& t1, T&... tail) {
	if constexpr (crow::is_tuple<std::decay_t<T1>>::value) {
	  static_assert(sizeof...(T) == 0);
	  return impl_.read(std::forward<T1>(t1));
	} else
	  return impl_.read(std::tie(t1, tail...));
  }

  template <typename B> template <typename T1, typename... T> auto sql_result<B>::r__() {
	auto t = [] {
	  if constexpr (sizeof...(T) == 0)
		return T1{};
	  else
		return std::tuple<T1, T...>{};
	}();
	if (!this->r__(t))
	  throw std::runtime_error("sql_result::r__: error: Trying to read a request that did not return any data.");
	return t;
  }

  template <typename B> template <typename T> void sql_result<B>::r__(std::optional<T>& o) {
	o = this->read_optional<T>();
  }

  template <typename B>
  template <typename T1, typename... T>
  auto sql_result<B>::read_optional() {
	auto t = [] {
	  if constexpr (sizeof...(T) == 0)
		return T1{};
	  else
		return std::tuple<T1, T...>{};
	}();
	if (this->r__(t))
	  return std::make_optional(std::move(t));
	else
	  return std::optional<decltype(t)>{};
  }

  namespace internal {

	template<typename T, typename F>
	constexpr auto is_valid(F&& f) -> decltype(f(std::declval<T>()), true) { return true; }

	template<typename>
	constexpr bool is_valid(...) { return false; }

  }

class sqlite_statement_result;
#define IS_VALID(T, EXPR) internal::is_valid<T>( [](auto&& obj)->decltype(obj.EXPR){} )

  template <typename B> template <typename F> void sql_result<B>::map(F map_function) {
	if constexpr (std::is_same_v<B, sqlite_statement_result>) {
	  //std::cout << "sqlite is not support!";
	} else {
	  if constexpr (IS_VALID(B, map(map_function)))
		this->impl_.map(map_function);
	  typedef typename unconstref_tuple_elements<callable_arguments_tuple_t<F>>::ret TP;
	  typedef std::tuple_element_t<0, TP> TP0;
	  auto t = [] {
		static_assert(std::tuple_size_v < TP >> 0, "sql_result map function must take at least 1 argument.");
		if constexpr (is_tuple<TP0>::value)
		  return TP0{};
		else
		  return TP{};
	  }();
	  while (this->r__(t)) {
		if constexpr (is_tuple<TP0>::value)
		  map_function(t);
		else
		  std::apply(map_function, t);
	  }
	}
  }
}
struct mysql_statement_data : std::enable_shared_from_this<mysql_statement_data> {
  MYSQL_STMT* stmt_ = nullptr;
  int num_fields_ = -1;
  MYSQL_RES* metadata_ = nullptr;
  MYSQL_FIELD* fields_ = nullptr;
  mysql_statement_data(MYSQL_STMT* stmt) {
	stmt_ = stmt;
	metadata_ = mysql_stmt_result_metadata(stmt_);
	if (metadata_) {
	  fields_ = mysql_fetch_fields(metadata_);
	  num_fields_ = mysql_num_fields(metadata_);
	}
  }
  ~mysql_statement_data() {
	if (metadata_)
	  mysql_free_result(metadata_);
	mysql_stmt_free_result(stmt_);
	if (mysql_stmt_close(stmt_))
	  std::cerr << "Error: could not free mysql statement" << std::endl;
  }
};
namespace crow {

  struct mysql_connection_data {
	~mysql_connection_data() { mysql_close(connection_); }
	MYSQL* connection_;
	std::unordered_map<std::string, std::shared_ptr<mysql_statement_data>> statements_;
	type_hashmap<std::shared_ptr<mysql_statement_data>> statements_hashmap_;
	int error_ = 0;
  };

  struct mysql_functions_blocking {
#define MYSQL_BLOCKING_WRAPPER(ERR, FN)                                                              \
  template <typename A1, typename... A> auto FN(int& connection_status, A1 a1, A&&... a) {\
    int ret = ::FN(a1, std::forward<A>(a)...); \
    if (ret && ret != MYSQL_NO_DATA && ret != MYSQL_DATA_TRUNCATED) \
    { \
      connection_status = 1;\
      throw std::runtime_error(std::string("Mysql error: ") + ERR(a1));\
    } \
    return ret; }

	MYSQL_ROW mysql_fetch_row(int& connection_status, MYSQL_RES* res) { return ::mysql_fetch_row(res); }
	int mysql_free_result(int& connection_status, MYSQL_RES* res) { ::mysql_free_result(res); return 0; }
	MYSQL_BLOCKING_WRAPPER(mysql_error, mysql_fetch_row)
	  MYSQL_BLOCKING_WRAPPER(mysql_error, mysql_real_query)
	  MYSQL_BLOCKING_WRAPPER(mysql_error, mysql_free_result)
	  MYSQL_BLOCKING_WRAPPER(mysql_stmt_error, mysql_stmt_execute)
	  MYSQL_BLOCKING_WRAPPER(mysql_stmt_error, mysql_stmt_reset)
	  MYSQL_BLOCKING_WRAPPER(mysql_stmt_error, mysql_stmt_prepare)
	  MYSQL_BLOCKING_WRAPPER(mysql_stmt_error, mysql_stmt_fetch)
	  MYSQL_BLOCKING_WRAPPER(mysql_stmt_error, mysql_stmt_free_result)
	  MYSQL_BLOCKING_WRAPPER(mysql_stmt_error, mysql_stmt_store_result)

#undef MYSQL_BLOCKING_WRAPPER

  };

#ifdef LIBMARIADB
  struct mysql_functions_non_blocking {

	template <typename RT, typename A1, typename... A, typename B1, typename... B>
	auto mysql_non_blocking_call(int& connection_status,
	  const char* fn_name,
	  const char* error_fun(B1),
	  int fn_start(RT*, B1, B...),
	  int fn_cont(RT*, B1, int), A1&& a1, A&&... args) {

	  RT ret;
	  int status = fn_start(&ret, std::forward<A1>(a1), std::forward<A>(args)...);

	  bool error = false;
	  while (status) {
		try {
		  std::this_thread::sleep_for(std::chrono::milliseconds(1));
		} catch (std::runtime_error& e) {
		  connection_status = 1;
		  throw std::move(e);
		}

		status = fn_cont(&ret, std::forward<A1>(a1), status);
	  }
	  if (ret && ret != MYSQL_NO_DATA && ret != MYSQL_DATA_TRUNCATED) {
		connection_status = 1;
		throw std::runtime_error(std::string("Mysql error in ") + fn_name + ": " + error_fun(a1));
	  }
	  return ret;
	}


#define MYSQL_NONBLOCKING_WRAPPER(ERR, FN)                                                           \
  template <typename... A> auto FN(int& connection_status, A&&... a) {                                                     \
    return mysql_non_blocking_call(connection_status, #FN, ERR, ::FN##_start, ::FN##_cont, std::forward<A>(a)...);              \
  }
	MYSQL_NONBLOCKING_WRAPPER(mysql_error, mysql_fetch_row)
	  MYSQL_NONBLOCKING_WRAPPER(mysql_error, mysql_real_query)
	  MYSQL_NONBLOCKING_WRAPPER(mysql_error, mysql_free_result)
	  MYSQL_NONBLOCKING_WRAPPER(mysql_stmt_error, mysql_stmt_execute)
	  MYSQL_NONBLOCKING_WRAPPER(mysql_stmt_error, mysql_stmt_reset)
	  MYSQL_NONBLOCKING_WRAPPER(mysql_stmt_error, mysql_stmt_prepare)
	  MYSQL_NONBLOCKING_WRAPPER(mysql_stmt_error, mysql_stmt_fetch)
	  MYSQL_NONBLOCKING_WRAPPER(mysql_stmt_error, mysql_stmt_free_result)
	  MYSQL_NONBLOCKING_WRAPPER(mysql_stmt_error, mysql_stmt_store_result)

#undef MYSQL_NONBLOCKING_WRAPPER
  };

#else
#error Only the MariaDB libmysqlclient is supported.
#endif

  inline auto type_to_mysql_statement_buffer_type(const char&) { return MYSQL_TYPE_TINY; }
  inline auto type_to_mysql_statement_buffer_type(const short int&) { return MYSQL_TYPE_SHORT; }
  inline auto type_to_mysql_statement_buffer_type(const int&) { return MYSQL_TYPE_LONG; }
  inline auto type_to_mysql_statement_buffer_type(const long long int&) { return MYSQL_TYPE_LONGLONG; }
  inline auto type_to_mysql_statement_buffer_type(const float&) { return MYSQL_TYPE_FLOAT; }
  inline auto type_to_mysql_statement_buffer_type(const double&) { return MYSQL_TYPE_DOUBLE; }
  inline auto type_to_mysql_statement_buffer_type(const sql_blob&) { return MYSQL_TYPE_BLOB; }
  inline auto type_to_mysql_statement_buffer_type(const char*) { return MYSQL_TYPE_STRING; }
  template <unsigned S> inline auto type_to_mysql_statement_buffer_type(const sql_varchar<S>) { return MYSQL_TYPE_STRING; }
  inline auto type_to_mysql_statement_buffer_type(const unsigned char&) { return MYSQL_TYPE_TINY; }
  inline auto type_to_mysql_statement_buffer_type(const unsigned short int&) { return MYSQL_TYPE_SHORT; }
  inline auto type_to_mysql_statement_buffer_type(const unsigned int&) { return MYSQL_TYPE_LONG; }
  inline auto type_to_mysql_statement_buffer_type(const unsigned long long int&) { return MYSQL_TYPE_LONGLONG; }
  template <typename T>
  typename std::enable_if_t<std::is_integral<T>::value, std::string> cpptype_to_mysql_type(const T&) { return "INT"; }
  template <typename T>
  typename std::enable_if_t<std::is_floating_point<T>::value, std::string> cpptype_to_mysql_type(const T&) { return "DOUBLE"; }
  inline std::string cpptype_to_mysql_type(const std::string&) { return "MEDIUMTEXT"; }
  inline std::string cpptype_to_mysql_type(const sql_blob&) { return "BLOB"; }
  template <unsigned S> inline std::string cpptype_to_mysql_type(const sql_varchar<S>) {
	std::ostringstream ss; ss << "VARCHAR(" << S << ')'; return ss.str();
  }
  template <unsigned N> struct mysql_bind_data {
	mysql_bind_data() {
	  memset(bind.data(), 0, N * sizeof(MYSQL_BIND)); for (int i = 0; i < N; ++i) bind[i].error = &errors[i];
	}
	std::array<unsigned long, N> real_lengths; std::array<MYSQL_BIND, N> bind; std::array<char, N> errors;
  };
  template <typename V> void mysql_bind_param(MYSQL_BIND& b, V& v) {
	b.buffer = const_cast<std::remove_const_t<V>*>(&v);
	b.buffer_type = type_to_mysql_statement_buffer_type(v);
	b.is_unsigned = std::is_unsigned<V>::value;
  }
  inline void mysql_bind_param(MYSQL_BIND& b, std::string& s) {
	b.buffer = &s[0]; b.buffer_type = MYSQL_TYPE_STRING;
	b.buffer_length = s.size();
  }
  inline void mysql_bind_param(MYSQL_BIND& b, const std::string& s) {
	mysql_bind_param(b, *const_cast<std::string*>(&s));
  }

  template <unsigned SIZE> void mysql_bind_param(MYSQL_BIND& b, const sql_varchar<SIZE>& s) {
	mysql_bind_param(b, *const_cast<std::string*>(static_cast<const std::string*>(&s)));
  }

  inline void mysql_bind_param(MYSQL_BIND& b, char* s) {
	b.buffer = s; b.buffer_type = MYSQL_TYPE_STRING; b.buffer_length = strlen(s);
  }
  inline void mysql_bind_param(MYSQL_BIND& b, const char* s) { mysql_bind_param(b, const_cast<char*>(s)); }
  inline void mysql_bind_param(MYSQL_BIND& b, sql_blob& s) {
	b.buffer = &s[0]; b.buffer_type = MYSQL_TYPE_BLOB; b.buffer_length = s.size();
  }
  inline void mysql_bind_param(MYSQL_BIND& b, const sql_blob& s) {
	mysql_bind_param(b, *const_cast<sql_blob*>(&s));
  }
  inline void mysql_bind_param(MYSQL_BIND& b, sql_null_t n) { b.buffer_type = MYSQL_TYPE_NULL; }
  template <typename T> void mysql_bind_output(MYSQL_BIND& b, unsigned long* real_length, T& v) {
	mysql_bind_param(b, v);
  }
  inline void mysql_bind_output(MYSQL_BIND& b, unsigned long* real_length, std::string& v) {
	v.resize(100); b.buffer_type = MYSQL_TYPE_STRING; b.buffer_length = v.size();
	b.buffer = (void*)v.data(); b.length = real_length;
  }
  template <unsigned SIZE>
  void mysql_bind_output(MYSQL_BIND& b, unsigned long* real_length, sql_varchar<SIZE>& s) {
	s.resize(SIZE); b.buffer = &s[0]; b.buffer_type = MYSQL_TYPE_STRING;
	b.buffer_length = s.size(); b.length = real_length;
  }
  template <typename A> mysql_bind_data<1> mysql_bind_output(mysql_statement_data& data, A& o) {
	if (data.num_fields_ != 1)
	  throw std::runtime_error("mysql_statement error: The number of column in the result set "
		"shoud be 1. Use std::tuple or crow::sio to fetch several columns or "
		"modify the request so that it returns a set of 1 column.");
	mysql_bind_data<1> bind_data; mysql_bind_output(bind_data.bind[0], &bind_data.real_lengths[0], o);
	return bind_data;
  }
  template <typename... A>
  mysql_bind_data<sizeof...(A)> mysql_bind_output(mysql_statement_data& data, std::tuple<A...>& o) {
	if (data.num_fields_ != sizeof...(A))
	  throw std::runtime_error("mysql_statement error: The number of column in the result set does "
		"not match the number of attributes of the tuple to bind.");
	mysql_bind_data<sizeof...(A)> bind_data;
	MYSQL_BIND* bind = bind_data.bind.data();
	unsigned long* real_lengths = bind_data.real_lengths.data();
	int i = 0; tuple_map(o, [&](auto& m) {
	  mysql_bind_output(bind[i], real_lengths + i, m);
	  ++i;
	  });
	return bind_data;
  }
  template <typename... A>
  mysql_bind_data<sizeof...(A)> mysql_bind_output(mysql_statement_data& data, std::tuple<A...>&& o) {
	if (data.num_fields_ != sizeof...(A))
	  throw std::runtime_error("mysql_statement error: The number of column in the result set does "
		"not match the number of attributes of the tuple to bind.");
	mysql_bind_data<sizeof...(A)> bind_data;
	MYSQL_BIND* bind = bind_data.bind.data();
	unsigned long* real_lengths = bind_data.real_lengths.data();
	int i = 0;
	tuple_map(std::forward<std::tuple<A...>>(o), [&](auto& m) {
	  mysql_bind_output(bind[i], real_lengths + i, m);
	  ++i;
	  });
	return bind_data;
  }

  template <typename B> struct mysql_statement_result {
	mysql_statement_result(B& mysql_wrapper_, mysql_statement_data& data_,
	  const std::shared_ptr<mysql_connection_data>& connection_)
	  : mysql_wrapper_(mysql_wrapper_), data_(data_), connection_(connection_) {}
	mysql_statement_result& operator=(mysql_statement_result&) = delete;
	mysql_statement_result(const mysql_statement_result&) = delete;
	mysql_statement_result(mysql_statement_result&&) = default;


	inline ~mysql_statement_result() { flush_results(); }

	inline void flush_results() {
	  if (connection_)         mysql_wrapper_.mysql_stmt_free_result(connection_->error_, data_.stmt_);
	}
	template <typename T> bool read(T&& output);

	template <typename T> bool read(T&& output, MYSQL_BIND* bind, unsigned long* real_lengths);

	template <typename F> void map(F map_callback);

	long long int affected_rows();

	long long int last_insert_id();
	void next_row();
	template <typename... A>
	void finalize_fetch(MYSQL_BIND* bind, unsigned long* real_lengths, std::tuple<A...>& o);
	template <typename T> void fetch_column(MYSQL_BIND*, unsigned long, T&, int);
	void fetch_column(MYSQL_BIND* b, unsigned long real_length, std::string& v, int i);
	template <unsigned SIZE>
	void fetch_column(MYSQL_BIND& b, unsigned long real_length, sql_varchar<SIZE>& v, int i);
	template <typename T> int fetch(T&& o);
	B& mysql_wrapper_;
	mysql_statement_data& data_;
	std::shared_ptr<mysql_connection_data> connection_;
	bool result_allocated_ = false;
  };
} namespace crow {
  template <typename B> long long int mysql_statement_result<B>::affected_rows() {
	return mysql_stmt_affected_rows(data_.stmt_);
  }
  template <typename B>
  template <typename T>
  void mysql_statement_result<B>::fetch_column(MYSQL_BIND* b, unsigned long, T&, int) {
	if (*b->error) {
	  throw std::runtime_error("Result could not fit in the provided types: loss of sign or "
		"significant digits or type mismatch.");
	}
  }
  template <typename B>
  void mysql_statement_result<B>::fetch_column(MYSQL_BIND* b, unsigned long real_length, std::string& v, int i) {
	if (real_length <= v.size()) {
	  v.resize(real_length);
	  return;
	}
	v.resize(real_length);
	b[i].buffer_length = v.size();
	b[i].buffer = v.data();
	mysql_stmt_bind_result(data_.stmt_, b);
	result_allocated_ = true;
	if (mysql_stmt_fetch_column(data_.stmt_, b + i, i, 0) != 0) {
	  connection_->error_ = 1;
	  throw std::runtime_error(std::string("mysql_stmt_fetch_column error: ") + mysql_stmt_error(data_.stmt_));
	}
  }
  template <typename B>
  template <unsigned SIZE>
  void mysql_statement_result<B>::fetch_column(MYSQL_BIND& b, unsigned long real_length, sql_varchar<SIZE>& v, int i) {
	v.resize(real_length);
  }
  template <typename B>
  template <typename F>
  void mysql_statement_result<B>::map(F map_callback) {
	typedef typename unconstref_tuple_elements<callable_arguments_tuple_t<F>>::ret TP;
	typedef std::tuple_element_t<0, TP> TP0;
	auto row_object = [] {
	  static_assert(std::tuple_size_v < TP >> 0, "sql_result map function must take at least 1 argument.");
	  if constexpr (is_tuple<TP0>::value)
		return TP0{};
	  else
		return TP{};
	}();
	result_allocated_ = true;
	auto bind_data = mysql_bind_output(data_, row_object);
	unsigned long* real_lengths = bind_data.real_lengths.data();
	MYSQL_BIND* bind = bind_data.bind.data();
	bool bind_ret = mysql_stmt_bind_result(data_.stmt_, bind_data.bind.data());
	if (bind_ret != 0) {
	  throw std::runtime_error(std::string("mysql_stmt_bind_result error: ") + mysql_stmt_error(data_.stmt_));
	}


	while (this->read(row_object, bind, real_lengths)) {
	  if constexpr (is_tuple<TP0>::value)
		map_callback(row_object);
	  else
		std::apply(map_callback, row_object);
	  if constexpr (is_tuple<std::decay_t<decltype(row_object)>>::value)
		tuple_map(row_object, [](auto& v) {
		if constexpr (std::is_same_v<std::decay_t<decltype(v)>, std::string>)
		  v.resize(100);
		  });
	}
  }
  template <typename B>
  template <typename T>
  bool mysql_statement_result<B>::read(T&& output) {
	result_allocated_ = true;
	auto bind_data = mysql_bind_output(data_, std::forward<T>(output));
	unsigned long* real_lengths = bind_data.real_lengths.data();
	MYSQL_BIND* bind = bind_data.bind.data();

	bool bind_ret = mysql_stmt_bind_result(data_.stmt_, bind_data.bind.data());
	if (bind_ret != 0) {
	  throw std::runtime_error(std::string("mysql_stmt_bind_result error: ") +
		mysql_stmt_error(data_.stmt_));
	}
	return this->read(std::forward<T>(output), bind, real_lengths);
  }
  template <typename B>
  template <typename T>
  bool mysql_statement_result<B>::read(T&& output, MYSQL_BIND* bind, unsigned long* real_lengths) {
	try {
	  int res = mysql_wrapper_.mysql_stmt_fetch(connection_->error_, data_.stmt_);
	  if (res == MYSQL_NO_DATA)         return false;
	  int i = 0;
	  tuple_map(std::forward<T>(output), [&](auto& m) {
		this->fetch_column(bind, real_lengths[i], m, i);
		++i;
		});
	  return true;
	} catch (const std::runtime_error& e) {
	  mysql_stmt_reset(data_.stmt_);
	  throw e;
	}
  }
  template <typename B> long long int mysql_statement_result<B>::last_insert_id() {
	return mysql_stmt_insert_id(data_.stmt_);
  }

  template <typename B> struct mysql_statement {

	template <typename... T> sql_result<mysql_statement_result<B>> operator()(T&&... args);

	B& mysql_wrapper_;
	mysql_statement_data& data_;
	std::shared_ptr<mysql_connection_data> connection_;
  };
  template <typename B>
  template <typename... T>
  sql_result<mysql_statement_result<B>> mysql_statement<B>::operator()(T&&... args) {
	if constexpr (sizeof...(T) > 0) {
	  MYSQL_BIND bind[sizeof...(T)];
	  memset(bind, 0, sizeof(bind));      int i = 0;
	  tuple_map(std::forward_as_tuple(args...), [&](auto& m) {
		mysql_bind_param(bind[i], m);
		++i;
		});
	  if (mysql_stmt_bind_param(data_.stmt_, bind) != 0) {
		connection_->error_ = 1;
		throw std::runtime_error(std::string("mysql_stmt_bind_param error: ") + mysql_stmt_error(data_.stmt_));
	  }
	}
	mysql_wrapper_.mysql_stmt_execute(connection_->error_, data_.stmt_);
	return sql_result<mysql_statement_result<B>>(mysql_statement_result<B>(mysql_wrapper_, data_, connection_));
  }

  template <typename B> struct mysql_result {
	B& mysql_wrapper_;
	std::shared_ptr<mysql_connection_data> connection_;
	MYSQL_RES* result_ = nullptr;
	unsigned long* current_row_lengths_ = nullptr;
	MYSQL_ROW current_row_ = nullptr;
	bool end_of_result_ = false;
	unsigned int current_row_num_fields_ = 0;
	char** proto_name_ = nullptr;
	uint16_t* proto_type_ = nullptr;
	uint64_t current_result_nrows_ = 1;
	mysql_result(B& mysql_wrapper_, std::shared_ptr<mysql_connection_data> connection_)
	  : mysql_wrapper_(mysql_wrapper_), connection_(connection_) {}
	mysql_result& operator=(mysql_result&) = delete;
	mysql_result(const mysql_result&) = delete;
	mysql_result(mysql_result&&) = default;
	~mysql_result() { free(proto_type_); free(proto_name_); proto_type_ = NULL; proto_name_ = NULL; flush(); }
	inline void flush() {
	  if (result_) {
		mysql_free_result(result_);
		result_ = nullptr;
	  }
	}
	inline void flush_results() { this->flush(); }
	inline void next_row();
	template <typename T> bool read(T&& output);
	template <typename T> unsigned int readJson(T&& output);
	long long int affected_rows();
	long long int last_insert_id();
  };

  template <typename B> inline void mysql_result<B>::next_row() {
	if (!result_)
	  result_ = mysql_use_result(connection_->connection_);
	current_row_ = mysql_wrapper_.mysql_fetch_row(connection_->error_, result_);
	current_row_num_fields_ = mysql_num_fields(result_);
	if (!current_row_ || current_row_num_fields_ == 0) {
	  end_of_result_ = true;
	  return;
	}
	current_row_lengths_ = mysql_fetch_lengths(result_);
  }
  template <typename B> template <typename T> unsigned int mysql_result<B>::readJson(T&& output) {
	next_row(); if (end_of_result_) return 0; T j;
	if (proto_type_ == nullptr) {
	  proto_name_ = (char**)malloc(sizeof(char*) * (int)current_row_num_fields_);
	  proto_type_ = (uint16_t*)malloc(sizeof(uint16_t) * (int)current_row_num_fields_);
	  MYSQL_FIELD* field;
	  for (unsigned int i = 0; i < current_row_num_fields_; ++i) {
		field = mysql_fetch_field(result_);
		proto_name_[i] = (char*)malloc(sizeof(char) * 51);
		strcpy(proto_name_[i], field->name);
		proto_type_[i] = (uint16_t)field->type;
	  }
	}_:
	for (unsigned int i = 0; i < current_row_num_fields_; ++i) {
	  if (current_row_[i] == 0) { j[proto_name_[i]] = nullptr; continue; }
		switch (proto_type_[i]) {
		case enum_field_types::MYSQL_TYPE_DOUBLE:
		  j[proto_name_[i]] = boost::lexical_cast<double>(
			std::string_view(current_row_[i], current_row_lengths_[i])); break;
		case enum_field_types::MYSQL_TYPE_FLOAT:
		  j[proto_name_[i]] = boost::lexical_cast<float>(
			std::string_view(current_row_[i], current_row_lengths_[i])); break;
		case enum_field_types::MYSQL_TYPE_TINY:
		  j[proto_name_[i]] = boost::lexical_cast<signed char>(
			std::string_view(current_row_[i], current_row_lengths_[i])); break;
		case enum_field_types::MYSQL_TYPE_INT24:
		  j[proto_name_[i]] = boost::lexical_cast<int>(
			std::string_view(current_row_[i], current_row_lengths_[i])); break;
		case enum_field_types::MYSQL_TYPE_SHORT:
		  j[proto_name_[i]] = boost::lexical_cast<short>(
			std::string_view(current_row_[i], current_row_lengths_[i])); break;
		case enum_field_types::MYSQL_TYPE_LONGLONG:
		  j[proto_name_[i]] = boost::lexical_cast<long long>(
			std::string_view(current_row_[i], current_row_lengths_[i])); break;
		case enum_field_types::MYSQL_TYPE_LONG:
		  j[proto_name_[i]] = boost::lexical_cast<long>(
			std::string_view(current_row_[i], current_row_lengths_[i])); break;
		case enum_field_types::MYSQL_TYPE_STRING:
		case enum_field_types::MYSQL_TYPE_VAR_STRING:
		case enum_field_types::MYSQL_TYPE_LONG_BLOB:
		case enum_field_types::MYSQL_TYPE_MEDIUM_BLOB:
		case enum_field_types::MYSQL_TYPE_TINY_BLOB:
		case enum_field_types::MYSQL_TYPE_BLOB: {
#ifdef SYS_IS_UTF8
		  j[proto_name_[i]] = boost::lexical_cast<std::string>(
			std::string_view(current_row_[i], current_row_lengths_[i])); break;
#else
		  char* c = UnicodeToUtf8(current_row_[i]);
		  j[proto_name_[i]] = boost::lexical_cast<std::string>(
			std::string_view(c, current_row_lengths_[i]));
		  free(c); c = NULL; break;
#endif // SYS_IS_UTF8
		} break;
		default: j[proto_name_[i]] = boost::lexical_cast<std::string>(
		  std::string_view(current_row_[i], current_row_lengths_[i])); break;
		}
	  //printf("[%.*s] ", (int)current_row_lengths_[i], current_row_[i] ? current_row_[i] : "NULL");
	}//printf("\n");
	if (!(current_row_ = mysql_wrapper_.mysql_fetch_row(connection_->error_, result_))) {
	  if (current_result_nrows_ == 1) output = j; else output.push_back(j);
	  return current_result_nrows_;
	} output.push_back(j); ++current_result_nrows_; goto _;
	}
  template <typename B> template <typename T> bool mysql_result<B>::read(T&& output) {
	next_row();
	if (end_of_result_) return false;
	unsigned int i = 0;
	if (std::tuple_size_v<std::decay_t<T>> != current_row_num_fields_)
	  throw std::runtime_error(std::string("The request number of field (") +
		boost::lexical_cast<std::string>(current_row_num_fields_) +
		") does not match the size of the tuple (" +
		boost::lexical_cast<std::string>(std::tuple_size_v<std::decay_t<T>>) + ")");
	crow::tuple_map(std::forward<T>(output), [&](auto& v) {
	  v = boost::lexical_cast<std::decay_t<decltype(v)>>(
		std::string_view(current_row_[i], current_row_lengths_[i]));
	  ++i;
	  });
	return true;
  }

  template <typename B> long long int mysql_result<B>::affected_rows() {
	return mysql_affected_rows(connection_->connection_);
  }

  template <typename B> long long int mysql_result<B>::last_insert_id() {
	return mysql_insert_id(connection_->connection_);
  }

  struct mysql_connection_data; struct mysql_tag {};  template <typename B>   struct mysql_connection {
	typedef mysql_tag db_tag;
	inline mysql_connection(B mysql_wrapper, std::shared_ptr<crow::mysql_connection_data>& data);

	long long int last_insert_rowid();

	sql_result<mysql_result<B>> operator()(const std::string& rq);

	mysql_statement<B> query(const std::string& rq);

	template <typename F, typename... K> mysql_statement<B> cached_statement(F f, K... keys);
	template <typename T> std::string type_to_string(const T& t) { return cpptype_to_mysql_type(t); }
	B mysql_wrapper_;
	std::shared_ptr<mysql_connection_data> data_;
  };
} namespace crow {
  template <typename B>
  inline mysql_connection<B>::mysql_connection(B mysql_wrapper, std::shared_ptr<crow::mysql_connection_data>& data)
	: mysql_wrapper_(mysql_wrapper), data_(data) {}
  template <typename B> long long int mysql_connection<B>::last_insert_rowid() {
	return mysql_insert_id(data_->connection_);
  }
  template <typename B>
  sql_result<mysql_result<B>> mysql_connection<B>::operator()(const std::string& rq) {
	mysql_wrapper_.mysql_real_query(data_->error_, data_->connection_, rq.c_str(), rq.size());
	return sql_result<mysql_result<B>>(
	  mysql_result<B>(mysql_wrapper_, data_));
  }
  template <typename B>
  template <typename F, typename... K>
  mysql_statement<B> mysql_connection<B>::cached_statement(F f, K... keys) {
	if (data_->statements_hashmap_(f).get() == nullptr) {
	  mysql_statement<B> res = query(f());
	  data_->statements_hashmap_(f, keys...) = res.data_.shared_from_this();
	  return res;
	} else
	  return mysql_statement<B>{mysql_wrapper_, * data_->statements_hashmap_(f, keys...),
	  data_};
  }
  template <typename B> mysql_statement<B> mysql_connection<B>::query(const std::string& rq) {
	auto it = data_->statements_.find(rq);
	if (it != data_->statements_.end()) {
	  return mysql_statement<B>{mysql_wrapper_, * it->second, data_};
	}
	MYSQL_STMT* stmt = mysql_stmt_init(data_->connection_);
	if (!stmt) {
	  data_->error_ = 1;
	  throw std::runtime_error(std::string("mysql_stmt_init error: ") + mysql_error(data_->connection_));
	}
	try {
	  if (mysql_wrapper_.mysql_stmt_prepare(data_->error_, stmt, rq.data(), rq.size())) {
		data_->error_ = 1;
		throw std::runtime_error(std::string("mysql_stmt_prepare error: ") + mysql_error(data_->connection_));
	  }
	} catch (...) {
	  mysql_stmt_close(stmt);
	  throw;
	}
	auto pair = data_->statements_.emplace(rq, std::make_shared<mysql_statement_data>(stmt));
	return mysql_statement<B>{mysql_wrapper_, * pair.first->second, data_};
  }
}
#define EXPECT_THROW(STM)\
try {STM;std::cerr << "This must have thrown an exception: " << #STM << std::endl; } catch (...) {}
#define EXPECT_EQUAL(A, B)\
if(A!=B){std::cerr << #A << " (== " << A << ") " << " != " << #B << " (== " << B << ") " << std::endl;}
#define EXPECT(A)\
  if (!(A)) { std::cerr << #A << " (== " << (A) << ") must be true" << std::endl; }
#define ASSERT(x,m) if(!x)std::cerr<<#m
template <typename... T> std::ostream& operator<<(std::ostream& os, std::tuple<T...> t) {
  bool one = true; os << "TUPLE<"; crow::tuple_map(std::forward<std::tuple<T...>>(t), [&os,&one](auto v) {
	if(one) os << v, one=false; else os << "," << v; }); os << ">"; return os;
}
namespace crow {
  struct mysql {
	typedef mysql_tag db_tag; typedef mysql_connection_data connection_data_type;
	inline mysql(const char* host, const char* database, const char* user, const char* password, ...);
	inline ~mysql();
	inline mysql_connection_data* new_connection();
	inline int get_socket(const std::shared_ptr<mysql_connection_data>& data);
	inline mysql_connection<mysql_functions_blocking> scoped_connection(std::shared_ptr<mysql_connection_data>& data);
	std::string host_, user_, passwd_, database_, character_set_;
	unsigned int port_;
	inline bool ping(mysql_connection_data* data) { return mysql_ping(data->connection_) == 0; }
  };
  inline mysql::mysql(const char* host, const char* database, const char* user, const char* password, ...) {
	ASSERT(host, "open_mysql_connection requires the host argument"); ASSERT(database, "open_mysql_connection requires the databaser argument"); ASSERT(user, "open_mysql_connection requires the user argument"); ASSERT(password, "open_mysql_connection requires the password argument");
	va_list ap; va_start(ap, password); unsigned int i = va_arg(ap, unsigned int); char* c = va_arg(ap, char*); port_ = i < 0xffff ? i : 0xcea; character_set_ = c[0] ? c : "utf8"; va_end(ap); host_ = host, database_ = database, user_ = user, passwd_ = password;
	if (mysql_library_init(0, NULL, NULL)) throw std::runtime_error("Could not initialize MySQL library."); if (!mysql_thread_safe()) throw std::runtime_error("Mysql is not compiled as thread safe.");
  }
  mysql::~mysql() { mysql_library_end(); }
  inline int mysql::get_socket(const std::shared_ptr<mysql_connection_data>& data) {
	return mysql_get_socket(data->connection_);
  }
  inline mysql_connection_data* mysql::new_connection() {
	MYSQL* mysql; MYSQL* connection;
	mysql = mysql_init(nullptr);

	mysql_options(mysql, MYSQL_OPT_NONBLOCK, 0);
	connection = nullptr;
	int mysql_fd = -1;
	int status;
	status = mysql_real_connect_start(&connection, mysql, host_.c_str(), user_.c_str(),
	  passwd_.c_str(), database_.c_str(), port_, NULL, 0);
	mysql_fd = mysql_get_socket(mysql);
	if (mysql_fd == -1) {
	  mysql_close(mysql);
	  return nullptr;
	}
	while (status) {
	  try {
		std::this_thread::sleep_for(std::chrono::microseconds(314));
		status = mysql_real_connect_cont(&connection, mysql, status);
	  } catch (std::runtime_error& e) {
		mysql_close(mysql);
		throw std::move(e);
	  }
	}
	if (!connection) { return nullptr; }
	char on = 1; mysql_options(mysql, MYSQL_REPORT_DATA_TRUNCATION, &on);
	mysql_set_character_set(mysql, character_set_.c_str());
	return new mysql_connection_data{ mysql };
  }
  inline mysql_connection<mysql_functions_blocking> mysql::scoped_connection(
	std::shared_ptr<mysql_connection_data>& data) {
	return mysql_connection<mysql_functions_blocking>(mysql_functions_blocking{}, data);
  }

  template <typename T>
  using tuple_remove_references_and_const_t = typename tuple_remove_references_and_const<T>::type;
  inline void free_sqlite3_statement(void* s) { sqlite3_finalize((sqlite3_stmt*)s); }
  struct sqlite_statement_result {
	sqlite3* db_; sqlite3_stmt* stmt_;
	int last_step_ret_; uint32_t rowcount_ = 0;
	inline void flush_results() { sqlite3_reset(stmt_); }
	// Read a tuple or a metamap.
	template <typename T> uint32_t readJson(T&& output) {
	  T j; int ncols = sqlite3_column_count(stmt_), i = 0; _:++rowcount_;
	  for (i = 0; i < ncols; ++i) {
		switch (sqlite3_column_type(stmt_, i)) {
		case SQLITE_INTEGER:j[sqlite3_column_name(stmt_, i)] = sqlite3_column_int64(stmt_, i);
		case SQLITE_FLOAT:j[sqlite3_column_name(stmt_, i)] = sqlite3_column_double(stmt_, i); break;
		case SQLITE_TEXT: {auto str = sqlite3_column_text(stmt_, i);
		  j[sqlite3_column_name(stmt_, i)] = (const char*)str;
		} break;
		case SQLITE_BLOB: {
		  j[sqlite3_column_name(stmt_, i)] = std::move(std::string((const char*)sqlite3_column_text(stmt_, i),
			sqlite3_column_bytes(stmt_, i)));
		} break;
		default:j[sqlite3_column_name(stmt_, i)] = nullptr; break;
		}
	  }
	  last_step_ret_ = sqlite3_step(stmt_);
	  if (last_step_ret_ != SQLITE_ROW) {
		if (rowcount_ == 1) output = j; else output.push_back(j);
		return rowcount_;
	  } output.push_back(j); goto _;
	  //if (last_step_ret_ != SQLITE_ROW && last_step_ret_ != SQLITE_DONE)
	  //  throw std::runtime_error(sqlite3_errstr(last_step_ret_));
	}

	template <typename T> bool read(T&& output) {
	  if (last_step_ret_ != SQLITE_ROW) return false;
	  //if constexpr (is_tuple<T>::value) {}
	  int ncols = sqlite3_column_count(stmt_);
	  int i = 0;
	  std::size_t tuple_size = std::tuple_size_v<std::decay_t<T>>;
	  if (ncols != tuple_size) {
		std::ostringstream ss;
		ss << "Invalid number of parameters: SQL request has " << ncols
		  << " fields but the function to process it has " << tuple_size << " parameters.";
		throw std::runtime_error(ss.str());
	  }
	  auto read_elt = [&](auto& v) {
		this->read_column(i, v, sqlite3_column_type(stmt_, i));
		++i;
	  };
	  ::crow::tuple_map(std::forward<T>(output), read_elt);
	  return true;
	}
	inline long long int last_insert_id() { return sqlite3_last_insert_rowid(db_); }
	inline void read_column(int pos, int& v, int sqltype) {
	  if (sqltype != SQLITE_INTEGER)
		throw std::runtime_error("destination type (integer).");
	  v = sqlite3_column_int(stmt_, pos);
	}
	inline void read_column(int pos, float& v, int sqltype) {
	  if (sqltype != SQLITE_FLOAT)
		throw std::runtime_error("destination type (float).");
	  v = float(sqlite3_column_double(stmt_, pos));
	}
	inline void read_column(int pos, double& v, int sqltype) {
	  if (sqltype != SQLITE_FLOAT)
		throw std::runtime_error("destination type (double).");
	  v = sqlite3_column_double(stmt_, pos);
	}
	inline void read_column(int pos, int64_t& v, int sqltype) {
	  if (sqltype != SQLITE_INTEGER)
		throw std::runtime_error("destination type (int64).");
	  v = sqlite3_column_int64(stmt_, pos);
	}
	inline void read_column(int pos, std::string& v, int sqltype) {
	  if (sqltype != SQLITE_TEXT && sqltype != SQLITE_BLOB)
		throw std::runtime_error("destination type (std::string).");
	  auto str = sqlite3_column_text(stmt_, pos);
	  auto n = sqlite3_column_bytes(stmt_, pos);
	  v = std::move(std::string((const char*)str, n));
	}
	// Todo: Date types
	// template <typename C, typename D>
	// void read_column(int pos, std::chrono::time_point<C, D>& v) {
	//   v = std::chrono::time_point<C, D>(sqlite3_column_int(stmt_, pos));
	// }
  };
  struct sqlite_statement {
	typedef std::shared_ptr<sqlite3_stmt> stmt_sptr;
	sqlite3* db_;
	sqlite3_stmt* stmt_;
	stmt_sptr stmt_sptr_;
	inline sqlite_statement() {}
	inline sqlite_statement(sqlite3* db, sqlite3_stmt* s)
	  : db_(db), stmt_(s), stmt_sptr_(stmt_sptr(s, free_sqlite3_statement)) {}
	template <typename... T> sql_result<sqlite_statement_result> operator()(T&&... args) {
	  sqlite3_reset(stmt_);
	  sqlite3_clear_bindings(stmt_);
	  int i = 1;
	  crow::tuple_map(std::forward_as_tuple(args...), [&](auto& m) {
		int err;
		if ((err = this->bind(stmt_, i, m)) != SQLITE_OK)
		  throw std::runtime_error(std::string("Sqlite error during binding: ") + sqlite3_errmsg(db_));
		++i;
		});
	  int last_step_ret = sqlite3_step(stmt_);
	  if (last_step_ret != SQLITE_ROW && last_step_ret != SQLITE_DONE) throw std::runtime_error(sqlite3_errstr(last_step_ret));
	  return sql_result<sqlite_statement_result>(sqlite_statement_result{ this->db_, this->stmt_, last_step_ret });
	}
	inline int bind(sqlite3_stmt* stmt, int pos, double d) const {
	  return sqlite3_bind_double(stmt, pos, d);
	}
	inline int bind(sqlite3_stmt* stmt, int pos, int d) const { return sqlite3_bind_int(stmt, pos, d); }
	inline int bind(sqlite3_stmt* stmt, int pos, long int d) const {
	  return sqlite3_bind_int64(stmt, pos, d);
	}
	inline int bind(sqlite3_stmt* stmt, int pos, long long int d) const {
	  return sqlite3_bind_int64(stmt, pos, d);
	}
	inline void bind(sqlite3_stmt* stmt, int pos, sql_null_t) { sqlite3_bind_null(stmt, pos); }
	inline int bind(sqlite3_stmt* stmt, int pos, const char* s) const {
#ifdef SYS_IS_UTF8
	  return sqlite3_bind_text(stmt, pos, s, strlen(s), nullptr);
#else
	  char* c = UnicodeToUtf8(s); int ret = sqlite3_bind_text(stmt, pos, c, strlen(c), nullptr);
	  free(c); c = NULL; return ret;
#endif
	}
	inline int bind(sqlite3_stmt* stmt, int pos, const std::string& s) const {
#ifdef SYS_IS_UTF8
	  return sqlite3_bind_text(stmt, pos, s.c_str(), s.length(), nullptr);
#else
	  char* c = UnicodeToUtf8(s.c_str()); int ret = sqlite3_bind_text(stmt, pos, c, strlen(c), nullptr);
	  free(c); c = NULL; return ret;
#endif
	}
	inline int bind(sqlite3_stmt* stmt, int pos, const std::string_view& s) const {
#ifndef SYS_IS_UTF8
	  return sqlite3_bind_text(stmt, pos, s.data(), s.length(), nullptr);
#else
	  char* c = UnicodeToUtf8(s.data()); int ret = sqlite3_bind_text(stmt, pos, c, strlen(c), nullptr);
	  free(c); c = NULL; return ret;
#endif
	}
	inline int bind(sqlite3_stmt* stmt, int pos, const sql_blob& b) const {
	  return sqlite3_bind_blob(stmt, pos, b.data(), b.size(), nullptr);
	}
  };
  inline void free_sqlite3_db(void* db) { sqlite3_close_v2((sqlite3*)db); }
  struct sqlite_connection {
	typedef std::shared_ptr<sqlite3> db_sptr;
	typedef std::unordered_map<std::string, sqlite_statement> stmt_map;
	typedef std::shared_ptr<std::unordered_map<std::string, sqlite_statement>> stmt_map_ptr;
	typedef std::shared_ptr<std::mutex> mutex_ptr;
	mutex_ptr cache_mutex_;
	sqlite3* db_;
	db_sptr db_sptr_;
	stmt_map_ptr stm_cache_;
	type_hashmap<sqlite_statement> statements_hashmap;
	inline sqlite_connection()
	  : db_(nullptr), stm_cache_(new stmt_map()), cache_mutex_(new std::mutex()) {}
	inline void conn(const std::string& filename,
	  int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE) {
	  int r = sqlite3_open_v2(filename.c_str(), &db_, flags, nullptr);
	  if (r != SQLITE_OK)
		throw std::runtime_error(std::string("Cannot open database ") + filename + " " + sqlite3_errstr(r));
	  db_sptr_ = db_sptr(db_, free_sqlite3_db);
	}
	template <typename E> inline void format_error(E&) const {}
	template <typename E, typename T1, typename... T>
	inline void format_error(E& err, T1 a, T... args) const {
	  err << a;
	  format_error(err, args...);
	}
	template <typename F> sqlite_statement cached_statement(F f) {
	  if (statements_hashmap(f).stmt_sptr_.get() == nullptr)
		return query(f());
	  else
		return statements_hashmap(f);
	}
	inline sqlite_statement query(const std::string& req) {
	  auto it = stm_cache_->find(req);
	  if (it != stm_cache_->end()) return it->second;
	  sqlite3_stmt* stmt;
	  int err = sqlite3_prepare_v2(db_, req.c_str(), req.size(), &stmt, nullptr);
	  if (err != SQLITE_OK)
		throw std::runtime_error(std::string("Sqlite error during query: ") + sqlite3_errmsg(db_) + " statement was: " + req);
	  cache_mutex_->lock();
	  auto it2 = stm_cache_->insert(it, std::make_pair(req, sqlite_statement(db_, stmt)));
	  cache_mutex_->unlock();
	  return it2->second;
	}
	inline sql_result<sqlite_statement_result> operator()(const std::string& req) { return query(req)(); }
	template <typename T>
	inline std::string type_to_string(const T&, std::enable_if_t<std::is_integral<T>::value>* = 0) {
	  return "INTEGER";
	}
	template <typename T>
	inline std::string type_to_string(const T&, std::enable_if_t<std::is_floating_point<T>::value>* = 0) {
	  return "REAL";
	}
	inline std::string type_to_string(const std::string&) { return "TEXT"; }
	inline std::string type_to_string(const sql_blob&) { return "BLOB"; }
	template <unsigned SIZE> inline std::string type_to_string(const sql_varchar<SIZE>&) {
	  std::ostringstream ss;
	  ss << "VARCHAR(" << SIZE << ')';
	  return ss.str();
	}
  };
  struct sqlite_tag {};
  struct Sqlite {
	typedef sqlite_tag db_tag;
	inline Sqlite() {}
	Sqlite(const std::string& path, unsigned int synchronous = 1) {
	  path_ = path;
	  con_.conn(path, SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
	  std::ostringstream ss;
	  ss << "PRAGMA synchronous=" << synchronous;
	  con_(ss.str());
	}
	template <typename Y> inline sqlite_connection conn(Y& y) { return con_; }
	inline sqlite_connection conn() { return con_; }
	inline void flush() { sqlite3_db_cacheflush(con_.db_); }
	inline void close() { sqlite3_close_v2(con_.db_); }
	sqlite_connection con_;
	std::string path_;
  };

  struct pgsql_statement_data;
  struct pgsql_connection_data {

	~pgsql_connection_data() {
	  if (pgconn_) {
		cancel();
		PQfinish(pgconn_);
	  }
	}
	void cancel() {
	  if (pgconn_) {
		PGcancel* cancel = PQgetCancel(pgconn_);
		char x[256];
		if (cancel) {
		  PQcancel(cancel, x, 256);
		  PQfreeCancel(cancel);
		}
	  }
	}
	void flush() {
	  while (int ret = PQflush(pgconn_)) {
		if (ret == -1) {
		  std::cerr << "PQflush error" << std::endl;
		}
		if (ret == 1)
		  std::this_thread::yield();
	  }
	}

	PGconn* pgconn_ = nullptr;
	int fd = -1;
	std::unordered_map<std::string, std::shared_ptr<pgsql_statement_data>> statements;
	type_hashmap<std::shared_ptr<pgsql_statement_data>> statements_hashmap;
	int error_ = 0;
  };

  struct pgsql_result {
  public:
	~pgsql_result() { if (current_result_) PQclear(current_result_); free(proto_name_);  proto_name_ = NULL; }
	// Read metamap and tuples.
	template <typename T> bool read(T&& t1);
	template <typename T> unsigned int readJson(T&& t1);
	long long int last_insert_id();
	// Flush all results.
	void flush_results();
	std::shared_ptr<pgsql_connection_data> connection_;
	int last_insert_id_ = -1;
	int row_i_ = 0;
	int current_result_nrows_ = 0;
	PGresult* current_result_ = nullptr;
	std::vector<int> curent_result_field_positions_;

	char** proto_name_ = nullptr;
	std::vector<Oid> proto_type_;
  private:

	// Wait for the next result.
	PGresult* wait_for_next_result();

	// Fetch a string from a result field.
	template <typename... A>
	void fetch_value(std::string& out, int field_i, Oid field_type);
	// Fetch a blob from a result field.
	template <typename... A> void fetch_value(sql_blob& out, int field_i, Oid field_type);
	// Fetch an int from a result field.
	void fetch_value(int& out, int field_i, Oid field_type);
	// Fetch an unsigned int from a result field.
	void fetch_value(unsigned int& out, int field_i, Oid field_type);
  };
  PGresult* pg_wait_for_next_result(PGconn* connection,
	int& connection_status, bool nothrow = false) {
	// std::cout << "WAIT ======================" << std::endl;
	while (true) {
	  if (PQconsumeInput(connection) == 0) {
		connection_status = 1;
		if (!nothrow)
		  throw std::runtime_error(std::string("PQconsumeInput() failed: ") +
			PQerrorMessage(connection));
		else
		  std::cerr << "PQconsumeInput() failed: " << PQerrorMessage(connection) << std::endl;
#ifdef DEBUG
		assert(0);
#endif
	  }

	  if (PQisBusy(connection)) {
		// std::cout << "isbusy" << std::endl;
		try {
		  std::this_thread::yield();
		} catch (std::runtime_error& e) {
		  // Free results.
		  // Yield thrown a exception (probably because a closed connection).
		  // Flush the remaining results.
		  while (true) {
			if (PQconsumeInput(connection) == 0) {
			  connection_status = 1;
			  break;
			}
			if (!PQisBusy(connection)) {
			  PGresult* res = PQgetResult(connection);
			  if (res) PQclear(res);
			  else break;
			}
		  }
		  throw std::move(e);
		}
	  } else {
		// std::cout << "notbusy" << std::endl;
		PGresult* res = PQgetResult(connection);
		if (PQresultStatus(res) == PGRES_FATAL_ERROR && PQerrorMessage(connection)[0] != 0) {
		  PQclear(res);
		  connection_status = 1;
		  if (!nothrow)
			throw std::runtime_error(std::string("Postresql fatal error:") +
			  PQerrorMessage(connection));
		  else
			std::cerr << "Postgresql FATAL error: " << PQerrorMessage(connection) << std::endl;
#ifdef DEBUG
		  assert(0);
#endif
		} else if (PQresultStatus(res) == PGRES_NONFATAL_ERROR)
		  std::cerr << "Postgresql non fatal error: " << PQerrorMessage(connection) << std::endl;

		return res;
	  }
	}
  }
  PGresult* pgsql_result::wait_for_next_result() {
	return pg_wait_for_next_result(connection_->pgconn_, connection_->error_);
  }

  void pgsql_result::flush_results() {
	try {
	  while (true) {
		if (connection_->error_ == 1) break;
		PGresult* res = pg_wait_for_next_result(connection_->pgconn_, connection_->error_, true);
		if (res)
		  PQclear(res);
		else break;
	  }
	} catch (std::runtime_error& e) {
	  // Forward fiber execptions.
	  throw std::move(e);
	}
  }

  // Fetch a string from a result field.
  template <typename... A>
  void pgsql_result::fetch_value(std::string& out, int field_i,
	Oid field_type) {
	// assert(!is_binary);
	// std::cout << "fetch string: " << length << " '"<< val <<"'" << std::endl;
	out = std::move(std::string(PQgetvalue(current_result_, row_i_, field_i),
	  PQgetlength(current_result_, row_i_, field_i)));
	// out = std::move(std::string(val, strlen(val)));
  }

  // Fetch a blob from a result field.
  template <typename... A>
  void pgsql_result::fetch_value(sql_blob& out, int field_i, Oid field_type) {
	// assert(is_binary);
	out = std::move(std::string(PQgetvalue(current_result_, row_i_, field_i),
	  PQgetlength(current_result_, row_i_, field_i)));
  }

  // Fetch an int from a result field.
  void pgsql_result::fetch_value(int& out, int field_i, Oid field_type) {
	assert(PQfformat(current_result_, field_i) == 1); // Assert binary format
	char* val = PQgetvalue(current_result_, row_i_, field_i);

	// TYPCATEGORY_NUMERIC
	// std::cout << "fetch integer " << length << " " << is_binary << std::endl;
	// std::cout << "fetch integer " << be64toh(*((uint64_t *) val)) << std::endl;
	if (field_type == INT8OID) {
	  // std::cout << "fetch 64b integer " << std::hex << int(32) << std::endl;
	  // std::cout << "fetch 64b integer " << std::hex << uint64_t(*((uint64_t *) val)) << std::endl;
	  // std::cout << "fetch 64b integer " << std::hex << (*((uint64_t *) val)) << std::endl;
	  // std::cout << "fetch 64b integer " << std::hex << be64toh(*((uint64_t *) val)) << std::endl;
	  out = be64toh(*((uint64_t*)val));
	} else if (field_type == INT4OID)
	  out = (uint32_t)ntohl(*((uint32_t*)val));
	else if (field_type == INT2OID)
	  out = (uint16_t)ntohs(*((uint16_t*)val));
	else
	  throw std::runtime_error("The type of request result does not match the destination type");
  }


  // Fetch an unsigned int from a result field.
  void pgsql_result::fetch_value(unsigned int& out, int field_i, Oid field_type) {
	assert(PQfformat(current_result_, field_i) == 1); // Assert binary format
	char* val = PQgetvalue(current_result_, row_i_, field_i);

	// if (length == 8)
	if (field_type == INT8OID)
	  out = be64toh(*((uint64_t*)val));
	else if (field_type == INT4OID)
	  out = ntohl(*((uint32_t*)val));
	else if (field_type == INT2OID)
	  out = ntohs(*((uint16_t*)val));
	else
	  assert(0);
  }

  template <typename T> uint32_t pgsql_result::readJson(T&& output) {
	int nfields = proto_type_.size();
	if (!current_result_ || row_i_ == current_result_nrows_) {
	  if (current_result_) {
		PQclear(current_result_);
		current_result_ = nullptr;
	  }
	  current_result_ = wait_for_next_result();
	  if (!current_result_)
		return 0;
	  row_i_ = 0;
	  current_result_nrows_ = PQntuples(current_result_);
	  if (current_result_nrows_ == 0) {
		PQclear(current_result_);
		current_result_ = nullptr;
		return 0;
	  }
	  if (nfields == 0) {
		proto_type_.resize(PQnfields(current_result_));
		nfields = proto_type_.size();
		proto_name_ = (char**)malloc(sizeof(char*) * nfields);
		for (int field_i = 0; field_i < nfields; ++field_i) {
		  proto_type_[field_i] = PQftype(current_result_, field_i);
		  proto_name_[field_i] = (char*)malloc(sizeof(char) * 51);
		  strcpy(proto_name_[field_i], PQfname(current_result_, field_i));
		}
	  }
	  }
	if (current_result_nrows_ == 1) {
	  for (int i = 0; i < nfields; ++i) {
		char* val = PQgetvalue(current_result_, 0, i);
		switch (proto_type_[i]) {
		case INT8OID:output[proto_name_[i]] = be64toh(*((uint64_t*)val)); break;
		case INT4OID:output[proto_name_[i]] = ntohl(*((uint32_t*)val)); break;
		case INT2OID:output[proto_name_[i]] = ntohs(*((uint16_t*)val)); break;
		case 25: {
#ifdef SYS_IS_UTF8
		  output[proto_name_[i]] = std::move(std::string(val, PQgetlength(current_result_, 0, i)));
#else
		  char* c = UnicodeToUtf8(val);
		  output[proto_name_[i]] = std::move(std::string(c, PQgetlength(current_result_, 0, i)));
		  free(c); c = NULL;
#endif // SYS_IS_UTF8
		} break;
		default:output[proto_name_[i]] = nullptr;
		  break;
		}
	  }
	  return 1;
	}
	for (T j; row_i_ < current_result_nrows_; ++row_i_) {
	  for (int i = 0; i < nfields; ++i) {
		char* val = PQgetvalue(current_result_, row_i_, i);
		switch (proto_type_[i]) {
		case INT8OID:j[proto_name_[i]] = be64toh(*((uint64_t*)val)); break;
		case INT4OID:j[proto_name_[i]] = ntohl(*((uint32_t*)val)); break;
		case INT2OID:j[proto_name_[i]] = ntohs(*((uint16_t*)val)); break;
		case 25: {
#ifdef SYS_IS_UTF8
		  j[proto_name_[i]] = std::move(std::string(val, PQgetlength(current_result_, row_i_, i)));
#else
		  char* c = UnicodeToUtf8(val);
		  j[proto_name_[i]] = std::move(std::string(c, PQgetlength(current_result_, row_i_, i)));
		  free(c); c = NULL;
#endif // SYS_IS_UTF8
		} break;
		default:j[proto_name_[i]] = nullptr;
		  break;
		}
	  }output.push_back(j);
	}
	return row_i_;
	}
  template <typename T> bool pgsql_result::read(T&& output) {
	int nfields = proto_type_.size();
	if (!current_result_ || row_i_ == current_result_nrows_) {
	  if (current_result_) {
		PQclear(current_result_);
		current_result_ = nullptr;
	  }
	  current_result_ = wait_for_next_result();
	  if (!current_result_)
		return false;
	  row_i_ = 0;
	  current_result_nrows_ = PQntuples(current_result_);
	  if (current_result_nrows_ == 0) {
		PQclear(current_result_);
		current_result_ = nullptr;
		return false;
	  }
	  if (nfields == 0) {
		proto_type_.resize(PQnfields(current_result_));
		nfields = proto_type_.size();
		for (int field_i = 0; field_i < nfields; ++field_i)
		  proto_type_[field_i] = PQftype(current_result_, field_i);
	  }
	}
	int i = 0;
	//if constexpr (is_tuple<T>::value) {}
	if (nfields != std::tuple_size_v<std::decay_t<T>>)
	  throw std::runtime_error("postgresql error: in fetch: Mismatch between the request number of "
		"field and the outputs.");
	crow::tuple_map(std::forward<T>(output), [&](auto& m) {
	  fetch_value(m, i, proto_type_[i]);
	  ++i;
	  });
	++this->row_i_;
	return true;
  }
  long long int pgsql_result::last_insert_id() {
	// while (PGresult* res = wait_for_next_result())
	//  PQclear(res);
	// PQsendQuery(connection_, "LASTVAL()");
	int t = 0;
	this->read(std::tie(t));
	return t;
	// PGresult *PQexec(connection_, const char *command);
	// this->operator()
	//   last_insert_id_ = PQoidValue(res);
	//   std::cout << "id " << last_insert_id_ << std::endl;
	//   PQclear(res);
	// }
	// return last_insert_id_;
  }

  struct pgsql_statement_data : std::enable_shared_from_this<pgsql_statement_data> {
	pgsql_statement_data(const std::string& s) : stmt_name(s) {}
	std::string stmt_name;
  };

  struct pgsql_statement {
  public:
	template <typename... T> sql_result<pgsql_result> operator()(T&&... args);
	std::shared_ptr<pgsql_connection_data> connection_;
	pgsql_statement_data& data_;

  private:

	template <unsigned N>
	void bind_param(sql_varchar<N>&& m, const char** values, int* lengths, int* binary);
	template <unsigned N>
	void bind_param(const sql_varchar<N>& m, const char** values, int* lengths, int* binary);
	void bind_param(const char* m, const char** values, int* lengths, int* binary);
	template <typename T>
	void bind_param(const std::vector<T>& m, const char** values, int* lengths, int* binary);
	template <typename T> void bind_param(const T& m, const char** values, int* lengths, int* binary);
	template <typename T> unsigned int bind_compute_nparam(const T& arg);
	template <typename T> unsigned int bind_compute_nparam(const std::vector<T>& arg);

  };

  template <unsigned N>
  void pgsql_statement::bind_param(sql_varchar<N>&& m, const char** values, int* lengths,
	int* binary) {
	*values = m.c_str();
	*lengths = m.size();
	*binary = 0;
  }
  template <unsigned N>
  void pgsql_statement::bind_param(const sql_varchar<N>& m, const char** values, int* lengths,
	int* binary) {
	*values = m.c_str();
	*lengths = m.size();
	*binary = 0;
  }
  void pgsql_statement::bind_param(const char* m, const char** values, int* lengths, int* binary) {
	*values = m;
	*lengths = strlen(m);
	*binary = 0;
  }

  template <typename T>
  void pgsql_statement::bind_param(const std::vector<T>& m, const char** values, int* lengths,
	int* binary) {
	int i = 0;
	for (int i = 0; i < m.size(); ++i)
	  bind_param(m[i], values + i, lengths + i, binary + i);
  }

  template <typename T>
  void pgsql_statement::bind_param(const T& m, const char** values, int* lengths, int* binary) {
	if constexpr (std::is_same<std::decay_t<decltype(m)>, std::string>::value ||
	  std::is_same<std::decay_t<decltype(m)>, std::string_view>::value) {
	  *values = m.c_str();
	  *lengths = m.size();
	  *binary = 0;
	} else if constexpr (std::is_same<std::remove_reference_t<decltype(m)>, const char*>::value) {
	  *values = m;
	  *lengths = strlen(m);
	  *binary = 0;
	} else if constexpr (std::is_same<std::decay_t<decltype(m)>, int>::value) {
	  *values = (char*)new int(htonl(m));
	  *lengths = sizeof(m);
	  *binary = 1;
	} else if constexpr (std::is_same<std::decay_t<decltype(m)>, long long int>::value) {
	  *values = (char*)new int(htonl(uint32_t(m)));
	  *lengths = sizeof(uint32_t);
	  *binary = 1;
	}
  }

  template <typename T>
  unsigned int pgsql_statement::bind_compute_nparam(const T& arg) {
	return 1;
  }
  template <typename T>
  unsigned int pgsql_statement::bind_compute_nparam(const std::vector<T>& arg) {
	return arg.size() * bind_compute_nparam(arg[0]);
  }

  template <typename... T>
  sql_result<pgsql_result> pgsql_statement::operator()(T&&... args) {

	unsigned int nparams = 0;
	if constexpr (sizeof...(T) > 0)
	  nparams = (bind_compute_nparam(std::forward<T>(args)) + ...);

#ifdef _WIN32
	std::vector<const char*> values_(nparams);
	std::vector<int> lengths_(nparams);
	std::vector<int> binary_(nparams);
	const char** values = values_.data();
	int* lengths = lengths_.data();
	int* binary = binary_.data();
#else
	const char* values_[nparams];
	int lengths_[nparams];
	int binary_[nparams];
	const char** values = values_;
	int* lengths = lengths_;
	int* binary = binary_;
#endif

	int i = 0;
	tuple_map(std::forward_as_tuple(args...), [&](const auto& a) {
	  bind_param(a, values + i, lengths + i, binary + i);
	  i += bind_compute_nparam(a);
	  });

	if (!PQsendQueryPrepared(connection_->pgconn_, data_.stmt_name.c_str(), nparams, values, lengths, binary,
	  1)) {
	  throw std::runtime_error(std::string("Postresql error:") + PQerrorMessage(connection_->pgconn_));
	}


	return sql_result<pgsql_result>{
	  pgsql_result{ this->connection_ }};
  }


  struct pgsql_tag {};

  struct pgsql_connection {
	std::shared_ptr<pgsql_connection_data> data_;
	std::unordered_map<std::string, std::shared_ptr<pgsql_statement_data>>& stm_cache_;
	PGconn* connection_;

	typedef pgsql_tag db_tag;

	inline pgsql_connection(const pgsql_connection&) = delete;
	inline pgsql_connection& operator=(const pgsql_connection&) = delete;
	inline pgsql_connection(pgsql_connection&& o) = default;

	inline pgsql_connection(std::shared_ptr<pgsql_connection_data>& data)
	  : data_(data), stm_cache_(data->statements), connection_(data->pgconn_) {

	}



	auto operator()(const std::string& rq) {
	  if (!PQsendQueryParams(connection_, rq.c_str(), 0, nullptr, nullptr, nullptr, nullptr, 1))
		throw std::runtime_error(std::string("Postresql error:") + PQerrorMessage(connection_));
	  return sql_result<pgsql_result>{
		pgsql_result{ this->data_, data_->error_ }};
	}
	inline std::string p_(int pos) {
	  std::stringstream ss; ss << "$" << pos; return ss.str();
	}
	template <typename F, typename... K> pgsql_statement cached_statement(F f, K... keys) {
	  if (data_->statements_hashmap(f, keys...).get() == nullptr) {
		pgsql_statement res = query(f());
		data_->statements_hashmap(f, keys...) = res.data_.shared_from_this();
		return res;
	  } else
		return pgsql_statement{ data_, *data_->statements_hashmap(f, keys...) };
	}
	pgsql_statement query(std::string& rq) {
	  uint16_t i = rq.size(), l = 0; while (rq[--i] != 0x28);
	  while (rq[i++]) {
		if (rq[i] == 0x3f) rq.replace(i, 1, p_(++l));
	  }
	  auto it = stm_cache_.find(rq);
	  if (it != stm_cache_.end()) {
		return pgsql_statement{ data_, *it->second };
	  }
	  std::string stmt_name = boost::lexical_cast<std::string>(stm_cache_.size());

	  if (!PQsendPrepare(connection_, stmt_name.c_str(), rq.c_str(), 0, nullptr)) {
		throw std::runtime_error(std::string("PQsendPrepare error") + PQerrorMessage(connection_));
	  }

	  while (PGresult* ret = pg_wait_for_next_result(connection_, data_->error_))
		PQclear(ret);

	  auto pair = stm_cache_.emplace(rq, std::make_shared<pgsql_statement_data>(stmt_name));
	  return pgsql_statement{ data_, *pair.first->second };
	}
	pgsql_statement query(const std::string& rq) {
	  std::string rq_ = rq;
	  uint16_t i = rq_.size(), l = 0; while (rq_[--i] != 0x28);
	  while (rq_[i++]) {
		if (rq_[i] == 0x3f) rq_.replace(i, 1, p_(++l));
	  }
	  auto it = stm_cache_.find(rq_);
	  if (it != stm_cache_.end()) {
		return pgsql_statement{ data_, *it->second };
	  }
	  std::string stmt_name = boost::lexical_cast<std::string>(stm_cache_.size());

	  if (!PQsendPrepare(connection_, stmt_name.c_str(), rq_.c_str(), 0, nullptr)) {
		throw std::runtime_error(std::string("PQsendPrepare error") + PQerrorMessage(connection_));
	  }

	  while (PGresult* ret = pg_wait_for_next_result(connection_, data_->error_))
		PQclear(ret);

	  auto pair = stm_cache_.emplace(rq_, std::make_shared<pgsql_statement_data>(stmt_name));
	  return pgsql_statement{ data_, *pair.first->second };
	}
	pgsql_statement query(const char* rq) {
	  auto it = stm_cache_.find(rq);
	  if (it != stm_cache_.end()) {
		return pgsql_statement{ data_, *it->second };
	  }
	  std::string stmt_name = boost::lexical_cast<std::string>(stm_cache_.size());

	  if (!PQsendPrepare(connection_, stmt_name.c_str(), rq, 0, nullptr)) {
		throw std::runtime_error(std::string("PQsendPrepare error") + PQerrorMessage(connection_));
	  }

	  while (PGresult* ret = pg_wait_for_next_result(connection_, data_->error_))
		PQclear(ret);

	  auto pair = stm_cache_.emplace(rq, std::make_shared<pgsql_statement_data>(stmt_name));
	  return pgsql_statement{ data_, *pair.first->second };
	}
	template <typename T>
	inline std::string type_to_string(const T&, std::enable_if_t<std::is_integral<T>::value>* = 0) {
	  return "INT";
	}
	template <typename T>
	inline std::string type_to_string(const T&,
	  std::enable_if_t<std::is_floating_point<T>::value>* = 0) {
	  return "DOUBLE";
	}
	inline std::string type_to_string(const std::string&) { return "TEXT"; }
	inline std::string type_to_string(const sql_blob&) { return "BLOB"; }
	template <unsigned S> inline std::string type_to_string(const sql_varchar<S>) {
	  std::ostringstream ss;
	  ss << "VARCHAR(" << S << ')';
	  return ss.str();
	}
  };

  struct pgsql {
	typedef pgsql_connection_data connection_data_type;
	typedef pgsql_tag db_tag;
	std::string host_, user_, passwd_, database_;
	unsigned int port_;
	std::string character_set_;
	std::string conninfo_;
	inline pgsql(const char* host, const char* database, const char* user, const char* password, ...) {
	  ASSERT(host, "open_pgsql_connection requires the host argument");
	  ASSERT(database, "open_pgsql_connection requires the database argument");
	  ASSERT(user, "open_pgsql_connection requires the user argument");
	  ASSERT(password, "open_pgsql_connection requires the password argument");
	  va_list ap; va_start(ap, password); unsigned int i = va_arg(ap, unsigned int); char* c = va_arg(ap, char*); port_ = i < 0xffff ? i : 0x1538; character_set_ = c[0] ? c : "utf8"; va_end(ap); host_ = host, database_ = database, user_ = user, passwd_ = password;
	  if (!PQisthreadsafe()) throw std::runtime_error("LibPQ is not threadsafe.");
	  std::stringstream coninfo; coninfo << "postgresql://" << user_ << ":" << passwd_ << "@"
		<< host_ << ":" << port_ << "/" << database_ << "?client_encoding=" << character_set_;
	  conninfo_ = coninfo.str();
	}
	inline bool ping() { return PQping(conninfo_.c_str()) == 0; }//pgsql is alive
	inline int get_socket(const std::shared_ptr<pgsql_connection_data>& data) {
	  return PQsocket(data->pgconn_);
	}
	inline pgsql_connection_data* new_connection() {
	  PGconn* connection = nullptr;
	  int pgsql_fd = -1;
	  connection = PQconnectdb(conninfo_.c_str());
	  //connection = PQconnectStart(conninfo_);
	  if (!connection) {
		std::cerr << "Warning: PQconnectStart returned null." << std::endl;
		return nullptr;
	  }
	  if (PQsetnonblocking(connection, 1) == -1) {
		std::cerr << "Warning: PQsetnonblocking returned -1: " << PQerrorMessage(connection)
		  << std::endl;
		PQfinish(connection);
		return nullptr;
	  }

	  int status = PQconnectPoll(connection);

	  pgsql_fd = PQsocket(connection);
	  if (pgsql_fd == -1) {
		std::cerr << "Warning: PQsocket returned -1: " << PQerrorMessage(connection) << std::endl;
		PQfinish(connection);
		return nullptr;
	  }
	  try {
		while (status != PGRES_POLLING_FAILED && status != PGRES_POLLING_OK) {
		  int new_pgsql_fd = PQsocket(connection);
		  if (new_pgsql_fd != pgsql_fd) {
			pgsql_fd = new_pgsql_fd;
		  }
		  std::this_thread::sleep_for(std::chrono::microseconds(314));
		  status = PQconnectPoll(connection);
		}
	  } catch (std::runtime_error& e) {
		PQfinish(connection);
		throw std::move(e);
	  }
	  if (status != PGRES_POLLING_OK) {
		std::cerr << "Warning: cannot conn to the postgresql server " << host_ << ": "
		  << PQerrorMessage(connection) << std::endl;
		PQfinish(connection);
		return nullptr;
	  }

	  return new pgsql_connection_data{ connection, pgsql_fd };
	}
	auto scoped_connection(std::shared_ptr<pgsql_connection_data>& data) {
	  return pgsql_connection(data);
	}
  };
  struct Timer {
	template<typename F> void setTimeout(F func, uint32_t milliseconds);
	template<typename F> void setTimeoutSec(F func, uint32_t seconds);
	template<typename F> void setInterval(F func, uint32_t milliseconds);
	template<typename F> void setIntervalSec(F func, uint32_t seconds);
	void stop();
  private:
	std::atomic<bool> alive{ true };
  };
  template<typename F>
  void Timer::setTimeout(F func, uint32_t milliseconds) {
	alive = true;
	std::thread t([=]() {
	  std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
	  if (!alive.load()) return; func();
	  });
	t.detach();
  }
  template<typename F>
  void Timer::setTimeoutSec(F func, uint32_t seconds) {
	alive = true;
	std::thread t([=]() {
	  std::this_thread::sleep_for(std::chrono::seconds(seconds));
	  if (!alive.load()) return; func();
	  });
	t.detach();
  }
  template<typename F>
  void Timer::setInterval(F func, uint32_t milliseconds) {
	alive = true;
	std::thread t([=]() {
	  while (alive.load()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
		if (!alive.load()) return; func();
	  }
	  });
	t.detach();
  }
  template<typename F>
  void Timer::setIntervalSec(F func, uint32_t seconds) {
	alive = true;
	std::thread t([=]() {
	  while (alive.load()) {
		std::this_thread::sleep_for(std::chrono::seconds(seconds));
		if (!alive.load()) return; func();
	  }
	  });
	t.detach();
  }
  void Timer::stop() { alive = false; }
  template <typename I, int Time = 28799> struct sql_database {
	I impl; Timer timer;
	typedef typename I::connection_data_type connection_data_type;
	typedef typename I::db_tag db_tag;
	std::deque<connection_data_type*> sync_connections_;std::mutex sync_connections_mutex_;
	int n_sync_connections_ = 0, max_sync_connections_ = 0;
	sql_database(unsigned int port, const char* host, const char* database, const char* user, const char* password, unsigned int max_sync_connections = MaxSyncConnections)
	  : impl(host, database, user, password, port), max_sync_connections_(max_sync_connections) { init(); };
	sql_database(const char* host, const char* database, const char* user, const char* password, const char* charset, unsigned int max_sync_connections = MaxSyncConnections)
	  : impl(host, database, user, password, 3306, charset), max_sync_connections_(max_sync_connections) { init(); };
	sql_database(const char* host, const char* database, const char* user, const char* password, unsigned int port, const char* charset, unsigned int max_sync_connections = MaxSyncConnections)
	  : impl(host, database, user, password, port, charset), max_sync_connections_(max_sync_connections) { init(); };
	~sql_database() { flush(); }
	inline void init() {
	  if constexpr (std::is_same_v<db_tag, mysql_tag>) {
		connection_data_type* data = impl.new_connection();
		assert(data); sync_connections_.push_back(data);
		timer.setIntervalSec([this]() { impl.ping(sync_connections_.back()); }, Time);
	  } else if constexpr (std::is_same_v<db_tag, pgsql_tag>) {
		timer.setIntervalSec([this]() { impl.ping(); }, Time);
	  }
	}
	void flush() {
	  std::lock_guard<std::mutex> lock(this->sync_connections_mutex_);
	  for (auto* ptr : this->sync_connections_) delete ptr;
	  sync_connections_.clear();
	  n_sync_connections_ = 0;
	}
	inline auto conn() {
	  connection_data_type* data = nullptr;
	  bool reuse = true;
	  if (!sync_connections_.empty()) {
		std::lock_guard<std::mutex> lock(this->sync_connections_mutex_);
		data = sync_connections_.back();
		sync_connections_.pop_back();
		reuse = false;
	  } else {
	  _:
		if (n_sync_connections_ > max_sync_connections_) {
		  std::this_thread::yield();
		  goto _;
		}
		++n_sync_connections_;
		try { data = impl.new_connection(); } catch (std::runtime_error& e) {
		  --n_sync_connections_;
		  throw std::move(e);
		}
		if (!data) { --n_sync_connections_; std::this_thread::yield(); goto _; }
	  }
	  assert(data); assert(data->error_ == 0);
	  auto sptr = std::shared_ptr<connection_data_type>(data, [this, &reuse](connection_data_type* data) {
		if (!data->error_ && sync_connections_.size() < max_sync_connections_) {
		  std::lock_guard<std::mutex> lock(this->sync_connections_mutex_);
		  sync_connections_.push_back(data);
		} else { --n_sync_connections_; if (reuse)delete data; }
		});
	  return impl.scoped_connection(sptr);
	}
  };
  //SqlDataBase will automatically shut down after 8 hours (28800 seconds) of
  // inactivity by default (determined by the mechanism provided by the server)
  //typedef sql_database<sqlType,time_wait> D;time_wait default 28799
  //I set time_wait and interactive_timeout is 100
  typedef sql_database<mysql, 99> Mysql;
  typedef sql_database<pgsql, 99> Pgsql;
  //-------------- utf8 / GB2312 / GBK --------------
#define D_mysql() crow::Mysql("127.0.0.1","test","root","",3306,SYS_IS_UTF8?"utf8":"GBK")
#define D_pgsql() crow::Pgsql("127.0.0.1","test","Asciphx","",5432,SYS_IS_UTF8?"utf8":"GBK")
//------ Use GBK or GB2312 to support Chinese ------
//---- SQLite can only support default encoding ----
#define D_sqlite(path) crow::Sqlite(path)
}
