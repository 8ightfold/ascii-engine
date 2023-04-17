:: Main batch file... expects g++ to be MinGW
@echo off
set app_name=consoleapp.exe
set resource_path=include/resources.o

:: Resource file compilation
echo Generating resources...
windres resources/resources.rc %resource_path%

:: Windows api interface
set api_src=include/api/console.cpp include/api/core.cpp include/api/input.cpp include/api/keypress_handler.cpp include/api/timer.cpp
set audio_src=include/audio/core.cpp include/audio/audiochannel.cpp include/audio/audiointerface.cpp
set ui_src=include/ui/strided_memcpy.cpp

set compile_opts= -std=c++20 -O4 -DCOMPILER_DEBUG=0 -Iinclude
set link_opts= -Iinclude -lgdi32 -lkernel32 -lole32 -lxaudio2_8

if exist %app_name% (
    echo Already compiled!
) else (
    echo Compiling...
    :: Main compile command
    echo on
    g++ %compile_opts% %resource_path% -o %app_name% driver.cpp %api_src% %audio_src% %ui_src% %link_opts%
    @echo off
)
:: Wait
PAUSE
