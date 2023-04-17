:: Main batch file... expects g++ to be MinGW
@echo off
set app_name=consoleapp.exe
:: Windows api interface
set api_src=include/api/console.cpp include/api/core.cpp include/api/input.cpp include/api/timer.cpp
set ui_src=include/ui/strided_memcpy.cpp

if exist %app_name% (
    echo Already compiled!
) else (
    echo Compiling...
    :: Main compile command
    g++ -std=c++20 -O4 -DCOMPILER_DEBUG=0 -o %app_name% driver.cpp %api_src% %ui_src% -Iinclude -lgdi32
)
:: Wait
PAUSE
