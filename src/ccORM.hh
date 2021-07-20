#pragma once
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

#include <mysql/mysql.h>

#include <atomic>
#include <stdarg.h>
#include <stdexcept>
#include <functional>
#include <assert.h>
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

	template <typename E, typename F> E& get_cache_entry(int& hash, F)
	{
	  if (hash == -1)
	  {
		std::lock_guard lock(mutex_);
		if (hash == -1)
		  hash = counter_++;
	  }
	  if (hash >= values_.size() or !values_[hash].has_value())
	  {
		if (values_.size() < hash + 1)
		  values_.resize(hash + 1);
		values_[hash] = E();
	  }

	  return std::any_cast<E&>(values_[hash]);
	}
	template <typename K, typename F> V& operator()(F f, K key)
	{
	  static int hash = -1;
	  return this->template get_cache_entry<std::unordered_map<K, V>>(hash, f)[key];
	}

	template <typename F> V& operator()(F f)
	{
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
  static sql_null_t null;

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
  };

  template <typename B>
  template <typename T1, typename... T>
  bool sql_result<B>::r__(T1&& t1, T&... tail) {
	if constexpr (crow::is_tuple<std::decay_t<T1>>::value) {
	  static_assert(sizeof...(T) == 0);
	  return impl_.read(std::forward<T1>(t1));
	}
	else
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

#define IS_VALID(T, EXPR) internal::is_valid<T>( [](auto&& obj)->decltype(obj.EXPR){} )

  template <typename B> template <typename F> void sql_result<B>::map(F map_function) {


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
	enum { is_blocking = true };

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
  template <typename Y> struct mysql_functions_non_blocking {
	enum { is_blocking = false };

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
		  fiber_.yield();
		}
		catch (typename Y::exception_type& e) {
		  connection_status = 1;
		  throw std::move(e);
		}

		status = fn_cont(&ret, std::forward<A1>(a1), status);
	  }
	  if (ret && ret != MYSQL_NO_DATA && ret != MYSQL_DATA_TRUNCATED)
	  {
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

	  Y& fiber_;
  };

#else
#error Only the MariaDB libmysqlclient is supported.
#endif
}
namespace crow {
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
	  if (connection_)       mysql_wrapper_.mysql_stmt_free_result(connection_->error_, data_.stmt_);
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
	}
	catch (const std::runtime_error& e) {
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
	  memset(bind, 0, sizeof(bind));	  int i = 0;
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
	B& mysql_wrapper_; 	std::shared_ptr<mysql_connection_data> connection_;
	MYSQL_RES* result_ = nullptr; 	unsigned long* current_row_lengths_ = nullptr;
	MYSQL_ROW current_row_ = nullptr;
	bool end_of_result_ = false;
	int current_row_num_fields_ = 0;
	mysql_result(B& mysql_wrapper_, std::shared_ptr<mysql_connection_data> connection_)
	  : mysql_wrapper_(mysql_wrapper_), connection_(connection_) {}
	mysql_result& operator=(mysql_result&) = delete;
	mysql_result(const mysql_result&) = delete;
	mysql_result(mysql_result&&) = default;
	inline ~mysql_result() { flush(); }
	inline void flush() {
	  if (result_) {
		mysql_free_result(result_);
		result_ = nullptr;
	  }
	}
	inline void flush_results() { this->flush(); }
	inline void next_row();
	template <typename T> bool read(T&& output);

	long long int affected_rows();

	long long int last_insert_id();
  };

  template <typename B> void mysql_result<B>::next_row() {

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

  template <typename B> template <typename T> bool mysql_result<B>::read(T&& output) {

	next_row();

	if (end_of_result_)
	  return false;

	if (std::tuple_size_v<std::decay_t<T>> != current_row_num_fields_)
	  throw std::runtime_error(std::string("The request number of field (") +
		boost::lexical_cast<std::string>(current_row_num_fields_) +
		") does not match the size of the tuple (" +
		boost::lexical_cast<std::string>(std::tuple_size_v<std::decay_t<T>>) + ")");
	int i = 0;
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

	mysql_statement<B> prepare(const std::string& rq);

	template <typename F, typename... K> mysql_statement<B> cached_statement(F f, K... keys);
	template <typename T> std::string type_to_string(const T& t) { return cpptype_to_mysql_type(t); }
	B mysql_wrapper_;
	std::shared_ptr<mysql_connection_data> data_;
  };
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
	  mysql_statement<B> res = prepare(f());
	  data_->statements_hashmap_(f, keys...) = res.data_.shared_from_this();
	  return res;
	}
	else
	  return mysql_statement<B>{mysql_wrapper_, * data_->statements_hashmap_(f, keys...),
	  data_};
  }
  template <typename B> mysql_statement<B> mysql_connection<B>::prepare(const std::string& rq) {
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
	}
	catch (...) {
	  mysql_stmt_close(stmt);
	  throw;
	}
	auto pair = data_->statements_.emplace(rq, std::make_shared<mysql_statement_data>(stmt));
	return mysql_statement<B>{mysql_wrapper_, * pair.first->second, data_};
  }
}
#define MaxSyncConnections 2000
#define EXPECT_THROW(STM)\
try {STM;std::cerr << "This must have thrown an exception: " << #STM << std::endl; } catch (...) {}
#define EXPECT_EQUAL(A, B)\
if(A!=B){std::cerr << #A << " (== " << A << ") " << " != " << #B << " (== " << B << ") " << std::endl;}
#define EXPECT(A)\
  if (!(A)) { std::cerr << #A << " (== " << (A) << ") must be true" << std::endl; }
#define ASSERT(x,m) if(!x)std::cerr<<#m
namespace crow {
  template <typename... T> std::ostream& operator<<(std::ostream& os, std::tuple<T...> t) {
	os << "TUPLE<"; tuple_map(std::forward<std::tuple<T...>>(t), [&os](auto v) { os << v << " ,"; }); os << ">"; return os;
  }
  struct mysql_database_impl {
	typedef mysql_tag db_tag;
	typedef mysql_connection_data connection_data_type;
	inline mysql_database_impl(const char* host, const char* database, const char* user, const char* password, ...);
	inline ~mysql_database_impl();
	inline mysql_connection_data* new_connection();
	inline int get_socket(const std::shared_ptr<mysql_connection_data>& data);    inline mysql_connection<mysql_functions_blocking> scoped_connection(std::shared_ptr<mysql_connection_data>& data);
	std::string host_, user_, passwd_, database_, character_set_;
	unsigned int port_;
  };
  inline mysql_database_impl::mysql_database_impl(const char* host, const char* database, const char* user, const char* password, ...) {
	ASSERT(host, "open_mysql_connection requires the host argument");
	ASSERT(database, "open_mysql_connection requires the databaser argument");
	ASSERT(user, "open_mysql_connection requires the user argument");
	ASSERT(password, "open_mysql_connection requires the password argument");
	va_list ap; va_start(ap, password); unsigned int i = va_arg(ap, unsigned int); char* c = va_arg(ap, char*);
	port_ = i < 0xffff ? i : 0xcea; character_set_ = c[0] ? c : "utf8"; va_end(ap);
	host_ = host, database_ = database, user_ = user, passwd_ = password;
	if (mysql_library_init(0, NULL, NULL)) throw std::runtime_error("Could not initialize MySQL library.");
	if (!mysql_thread_safe()) throw std::runtime_error("Mysql is not compiled as thread safe.");
  }
  mysql_database_impl::~mysql_database_impl() { mysql_library_end(); }
  inline int mysql_database_impl::get_socket(const std::shared_ptr<mysql_connection_data>& data) {
	return mysql_get_socket(data->connection_);
  }
  inline mysql_connection_data* mysql_database_impl::new_connection() {
	MYSQL* mysql; MYSQL* connection;
	mysql = mysql_init(nullptr); connection = mysql;
	connection = mysql_real_connect(connection, host_.c_str(), user_.c_str(), passwd_.c_str(), database_.c_str(), port_, NULL, 0);
	if (!connection) return nullptr;
	char on = 1; mysql_options(mysql, MYSQL_REPORT_DATA_TRUNCATION, &on);
	mysql_set_character_set(mysql, character_set_.c_str());
	return new mysql_connection_data{ mysql };
  }
  inline mysql_connection<mysql_functions_blocking> mysql_database_impl::scoped_connection(
	std::shared_ptr<mysql_connection_data>& data) {
	return mysql_connection<mysql_functions_blocking>(mysql_functions_blocking{}, data);
  }
  template <typename I> struct sql_database {
	I impl;
	typedef typename I::connection_data_type connection_data_type;
	typedef typename I::db_tag db_tag;
	std::deque<connection_data_type*> sync_connections_;	std::mutex sync_connections_mutex_;	int n_sync_connections_ = 0;
	int max_sync_connections_ = 0;
	sql_database(unsigned int port, const char* host, const char* database, const char* user, const char* password, unsigned int max_sync_connections = MaxSyncConnections)
	  : impl(host, database, user, password, port), max_sync_connections_(max_sync_connections) {};
	sql_database(const char* host, const char* database, const char* user, const char* password, const char* charset, unsigned int max_sync_connections = MaxSyncConnections)
	  : impl(host, database, user, password, 3306, charset), max_sync_connections_(max_sync_connections) {};
	sql_database(const char* host, const char* database, const char* user, const char* password, unsigned int port, const char* charset, unsigned int max_sync_connections = MaxSyncConnections)
	  : impl(host, database, user, password, port, charset), max_sync_connections_(max_sync_connections) {};
	~sql_database() { flush(); }
	void flush() {
	  std::lock_guard<std::mutex> lock(this->sync_connections_mutex_);
	  for (auto* ptr : this->sync_connections_) delete ptr;
	  sync_connections_.clear();
	  n_sync_connections_ = 0;
	}
	inline auto conn() {
	  ++n_sync_connections_;
	  connection_data_type* data = nullptr;
	  if (!sync_connections_.empty()) {
		auto lock = [this] {
		  return std::lock_guard<std::mutex>(this->sync_connections_mutex_);
		}();
		data = sync_connections_.back();
		sync_connections_.pop_back();
	  }
	  else {
		if (n_sync_connections_ >= max_sync_connections_) {
		  flush();
		  std::this_thread::sleep_for(std::chrono::milliseconds(200));
		  goto _;
		  throw std::runtime_error("Maximum number of sql connection exeeded.");
		}
		_:try { data = impl.new_connection(); }
		catch (std::runtime_error& e) {
		  --n_sync_connections_;
		  throw std::move(e);
		}
		if (!data)
		  --n_sync_connections_;
	  }
	  assert(data);
	  assert(data->error_ == 0);
	  auto sptr = std::shared_ptr<connection_data_type>(data, [this](connection_data_type* data) {
		if (!data->error_ && sync_connections_.size() < max_sync_connections_) {
		  auto lock = [this] {
			return std::lock_guard<std::mutex>(this->sync_connections_mutex_);
		  }();
		  sync_connections_.push_back(data);
		}
		else {
		  --n_sync_connections_;
		  delete data;
		}
		});
	  return impl.scoped_connection(sptr);
	}
  };
  using sql_type = crow::mysql_database_impl;
  typedef sql_database<sql_type> D;
#define D__(a, b, c, d,...) D(a,b,c,d,##__VA_ARGS__)
#define D_() D("127.0.0.1","mysql_test","root","",3306,"utf8")
}
