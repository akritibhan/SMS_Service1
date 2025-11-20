@echo off
REM Build script for SMS Service

echo Building SMS Service


call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"

cl /EHsc /std:c++17 /O2 ^
   /I"include" ^
   /I"third_party/json/include" ^
   /I"third_party/pugixml/src" ^
   /I"third_party/cpp-httplib" ^
   /I"C:\Program Files\MySQL\MySQL Server 8.0\include" ^
   /DCPPHTTPLIB_THREAD_POOL_COUNT=128 ^
   src\main.cpp ^
   src\HttpServer.cpp ^
   src\RegexCache.cpp ^
   src\ConnectionPool.cpp ^
   src\Config.cpp ^
   src\SMSProcessor.cpp ^
   src\XMLParser.cpp ^
   third_party\pugixml\src\pugixml.cpp ^
   /Fe:sms_service.exe ^
   /link ^
   /LIBPATH:"C:\Program Files\MySQL\MySQL Server 8.0\lib" ^
   libmysql.lib ws2_32.lib

if %ERRORLEVEL% EQU 0 (
    echo.
   
    echo Build successful: sms_service.exe
   
) else (
    echo.
 
    echo Build failed!
  
    exit /b 1
)
