g++ ./src/main.cc -o ./main -Waddress -Wmultichar -Wmisleading-indentation -Wunused-variable -Wl,-rpath,/usr/local/lib -L/usr/local/lib -l:libboost_system.a -DBOOST_ALL_NO_LIB -DBOOST_CONTEXT_DYN_LINK -I/usr/include -I/home/asciphx/zip/vcpkg/installed/x64-linux/include -I./include -L/home/asciphx/zip/vcpkg/installed/x64-linux/lib -l:libsqlite3.a -l:libmariadb.a -l:libmariadbclient.a -l:libssl.a -l:libcrypto.a -l:libpq.a -l:libpgport.a -l:libpgcommon.a -l:libz.a -g -m64 -Wall -ldl -pthread -static-libgcc -fexec-charset=GBK -std=c++17