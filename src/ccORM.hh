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
#include <libpq-fe.h>
#include <sqlite3.h>

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
//#include <catalog/pg_type_d.h>
#if __APPLE__ // from https://gist.github.com/yinyin/2027912

#include <libkern/OSByteOrder.h>
#define be64toh(x) OSSwapBigToHostInt64(x)
#elif _WIN32 // from https://gist.github.com/PkmX/63dd23f28ba885be53a5
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
    // void*,
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
  } // namespace internal
  // Traits on callable (function, functors and lambda functions).

  // callable_traits<F>::is_callable = true_type if F is callable.
  // callable_traits<F>::arity = N if F takes N arguments.
  // callable_traits<F>::arguments_tuple_type = tuple<Arg1, ..., ArgN>

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
      // Init hash if needed.
      if (hash == -1)
      {
        std::lock_guard lock(mutex_);
        if (hash == -1)
          hash = counter_++;
      }
      // Init cache if miss.
      if (hash >= values_.size() or !values_[hash].has_value())
      {
        if (values_.size() < hash + 1)
          values_.resize(hash + 1);
        values_[hash] = E();
      }

      // Return existing cache entry.
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
  /**
   * @brief Provide access to the result of a sql query.
   */
  template <typename I> struct sql_result {
    I impl_;
    sql_result() = delete;
    sql_result& operator=(sql_result&) = delete;
    sql_result(const sql_result&) = delete;
    sql_result(I&& impl) : impl_(std::forward<I>(impl)) {}

    inline ~sql_result() { this->flush_results(); }

    inline void flush_results() { impl_.flush_results(); }

    /**
     * @brief Return the last id generated by a insert comment.
     * With postgresql, it requires the previous command to use the "INSERT [...] returning id;"
     * syntax.
     *
     * @return the last inserted id.
     */
    long long int last_insert_id() { return impl_.last_insert_id(); }

    /**
     * @brief read one row of the result set and advance to next row.
     * Throw an exception if the end of the result set is reached.
     *
     * @return If only one template argument is provided return this same type.
     *         otherwise return a std::tuple of the requested types.
     */
    template <typename T1, typename... T> auto r__();
    /**
     * @brief Like read<T>() but do not throw is the eand of the result set is reached, instead
     * it wraps the result in a std::optional that is empty if no data could be fetch.
     *
     */
    template <typename T1, typename... T> auto read_optional();

    /**
     * @brief read one row of the result set and advance to next row.
     * Throw an exception if the end of the result set is reached or if another error happened.
     *
     * Valid calls are:
     *    read(std::tuple<...>& )
     *        fill the tuple according to the current row. The tuple must match
     *        the number of fields in the request.
     *    read(crow::metamap<...>& )
     *        fill the metamap according to the current row. The metamap (value types and keys) must
     * match the fields (types and names) of the request.
     *    read(A& a, B& b, C& c, ...)
     *        fill a, b, c,...
     *        with each field of the current row. Types of a, b, c... must match the types of the fields.
     *        supported types : only values (i.e not tuples or metamaps) like std::string, integer and floating numbers.
     * @return T the result value.
     */
    template <typename T1, typename... T> bool r__(T1&& t1, T&... tail);
    template <typename T> void r__(std::optional<T>& o);

    /**
     * @brief Call \param f on each row of the set.
     * The function must take as argument all the select fields of the request, it should
     * follow one of the signature of read (check read documentation for more info).
     * \param f can take arguments by reference to avoid copies but keep in mind that
     * there references will be invalid at the end of the function scope.
     *
     * @example connection.query("Select id,post from post_items;")().map(
     *        [&](std::string id, std::string post) {
     *             std::cout << id << post << std::endl; });
     *
     * @param f the function.
     */
    template <typename F> void map(F f);
  };

  template <typename B>
  template <typename T1, typename... T>
  bool sql_result<B>::r__(T1&& t1, T&... tail) {
    // Metamap and tuples
    if constexpr (crow::is_tuple<std::decay_t<T1>>::value) {
      static_assert(sizeof...(T) == 0);
      return impl_.read(std::forward<T1>(t1));
    }
    // Scalar values.
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
} // namespace crow
/**
 * @brief Store the data that mysql_statement holds.
 */
struct mysql_statement_data : std::enable_shared_from_this<mysql_statement_data> {
  MYSQL_STMT* stmt_ = nullptr;
  int num_fields_ = -1;
  MYSQL_RES* metadata_ = nullptr;
  MYSQL_FIELD* fields_ = nullptr;
  mysql_statement_data(MYSQL_STMT* stmt) {
    // std::cout << "create statement " << std::endl;
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
    // std::cout << "delete statement " << std::endl;
  }
};
namespace crow {
  /** @brief Data of a connection.*/
  struct mysql_connection_data {
    ~mysql_connection_data() { mysql_close(connection_); }
    MYSQL* connection_;
    std::unordered_map<std::string, std::shared_ptr<mysql_statement_data>> statements_;
    type_hashmap<std::shared_ptr<mysql_statement_data>> statements_hashmap_;
    int error_ = 0;
  };

  // Blocking version.
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

  // ========================================================================
  // =================== MARIADB ASYNC WRAPPERS =============================
  // ========================================================================
#ifdef LIBMARIADB
// Non blocking version.
  struct mysql_functions_non_blocking {
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
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        catch (std::runtime_error& e) {
          // Yield thrown a exception (probably because a closed connection).
          // Mark the connection as broken because it is left in a undefined state.
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
  };

#else
// MYSQL not supported yet because it does not have a
// nonblocking API for prepared statements.
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
  // Bind parameter functions,Used to bind input parameters of prepared statement.
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
  // Bind output function. Used to bind output values to result sets.
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
  // template <typename... A>
  // mysql_bind_data<sizeof...(A)> mysql_bind_output(mysql_statement_data& data,metamap<A...>& o) {
     //if (data.num_fields_!=sizeof...(A)) {
     //  throw std::runtime_error("mysql_statement error: Not enough columns in the result set to fill the object.");
     //}
     //mysql_bind_data<sizeof...(A)> bind_data;
     //MYSQL_BIND* bind = bind_data.bind.data();
     //unsigned long* real_lengths = bind_data.real_lengths.data();
     //crow::map(o,[&](auto k,auto& v) {
     //  // Find crow::symbol_string(k) position.
     //  for (int i = 0; i<data.num_fields_; ++i)
     //	if (!strcmp(data.fields_[i].name,crow::symbol_string(k))){
     //	  mysql_bind_output(bind[i],real_lengths+i,v);
     //	}
     //});
     //for (int i = 0; i<data.num_fields_; ++i) {
     //  if (!bind[i].buffer_type) {
     //	std::ostringstream ss;ss<<"Error while binding the mysql request to a metamap object: "<<std::endl
     //	  <<"   Field "<<data.fields_[i].name<<" could not be bound."<<std::endl;
     //	throw std::runtime_error(ss.str());
     //  }
     //}
     //return bind_data;
  // }
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
  // Forward reference tuple impl.
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
  /**
   * @brief The prepared statement result interface.
   * @tparam B
   */
  template <typename B> struct mysql_statement_result {
    mysql_statement_result(B& mysql_wrapper_, mysql_statement_data& data_,
      const std::shared_ptr<mysql_connection_data>& connection_)
      : mysql_wrapper_(mysql_wrapper_), data_(data_), connection_(connection_) {}
    mysql_statement_result& operator=(mysql_statement_result&) = delete;
    mysql_statement_result(const mysql_statement_result&) = delete;
    mysql_statement_result(mysql_statement_result&&) = default;

    /**
     * @brief Destructor. Free the result if needed.
     */
    inline ~mysql_statement_result() { flush_results(); }

    inline void flush_results() {
      // if (result_allocated_)
      if (connection_) // connection is null if this has been moved in another instance.
        mysql_wrapper_.mysql_stmt_free_result(connection_->error_, data_.stmt_);
      // result_allocated_ = false;
    }
    template <typename T> bool read(T&& output);

    template <typename T> bool read(T&& output, MYSQL_BIND* bind, unsigned long* real_lengths);

    template <typename F> void map(F map_callback);
    /**
     * @return the number of rows affected by the request.
     */
    long long int affected_rows();
    /**
     * @brief Return the last id generated by a insert comment.
     *
     * @return the last inserted id.
     */
    long long int last_insert_id();
    void next_row();
    // Internal methods.
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
} // namespace crow
namespace crow {
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
    // If the string was big enough to hold the result string, return it.
    if (real_length <= v.size()) {
      v.resize(real_length);
      return;
    }
    // Otherwise we need to call mysql_stmt_fetch_column again to get the result string.
    // Reserve enough space to fetch the string.
    v.resize(real_length);
    // Bind result.
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
    // std::cout << " specialized" << std::endl;
    auto row_object = [] {
      static_assert(std::tuple_size_v < TP >> 0, "sql_result map function must take at least 1 argument.");
      if constexpr (is_tuple<TP0>::value)
        return TP0{};
      else
        return TP{};
    }();
    result_allocated_ = true;
    // Bind output.
    auto bind_data = mysql_bind_output(data_, row_object);
    unsigned long* real_lengths = bind_data.real_lengths.data();
    MYSQL_BIND* bind = bind_data.bind.data();
    bool bind_ret = mysql_stmt_bind_result(data_.stmt_, bind_data.bind.data());
    // std::cout << "bind_ret: " << bind_ret << std::endl;
    if (bind_ret != 0) {
      throw std::runtime_error(std::string("mysql_stmt_bind_result error: ") + mysql_stmt_error(data_.stmt_));
    }


    while (this->read(row_object, bind, real_lengths)) {
      if constexpr (is_tuple<TP0>::value)
        map_callback(row_object);
      else
        std::apply(map_callback, row_object);
      // restore string sizes to 100.
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
    // Bind output.
    auto bind_data = mysql_bind_output(data_, std::forward<T>(output));
    unsigned long* real_lengths = bind_data.real_lengths.data();
    MYSQL_BIND* bind = bind_data.bind.data();

    bool bind_ret = mysql_stmt_bind_result(data_.stmt_, bind_data.bind.data());
    // std::cout << "bind_ret: " << bind_ret << std::endl;
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
      // Fetch row.
      // Note: This also advance to the next row.
      int res = mysql_wrapper_.mysql_stmt_fetch(connection_->error_, data_.stmt_);
      if (res == MYSQL_NO_DATA) // If end of result, return false.
        return false;
      // Finalize fetch:
      //    - fetch strings that did not fit the preallocated strings.
      //    - check for truncated data errors.
      //    - resize preallocated strings that were bigger than the request result.
      // is tuple
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
  /**
   * @brief Mysql prepared statement @tparam B the blocking or non blocking mode.
   */
  template <typename B> struct mysql_statement {
    /**
     * @brief Execute the statement with argument.Number of args must be equal to the number of placeholders in the request.
     * @param args the arguments @return mysql_statement_result<B> the result
     */
    template <typename... T> sql_result<mysql_statement_result<B>> operator()(T&&... args);

    B& mysql_wrapper_;
    mysql_statement_data& data_;
    std::shared_ptr<mysql_connection_data> connection_;
  };
  template <typename B>
  template <typename... T>
  sql_result<mysql_statement_result<B>> mysql_statement<B>::operator()(T&&... args) {
    if constexpr (sizeof...(T) > 0) {
      // if (sizeof...(T) > 0) { Bind the ...args in the MYSQL BIND structure.
      MYSQL_BIND bind[sizeof...(T)];
      memset(bind, 0, sizeof(bind));//memset(bind, 0, sizeof...(T) * sizeof(MYSQL_BIND));
      int i = 0;
      tuple_map(std::forward_as_tuple(args...), [&](auto& m) {
        mysql_bind_param(bind[i], m);
        ++i;
        });
      // Pass MYSQL BIND to mysql.
      if (mysql_stmt_bind_param(data_.stmt_, bind) != 0) {
        connection_->error_ = 1;
        throw std::runtime_error(std::string("mysql_stmt_bind_param error: ") + mysql_stmt_error(data_.stmt_));
      }
    }
    // Execute the statement.
    mysql_wrapper_.mysql_stmt_execute(connection_->error_, data_.stmt_);
    // Return the wrapped mysql result.
    return sql_result<mysql_statement_result<B>>(mysql_statement_result<B>(mysql_wrapper_, data_, connection_));
  }
  /**
   * @brief Store a access to the result of a sql query (non prepared).
   * @tparam B must be mysql_functions_blocking or mysql_functions_non_blocking
   */
  template <typename B> struct mysql_result {
    B& mysql_wrapper_; // blocking or non blockin mysql functions wrapper.
    std::shared_ptr<mysql_connection_data> connection_;
    MYSQL_RES* result_ = nullptr; // Mysql result.
    unsigned long* current_row_lengths_ = nullptr;
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
    /**
     * @return the number of rows affected by the request.
     */
    long long int affected_rows();
    /**
     * @brief Return the last id generated by a insert comment.
     * @return the last inserted id.
     */
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

    //if constexpr (is_tuple<T>::value) { // Tuple
    //}
    if (std::tuple_size_v<std::decay_t<T>> != current_row_num_fields_)
      throw std::runtime_error(std::string("The request number of field (") +
        boost::lexical_cast<std::string>(current_row_num_fields_) +
        ") does not match the size of the tuple (" +
        boost::lexical_cast<std::string>(std::tuple_size_v<std::decay_t<T>>) + ")");
    int i = 0;
    crow::tuple_map(std::forward<T>(output), [&](auto& v) {
      // std::cout << "read " << std::string_view(current_row_[i], current_row_lengths_[i]) << std::endl;
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

  struct mysql_connection_data; struct mysql_tag {};// Forward ref.
  template <typename B> // must be mysql_functions_blocking or mysql_functions_non_blocking
  struct mysql_connection {
    typedef mysql_tag db_tag;
    inline mysql_connection(B mysql_wrapper, std::shared_ptr<crow::mysql_connection_data>& data);
    /** @brief Last inserted row id.@return long long int the row id.*/
    long long int last_insert_rowid();
    /** @brief Execute a SQL request. @param rq the request string @return mysql_result<B> the result.*/
    sql_result<mysql_result<B>> operator()(const std::string& rq);
    /** @brief Build a sql prepared statement. @param rq the request string @return mysql_statement<B> the statement.*/
    mysql_statement<B> query(const std::string& rq);
    /**
     * @brief Build or retrieve a sql statement from the connection cache. Will regenerate the statement if one of the @param keys changed.
     * @param f the function that generate the statement. @param keys the keys. @return mysql_statement<B> the statement.
     */
    template <typename F, typename... K> mysql_statement<B> cached_statement(F f, K... keys);
    template <typename T> std::string type_to_string(const T& t) { return cpptype_to_mysql_type(t); }
    B mysql_wrapper_;
    std::shared_ptr<mysql_connection_data> data_;
  };
} // namespace crow
namespace crow {
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
    }
    else
      return mysql_statement<B>{mysql_wrapper_, * data_->statements_hashmap_(f, keys...),
      data_};
  }
  template <typename B> mysql_statement<B> mysql_connection<B>::query(const std::string& rq) {
    auto it = data_->statements_.find(rq);
    if (it != data_->statements_.end()) {
      // mysql_wrapper_.mysql_stmt_free_result(it->second->stmt_);
      // mysql_wrapper_.mysql_stmt_reset(it->second->stmt_);
      return mysql_statement<B>{mysql_wrapper_, * it->second, data_};
    }
    //std::cout << "query " << rq << "  "  << data_->statements_.size() << std::endl;
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
#define MaxSyncConnections 32
#define EXPECT_THROW(STM)\
try {STM;std::cerr << "This must have thrown an exception: " << #STM << std::endl; } catch (...) {}
#define EXPECT_EQUAL(A, B)\
if(A!=B){std::cerr << #A << " (== " << A << ") " << " != " << #B << " (== " << B << ") " << std::endl;}
#define EXPECT(A)\
  if (!(A)) { std::cerr << #A << " (== " << (A) << ") must be true" << std::endl; }
#define ASSERT(x,m) if(!x)std::cerr<<#m
template <typename... T> std::ostream& operator<<(std::ostream& os, std::tuple<T...> t) {
  os << "TUPLE<"; crow::tuple_map(std::forward<std::tuple<T...>>(t), [&os](auto v) { os << v << " ,"; }); os << ">"; return os;
}
namespace crow {
  struct mysql {
    typedef mysql_tag db_tag;
    typedef mysql_connection_data connection_data_type;
    inline mysql(const char* host, const char* database, const char* user, const char* password, ...);
    inline ~mysql();
    inline mysql_connection_data* new_connection();
    inline int get_socket(const std::shared_ptr<mysql_connection_data>& data);//none
    inline mysql_connection<mysql_functions_blocking> scoped_connection(std::shared_ptr<mysql_connection_data>& data);
    std::string host_, user_, passwd_, database_, character_set_;
    unsigned int port_;
  };
  //typedef sql_database<mysql> Mysql;
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
    //connection = mysql;
    //connection = mysql_real_connect(connection, host_.c_str(), user_.c_str(), passwd_.c_str(), database_.c_str(), port_, NULL, 0);
    //if (!connection) return nullptr;

    mysql_options(mysql, MYSQL_OPT_NONBLOCK, 0);
    connection = nullptr;
    int mysql_fd = -1;
    int status;
    status = mysql_real_connect_start(&connection, mysql, host_.c_str(), user_.c_str(),
      passwd_.c_str(), database_.c_str(), port_, NULL, 0);
    // std::cout << "after: " << mysql_get_socket(mysql) << " " << status == MYSQL_ <<
    // std::endl;
    mysql_fd = mysql_get_socket(mysql);
    if (mysql_fd == -1) {
      // std::cout << "Invalid mysql connection bad mysql_get_socket " << status << " " << mysql
      // << std::endl;
      mysql_close(mysql);
      return nullptr;
    }
    while (status)
      try {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      status = mysql_real_connect_cont(&connection, mysql, status);
    }
    catch (std::runtime_error& e) {
      // Yield thrown a exception (probably because a closed connection).
      // std::cerr << "Warning: yield threw an exception while connecting to mysql: "
      //  << total_number_of_mysql_connections << std::endl;
      mysql_close(mysql);
      throw std::move(e);
    }
    if (!connection) { return nullptr; }// Error in mysql_real_connect_cont

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
    sqlite3* db_;
    sqlite3_stmt* stmt_;
    int last_step_ret_;
    inline void flush_results() { sqlite3_reset(stmt_); }
    // Read a tuple or a metamap.
    template <typename T> bool read(T&& output) {
      // Throw is nothing to read.
      if (last_step_ret_ != SQLITE_ROW)
        return false;
      //if constexpr (is_tuple<T>::value) {}
      int ncols = sqlite3_column_count(stmt_);
      std::size_t tuple_size = std::tuple_size_v<std::decay_t<T>>;
      if (ncols != tuple_size) {
        std::ostringstream ss;
        ss << "Invalid number of parameters: SQL request has " << ncols
          << " fields but the function to process it has " << tuple_size << " parameters.";
        throw std::runtime_error(ss.str());
      }
      int i = 0;
      auto read_elt = [&](auto& v) {
        this->read_column(i, v, sqlite3_column_type(stmt_, i));
        ++i;
      };
      ::crow::tuple_map(std::forward<T>(output), read_elt);
      // Go to the next row.
      last_step_ret_ = sqlite3_step(stmt_);
      if (last_step_ret_ != SQLITE_ROW && last_step_ret_ != SQLITE_DONE)
        throw std::runtime_error(sqlite3_errstr(last_step_ret_));
      return true;
    }
    inline long long int last_insert_id() { return sqlite3_last_insert_rowid(db_); }
    inline void read_column(int pos, int& v, int sqltype) {
      if (sqltype != SQLITE_INTEGER)
        throw std::runtime_error(
          "Type mismatch between request result data type && destination type (integer).");
      v = sqlite3_column_int(stmt_, pos);
    }
    inline void read_column(int pos, float& v, int sqltype) {
      if (sqltype != SQLITE_FLOAT)
        throw std::runtime_error(
          "Type mismatch between request result data type && destination type (float).");
      v = float(sqlite3_column_double(stmt_, pos));
    }
    inline void read_column(int pos, double& v, int sqltype) {
      if (sqltype != SQLITE_FLOAT)
        throw std::runtime_error(
          "Type mismatch between request result data type && destination type (double).");
      v = sqlite3_column_double(stmt_, pos);
    }
    inline void read_column(int pos, int64_t& v, int sqltype) {
      if (sqltype != SQLITE_INTEGER)
        throw std::runtime_error(
          "Type mismatch between request result data type && destination type (int64).");
      v = sqlite3_column_int64(stmt_, pos);
    }
    inline void read_column(int pos, std::string& v, int sqltype) {
      if (sqltype != SQLITE_TEXT && sqltype != SQLITE_BLOB)
        throw std::runtime_error(
          "Type mismatch between request result data type && destination type (std::string).");
      auto str = sqlite3_column_text(stmt_, pos);
      auto n = sqlite3_column_bytes(stmt_, pos);
      v = std::move(std::string((const char*)str, n));
    }
    // Todo: Date types
    // template <typename C, typename D>
    // void read_column(int pos, std::chrono::time_point<C, D>& v)
    // {
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
    // Bind arguments to the request unknowns (marked with ?)
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
      return sqlite3_bind_text(stmt, pos, s, strlen(s), nullptr);
    }
    inline int bind(sqlite3_stmt* stmt, int pos, const std::string& s) const {
      return sqlite3_bind_text(stmt, pos, s.data(), s.size(), nullptr);
    }
    inline int bind(sqlite3_stmt* stmt, int pos, const std::string_view& s) const {
      return sqlite3_bind_text(stmt, pos, s.data(), s.size(), nullptr);
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
      : db_(nullptr), stm_cache_(new stmt_map()), cache_mutex_(new std::mutex()) // FIXME
    {}
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
      // std::cout << req << std::endl;
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
  struct Sqlite {
    inline Sqlite() {}
    Sqlite(const std::string& path, unsigned int synchronous = 2) {
      path_ = path;
      con_.conn(path, SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
      std::ostringstream ss;
      ss << "PRAGMA synchronous=" << synchronous;
      con_(ss.str());
    }
    template <typename Y> inline sqlite_connection conn(Y& y) { return con_; }
    inline sqlite_connection conn() { return con_; }
    inline void flush() { sqlite3_close_v2(con_.db_); }
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
        // Cancel any pending request.
        PGcancel* cancel = PQgetCancel(pgconn_);
        char x[256];
        if (cancel) {
          PQcancel(cancel, x, 256);
          PQfreeCancel(cancel);
        }
      }
    }
    void flush() {
      while (int ret = PQflush(pgconn_))
      {
        if (ret == -1)
        {
          std::cerr << "PQflush error" << std::endl;
        }
        if (ret == 1)
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
    ~pgsql_result() { if (current_result_) PQclear(current_result_); }
    // Read metamap and tuples.
    template <typename T> bool read(T&& t1);
    long long int last_insert_id();
    // Flush all results.
    void flush_results();
    std::shared_ptr<pgsql_connection_data> connection_;
    int last_insert_id_ = -1;
    int row_i_ = 0;
    int current_result_nrows_ = 0;
    PGresult* current_result_ = nullptr;
    std::vector<Oid> curent_result_field_types_;
    std::vector<int> curent_result_field_positions_;

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
      if (PQconsumeInput(connection) == 0)
      {
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
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        catch (std::runtime_error& e) {
          // Free results.
          // Yield thrown a exception (probably because a closed connection).
          // Flush the remaining results.
          while (true)
          {
            if (PQconsumeInput(connection) == 0)
            {
              connection_status = 1;
              break;
            }
            if (!PQisBusy(connection))
            {
              PGresult* res = PQgetResult(connection);
              if (res) PQclear(res);
              else break;
            }
          }
          throw std::move(e);
        }
      }
      else {
        // std::cout << "notbusy" << std::endl;
        PGresult* res = PQgetResult(connection);
        if (PQresultStatus(res) == PGRES_FATAL_ERROR && PQerrorMessage(connection)[0] != 0)
        {
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
        }
        else if (PQresultStatus(res) == PGRES_NONFATAL_ERROR)
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
      while (true)
      {
        if (connection_->error_ == 1) break;
        PGresult* res = pg_wait_for_next_result(connection_->pgconn_, connection_->error_, true);
        if (res)
          PQclear(res);
        else break;
      }
    }
    catch (std::runtime_error& e) {
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
    }
    else if (field_type == INT4OID)
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

  template <typename T> bool pgsql_result::read(T&& output) {

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

      if (curent_result_field_types_.size() == 0) {

        curent_result_field_types_.resize(PQnfields(current_result_));
        for (int field_i = 0; field_i < curent_result_field_types_.size(); field_i++)
          curent_result_field_types_[field_i] = PQftype(current_result_, field_i);
      }
    }

    // Tuples
    if constexpr (is_tuple<T>::value) {
      int field_i = 0;

      int nfields = curent_result_field_types_.size();
      if (nfields != std::tuple_size_v<std::decay_t<T>>)
        throw std::runtime_error("postgresql error: in fetch: Mismatch between the request number of "
          "field and the outputs.");

      tuple_map(std::forward<T>(output), [&](auto& m) {
        fetch_value(m, field_i, curent_result_field_types_[field_i]);
        field_i++;
        });
    }
    else { // Metamaps.
      int i = 0;
      crow::map(std::forward<T>(output), [&](auto k, auto& m) {
        int field_i = curent_result_field_positions_[i];
        fetch_value(m, field_i, curent_result_field_types_[field_i]);
        i++;
        });
    }

    this->row_i_++;

    return true;
  }

  // Get the last id of the row inserted by the last command.
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

    // Bind statement param utils.
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
    // Bind parameter to the prepared statement and execute it.
    // FIXME long long int affected_rows() { return pgsql_stmt_affected_rows(data_.stmt_); }

  };

  // Execute a request with placeholders.
  template <unsigned N>
  void pgsql_statement::bind_param(sql_varchar<N>&& m, const char** values, int* lengths,
    int* binary) {
    // std::cout << "send param varchar " << m << std::endl;
    *values = m.c_str();
    *lengths = m.size();
    *binary = 0;
  }
  template <unsigned N>
  void pgsql_statement::bind_param(const sql_varchar<N>& m, const char** values, int* lengths,
    int* binary) {
    // std::cout << "send param const varchar " << m << std::endl;
    *values = m.c_str();
    *lengths = m.size();
    *binary = 0;
  }
  void pgsql_statement::bind_param(const char* m, const char** values, int* lengths, int* binary) {
    // std::cout << "send param const char*[N] " << m << std::endl;
    *values = m;
    *lengths = strlen(m);
    *binary = 0;
  }

  template <typename T>
  void pgsql_statement::bind_param(const std::vector<T>& m, const char** values, int* lengths,
    int* binary) {
    int i = 0;
    for (int i = 0; i < m.size(); i++)
      bind_param(m[i], values + i, lengths + i, binary + i);
  }

  template <typename T>
  void pgsql_statement::bind_param(const T& m, const char** values, int* lengths, int* binary) {
    if constexpr (std::is_same<std::decay_t<decltype(m)>, std::string>::value ||
      std::is_same<std::decay_t<decltype(m)>, std::string_view>::value) {
      // std::cout << "send param string: " << m << std::endl;
      *values = m.c_str();
      *lengths = m.size();
      *binary = 0;
    }
    else if constexpr (std::is_same<std::remove_reference_t<decltype(m)>, const char*>::value) {
      // std::cout << "send param const char* " << m << std::endl;
      *values = m;
      *lengths = strlen(m);
      *binary = 0;
    }
    else if constexpr (std::is_same<std::decay_t<decltype(m)>, int>::value) {
      *values = (char*)new int(htonl(m));
      *lengths = sizeof(m);
      *binary = 1;
    }
    else if constexpr (std::is_same<std::decay_t<decltype(m)>, long long int>::value) {
      // FIXME send 64bit values.
      // std::cout << "long long int param: " << m << std::endl;
      *values = (char*)new int(htonl(uint32_t(m)));
      *lengths = sizeof(uint32_t);
      // does not work:
      // values = (char*)new uint64_t(htobe64((uint64_t) m));
      // lengths = sizeof(uint64_t);
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

  // Bind parameter to the prepared statement and execute it.
  template <typename... T>
  sql_result<pgsql_result> pgsql_statement::operator()(T&&... args) {

    unsigned int nparams = 0;
    if constexpr (sizeof...(T) > 0)
      nparams = (bind_compute_nparam(std::forward<T>(args)) + ...);

#ifdef _WIN32 // MSVC does not support variable sized arrays.
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

    // Now calling pqflush seems to work aswell...
    // connection_->flush(this->fiber_);

    return sql_result<pgsql_result>{
      pgsql_result{ this->connection_ }};
  }

  // FIXME long long int affected_rows() { return pgsql_stmt_affected_rows(data_.stmt_); }

  struct pgsql_tag {};

  // template <typename Y> void pq_wait(Y& yield, PGconn* con) {
  //   while (PQisBusy(con))
  //     yield();
  // }
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

    // FIXME long long int last_insert_rowid() { return pgsql_insert_id(connection_); }

    // pgsql_statement operator()(const std::string& rq) { return query(rq)(); }

    auto operator()(const std::string& rq) {
      if (!PQsendQueryParams(connection_, rq.c_str(), 0, nullptr, nullptr, nullptr, nullptr, 1))
        throw std::runtime_error(std::string("Postresql error:") + PQerrorMessage(connection_));
      return sql_result<pgsql_result>{
        pgsql_result{ this->data_, data_->error_ }};
    }
    std::string p_(int pos) {
      std::stringstream ss; ss << "$" << pos; return ss.str();
    }
    // PQsendQueryParams
    template <typename F, typename... K> pgsql_statement cached_statement(F f, K... keys) {
      if (data_->statements_hashmap(f, keys...).get() == nullptr) {
        pgsql_statement res = query(f());
        data_->statements_hashmap(f, keys...) = res.data_.shared_from_this();
        return res;
      }
      else
        return pgsql_statement{ data_, *data_->statements_hashmap(f, keys...) };
    }
    //var i=1;"INSERT into users_test(id, name, age) values (?,?,?);".replace(/\?/g,()=>{return "$"+i++;});
    pgsql_statement query(std::string& rq) {
      uint16_t i = rq.size(); while (rq[--i] != 0x28);
      for (uint16_t l = 0; rq[i]; ++i) if (rq[i] == 0x3f) rq.replace(i, 1, p_(++l));
      auto it = stm_cache_.find(rq);
      if (it != stm_cache_.end()) {
        return pgsql_statement{ data_, *it->second };
      }
      std::string stmt_name = boost::lexical_cast<std::string>(stm_cache_.size());

      if (!PQsendPrepare(connection_, stmt_name.c_str(), rq.c_str(), 0, nullptr)) {
        throw std::runtime_error(std::string("PQsendPrepare error") + PQerrorMessage(connection_));
      }

      // flush results.
      while (PGresult* ret = pg_wait_for_next_result(connection_, data_->error_))
        PQclear(ret);

      auto pair = stm_cache_.emplace(rq, std::make_shared<pgsql_statement_data>(stmt_name));
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

      // flush results.
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

    inline pgsql(const char* host, const char* database, const char* user, const char* password, ...) {
      ASSERT(host, "open_pgsql_connection requires the host argument");
      ASSERT(database, "open_pgsql_connection requires the database argument");
      ASSERT(user, "open_pgsql_connection requires the user argument");
      ASSERT(password, "open_pgsql_connection requires the password argument");
      va_list ap; va_start(ap, password); unsigned int i = va_arg(ap, unsigned int); char* c = va_arg(ap, char*); port_ = i < 0xffff ? i : 0xcea; character_set_ = c[0] ? c : "utf8"; va_end(ap); host_ = host, database_ = database, user_ = user, passwd_ = password;
      if (!PQisthreadsafe())
        throw std::runtime_error("LibPQ is not threadsafe.");
    }

    inline int get_socket(const std::shared_ptr<pgsql_connection_data>& data) {
      return PQsocket(data->pgconn_);
    }

    inline pgsql_connection_data* new_connection() {

      PGconn* connection = nullptr;
      int pgsql_fd = -1;
      std::stringstream coninfo;
      coninfo << "postgresql://" << user_ << ":" << passwd_ << "@" << host_ << ":" << port_ << "/"
        << database_;
      // connection = PQconnectdb(coninfo.str().c_str());
      connection = PQconnectStart(coninfo.str().c_str());
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
        // If PQsocket return -1, retry later.
        PQfinish(connection);
        return nullptr;
      }
      try {
        while (status != PGRES_POLLING_FAILED && status != PGRES_POLLING_OK) {
          int new_pgsql_fd = PQsocket(connection);
          if (new_pgsql_fd != pgsql_fd) {
            pgsql_fd = new_pgsql_fd;
          }
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          status = PQconnectPoll(connection);
        }
      }
      catch (std::runtime_error& e) {
        // Yield thrown a exception (probably because a closed connection).
        PQfinish(connection);
        throw std::move(e);
      }
      // std::cout << "CONNECTED " << std::endl;
      if (status != PGRES_POLLING_OK) {
        std::cerr << "Warning: cannot conn to the postgresql server " << host_ << ": "
          << PQerrorMessage(connection) << std::endl;
        PQfinish(connection);
        return nullptr;
      }

      // pgsql_set_character_set(pgsql, character_set_.c_str());
      return new pgsql_connection_data{ connection, pgsql_fd };
    }
    auto scoped_connection(std::shared_ptr<pgsql_connection_data>& data) {
      return pgsql_connection(data);
    }
  };
  template <typename I> struct sql_database {
    I impl;
    typedef typename I::connection_data_type connection_data_type;
    typedef typename I::db_tag db_tag;
    std::deque<connection_data_type*> sync_connections_;// Sync connections pool.
    std::mutex sync_connections_mutex_;// Sync connections mutex.
    int n_sync_connections_ = 0, max_sync_connections_ = 0;
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
      connection_data_type* data = nullptr; _:
    //bool reuse = false;
    if (!sync_connections_.empty()) {
      //auto lock = [this] { return std::lock_guard<std::mutex>(this->sync_connections_mutex_); }();
      std::lock_guard<std::mutex> lock(this->sync_connections_mutex_);
      data = sync_connections_.back();
      sync_connections_.pop_back();
      //reuse = true;
    }
    else {
      if (n_sync_connections_ > max_sync_connections_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        goto _;
      }
      ++n_sync_connections_;
      try { data = impl.new_connection(); }
      catch (std::runtime_error& e) {
        --n_sync_connections_;
        throw std::move(e);
      }
      //if (!data)--n_sync_connections_;
    }
    assert(data); assert(data->error_ == 0);
    auto sptr = std::shared_ptr<connection_data_type>(data, [this](connection_data_type* data) {
      if (!data->error_ && sync_connections_.size() < max_sync_connections_) {
        std::lock_guard<std::mutex> lock(this->sync_connections_mutex_);
        sync_connections_.push_back(data);
      }
      else { --n_sync_connections_; delete data; }
      });
    //if (reuse) fiber.reassign_fd_to_this_fiber(impl.get_socket(sptr));
    return impl.scoped_connection(sptr);
    }
  };
  //typedef sql_database<mysql> D;
  typedef sql_database<mysql> Mysql;
  typedef sql_database<pgsql> Pgsql;
  //#define D_(a, b, c, d,...) crow::D(a,b,c,d,##__VA_ARGS__)
#define D_mysql() crow::Mysql("127.0.0.1","test","root","",3306,"utf8")
#define D_pgsql() crow::Pgsql("127.0.0.1","test","Asciphx","",5432,"utf8")
#define D_sqlite(...) crow::Sqlite(##__VA_ARGS__)
}
