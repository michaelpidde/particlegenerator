@echo off

set BASE=D:\code\particlegenerator
set IMGUI=D:\code\particlegenerator\libs\imgui

rmdir /S /Q %BASE%\build
mkdir %BASE%\build
pushd %BASE%\build

xcopy /Y /D %BASE%\libs\SDL\lib\x64\SDL2.dll
xcopy /Y /D %BASE%\libs\SDL_image\lib\x64\SDL2_image.dll
xcopy /Y /D %BASE%\sprite sprite\

REM -std:c++20 Use C++ 2020 standard
REM -MD link with MSVCRT.LIB (Microsoft C and C++ (MSVC) runtime libraries)
REM -W4 Turn on warning level 4
REM -WX Treat warnings as errors
REM -EHa- Turn off C++ exceptions
REM -Zi Output debug info
REM -favor select processor to optimize for
REM -MP[n] use up to 'n' processes for compilation. If you omit the processMax argument, the compiler retrieves the number of effective processors on your computer from the operating system, and creates a process for each processor.
set CompilerFlags=-nologo -std:c++20 -MD -W4 -WX -EHa- -Zi -favor:INTEL64 -MP
set MyFlags=-DFIX_FRAMERATE
set CompilerOutputs=-Fmparticlegenerator.map -Feparticlegenerator.exe

REM del /q *.pdb 2> NUL

set Sources=%BASE%\main.cpp ^
    %BASE%\particles.cpp ^
    %BASE%\libs\imgui\backends\imgui_impl_sdl2.cpp ^
    %BASE%\libs\imgui\backends\imgui_impl_sdlrenderer2.cpp ^
    %BASE%\libs\imgui\imgui.cpp ^
    %BASE%\libs\imgui\imgui_draw.cpp ^
    %BASE%\libs\imgui\imgui_widgets.cpp ^
    %BASE%\libs\imgui\imgui_tables.cpp
set Includes=-I %BASE%\libs\SDL\include -I %BASE%\libs\SDL_image\include -I %BASE%\libs\imgui
set Libs=-LIBPATH:%BASE%\libs\SDL\lib\x64 -LIBPATH:%BASE%\libs\SDL_image\lib\x64 SDL2.lib SDL2main.lib SDL2_image.lib winmm.lib

cl %CompilerFlags% %CompilerOutputs% %Sources% %Includes% %MyFlags% ^
-link -subsystem:console -PDB:particlegenerator%random%.pdb %Libs%


popd

echo Done.
