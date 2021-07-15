@echo off
title test conan
echo Here, i only test the connection of all header files, and can't output them correctly¡­
if exist .\msql.hh del .\msql.hh;
for /f "tokens=2 skip=1 delims= " %%i in (.\www\a.hh) do (
	type .\www\sql\%%~nxi >> .\msql.hh
)
pause