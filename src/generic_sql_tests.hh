#pragma once
#include <iostream>
template <typename C> void init_test_table(C&& q) {
  q("DROP table if exists users_test;");
  q("CREATE TABLE users_test (id int,name TEXT,age int);");
}
template <typename R, typename S> void test_result(R&& q, S&& new_query) {
  // std::std::cout << " STRING TO INT " << std::endl;
  // std::std::cout << "q(SELECT 'xxx';).template r__<int>() == " << q("SELECT 'xxx';").template r__<int>() << std::endl;
  // new_query("SELECTT 1+2").template r__<int>();
  // Invalid queries must throw.
  EXPECT_THROW(new_query("SELECTTT 1+2").template r__<int>());
  // //   long long int affected_rows();
  // //   template <typename T> T r__();
  EXPECT_EQUAL(3, q("SELECT 1+2").template r__<int>());
  EXPECT_EQUAL(-1, q("SELECT 1-2").template r__<int>());
  EXPECT_EQUAL(q("SELECT 'xxx'").template r__<std::string>(), "xxx");
  init_test_table(q);
  // Reading empty sets must throw.
  EXPECT_THROW(q("SELECT age from users_test;").template r__<int>());
  // Invalid r__ types must throw.
  EXPECT_THROW(q("SELECT 'xxx';").template r__<int>());
  // Mismatch in number of fields must throw.
  EXPECT_THROW((q("SELECT 'xxx';").template r__<std::string, int>()));

  int i = 0;
  q("SELECT 1+2").r__(i);
  EXPECT_EQUAL(i, 3);

  std::string str;
  q("SELECT 'xxx'").r__(str);
  EXPECT_EQUAL(str, "xxx");

  std::tuple<int, std::string, int> tp;
  q("SELECT 1,'xxx',3 ").r__(tp);
  EXPECT_EQUAL(tp, (std::make_tuple(1, "xxx", 3)));

  int i1 = 0;
  int i2 = 0;
  str = "";
  q("SELECT 1,'xxx',3 ").r__(i1, str, i2);
  EXPECT_EQUAL((std::tie(i1, str, i2)), (std::make_tuple(1, "xxx", 3)));

  //   template <typename T> std::optional<T> read_optional();
  EXPECT_EQUAL(0, q("SELECT count(*) from users_test;").template r__<int>());

  EXPECT(!q("SELECT age from users_test;").template read_optional<int>().has_value());
  EXPECT_EQUAL(3, q("SELECT 1+2").template read_optional<int>().value());

  //   template <typename T> void r__(std::optional<T>& o);
  std::optional<int> opt;
  q("SELECT 1+2").r__(opt);
  EXPECT_EQUAL(opt.value(), 3);

  q("INSERT into users_test(id, name, age) values (1,'a',41);");
  q("INSERT into users_test(id, name, age) values (2,'b',42);");
  q("INSERT into users_test(id, name, age) values (3,'c',43);");
  EXPECT_EQUAL(3, q("SELECT count(*) from users_test").template r__<int>());

  int index = 1;
  q("SELECT id, name, age from users_test;").map([&](int id, std::string name, int age) {
    EXPECT_EQUAL(id, index);
    EXPECT_EQUAL(age, index + 40);
    EXPECT_EQUAL(name.size(), 1);
    EXPECT_EQUAL(name[0], 'a' + index - 1);
    index++;
  });


  // map with growing string
  init_test_table(q);
  q("INSERT into users_test(id, name, age) values (1,'aaaaaaaaaaaaaaaaaaa',41);");
  q("INSERT into users_test(id, name, age) values (2,'bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',42);");
  q("INSERT into users_test(id, name, age) values (3,'cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc',43);");
  int sizes[] = {19, 57, 114};
  index = 0;
  q("SELECT name from users_test order by id;").map([&](std::string name) {
    EXPECT_EQUAL(name.size(), sizes[index]);
    index++;
  });
}
template <typename Q> void test_long_strings_prepared_statement(Q& q) {
  init_test_table(q);
  auto insert_user =
      q.query(std::string("INSERT into users_test(id, name, age) values (?,?,?);"));
  std::string test_str_patern = "lkjlkjdgfad0875g9f658g8w97f32orjw0r89ofuhq07fy0rjgq3478fyqh03g7y0b"
                                "347fyj08yg034f78yj047yh078fy0fyj40";
  std::string test_str = "";
  for (int k = 0; k < 10; k++) {
    test_str += test_str_patern;
    insert_user(k, test_str, 42);
  }
  int i = 1;
  q.query("Select id,name from users_test;")().map([&](int id, std::string name) {
    EXPECT_EQUAL(name.size(), 100 * i);
    i++;
  });
}

template <typename D> void generic_sql_tests(D& database) {
  auto q = database.conn();
  // try {
  //   // database.conn();
  //   auto fun = [&](std::string q) { return database.conn()(q); };
  //   fun("SELECTT 1+2").template r__<int>();
  // } catch(...) {}
  try {
    // q.query("SELECT 2+2")().template r__<int>();
    // q.query()
    // database.conn();
    auto fun = [&](const char*s) { return database.conn().query(s)(); };
    fun("SELECTT 1+2").template r__<int>();
  } catch(...) {}
  // auto fun = [&](std::string s) { return database.conn().query(s)(); };
    // fun("SELECTT 1+2").template r__<int>();
  // q.query("SELECT 1+2")().template r__<int>();
  // q.query("SELECT 2+2")().template r__<int>();
  // test_result([&](std::string s) { return q.query(s)(); }, [&](std::string s) { return database.conn().query(s)(); });
  //return;
  // test non prepared statement result.
  test_result(q, [&](const char* s) { return database.conn()(s); });
  // test prepared statement result.
  test_result([&](const char* s) { return q.query(s)(); }, [&](const char* s) { return database.conn().query(s)(); });
  test_long_strings_prepared_statement(q);
  init_test_table(q);
  // Prepared statement.
  auto insert_user = q.query(std::string("INSERT into users_test(id, name, age) values (?,?,?);"));
  insert_user(1, "John", 42);
  insert_user(2, "Bob", 24);
  EXPECT_EQUAL(
      (std::make_tuple("John", 42)),(q("select name, age from users_test where id = 1").template r__<std::string, int>()));
  EXPECT_EQUAL(
      (std::make_tuple("Bob", 24)),
      (q("select name, age from users_test where id = 2").template r__<std::string, int>()));
}