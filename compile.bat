:: Main batch file... expects g++ to be MinGW
@echo off
set app_name=../consoleapp.exe
set parser_name=ui/parser/parser.exe
set resource_path=include/resources.o

:: Resource file compilation
echo Generating resources...
windres resources/resources.rc %resource_path%
cd include

:: Windows api interface
set api_src=api/console.cpp api/core.cpp api/input.cpp api/keypress_handler.cpp api/timer.cpp
set audio_src=audio/core.cpp audio/audiochannel.cpp audio/audiointerface.cpp audio/source_types/audiosource_single.cpp audio/source_types/audiosource_circular.cpp audio/source_types/audiosource_looping.cpp audio/source_types/iaudiosource.cpp
set ui_src=ui/strided_memcpy.cpp

set compile_opts= -std=c++20 -O3 -DCOMPILER_DEBUG=0 -I.
set link_opts= -Iinclude -lgdi32 -lkernel32 -lole32 -lxaudio2_8

if exist %app_name% (
    echo Already compiled!
) else (
    echo Compiling...
    :: Main compile command
    echo on
    g++ %compile_opts% resources.o -o %app_name% ../driver.cpp %api_src% %audio_src% %ui_src% %link_opts%
    @echo off
)
:: Wait
PAUSE
