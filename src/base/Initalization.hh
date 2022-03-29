#pragma once
#include <unordered_map>
namespace orm { static tm* RES_NOW; static auto RES_last = std::chrono::steady_clock::now(); tm now() { if (std::chrono::steady_clock::now() - RES_last < std::chrono::seconds(1)) return *RES_NOW; time_t rawtime; std::time(&rawtime);
#if defined(_MSC_VER) || defined(__MINGW32__)
 localtime_s(RES_NOW, &rawtime);
#else
 localtime_r(&rawtime, RES_NOW);
#endif
 RES_last = std::chrono::steady_clock::now(); return *RES_NOW; } //Prepare the global time variable and the type system of PgSQL
 static int InitializationOrm() { time_t rawtime; std::time(&rawtime); RES_NOW = std::localtime(&rawtime); RES_NOW->tm_isdst = 0;
#if _WIN32
 system("chcp 65001");
#else
 std::locale::global(std::locale("en_US.UTF8"));
#endif
 if (pgsqL) { auto DbQuery = D.conn();//auto support tinyint and unsigned fields.
 if (DbQuery("SELECT count(*) FROM pg_type WHERE typname = 'tinyint';").template r__<int>() == 0) { DbQuery("CREATE DOMAIN TINYINT AS int2 CHECK(VALUE > -129 AND VALUE < 128);"); DbQuery("CREATE DOMAIN UNSIGNED_TINYINT AS int2 CHECK(VALUE > -1 AND VALUE < 256);"); DbQuery("CREATE DOMAIN UNSIGNED_SMALLINT AS int4 CHECK(VALUE > -1 AND VALUE < 65536);"); DbQuery("CREATE DOMAIN UNSIGNED_INTEGER AS int8 CHECK(VALUE > -1 AND VALUE < 4294967296);"); DbQuery("CREATE DOMAIN UNSIGNED_BIGINT AS int8 CHECK(VALUE > -1);");// AND VALUE <= 18446744073709551615
 } } return 1; } static std::string Time2Str(const time_t* t) { tm* _v; _v = std::localtime(t); std::ostringstream os; os << std::setfill('0'); std::string s;
#ifdef _WIN32
 os << std::setw(4) << _v->tm_year + 1900;
#else
 int y = _v->tm_year / 100; os << std::setw(2) << 19 + y << std::setw(2) << _v->tm_year - y * 100;
#endif
 os << '-' << std::setw(2) << (_v->tm_mon + 1) << '-' << std::setw(2) << _v->tm_mday << ' ' << std::setw(2) << _v->tm_hour << ':' << std::setw(2) << _v->tm_min << ':' << std::setw(2) << _v->tm_sec; s = os.str(); return s; }}
