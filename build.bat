@echo off

set BASE=D:\code\particlegenerator
set IMGUI=D:\code\particlegenerator\libs\imgui

rmdir /S /Q %BASE%\build
mkdir %BASE%\build
pushd %BASE%\build

xcopy /Y /D %BASE%\libs\SDL2-2.30.8\lib\x64\SDL2.dll

REM -std:c++20 Use C++ 2020 standard
REM -MD link with MSVCRT.LIB (Microsoft C and C++ (MSVC) runtime libraries)
REM -W4 Turn on warning level 4
REM -WX Treat warnings as errors
REM -EHa- Turn off C++ exceptions
REM -Zi Output debug info
set CompilerFlags=-nologo -std:c++20 -MD -W4 -WX -EHa- -Zi
set CompilerOutputs=-Fmparticlegenerator.map -Feparticlegenerator.exe

REM del /q *.pdb 2> NUL

set Sources=%BASE%\main.cpp %IMGUI%\backends\imgui_impl_sdl2.cpp %IMGUI%\backends\imgui_impl_sdlrenderer2.cpp ^
    %IMGUI%\imgui.cpp %IMGUI%\imgui_draw.cpp %IMGUI%\imgui_widgets.cpp %IMGUI%\imgui_tables.cpp
set Includes=-I %BASE%\libs\SDL2-2.30.8\include -I %BASE%\libs\imgui
set Libs=-LIBPATH:%BASE%\libs\SDL2-2.30.8\lib\x64 SDL2.lib SDL2main.lib winmm.lib

cl %CompilerFlags% %CompilerOutputs% %Sources% %Includes% ^
-link -subsystem:console -PDB:particlegenerator%random%.pdb %Libs%


popd

echo Done.
