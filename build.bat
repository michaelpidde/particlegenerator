@echo off

set BASE=D:\code\particlegenerator
set IMGUI=D:\code\particlegenerator\libs\imgui
set CPP_COMMON=D:\code\cpp_common

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
REM -wd Disable warning number
REM     C4530: C++ exception handler used, but unwind semantics are not enabled.
REM     C4189: local variable is initialized but not referenced
set CompilerFlags=-nologo -std:c++20 -MD -W4 -WX -EHa- -Zi -favor:INTEL64 -MP -wd4530 -wd4189
set MyFlags=-DFIX_FRAMERATE

REM del /q *.pdb 2> NUL



REM Compile shared DLL
set SourcesDll=%BASE%\particlegenerator.cpp ^
    %BASE%\particles.cpp
set CompilerOutputsDll=-Fmparticlegeneratordll.map -Feparticlegenerator.dll
set IncludesDll=-I %BASE%\libs\SDL\include -I %BASE%\libs\SDL_image\include -I %CPP_COMMON%
set LibsDll=-LIBPATH:%BASE%\libs\SDL\lib\x64 -LIBPATH:%BASE%\libs\SDL_image\lib\x64 SDL2.lib SDL2main.lib SDL2_image.lib

cl -DAPI_EXPORT %CompilerFlags% %CompilerOutputsDll% %SourcesDll% %IncludesDll% ^
-LD ^
-link -opt:ref -PDB:particlegeneratordll%random%.pdb %LibsDll%



REM Compile generator executable
set Sources=%BASE%\main.cpp ^
    %BASE%\libs\imgui\backends\imgui_impl_sdl2.cpp ^
    %BASE%\libs\imgui\backends\imgui_impl_sdlrenderer2.cpp ^
    %BASE%\libs\imgui\imgui.cpp ^
    %BASE%\libs\imgui\imgui_draw.cpp ^
    %BASE%\libs\imgui\imgui_widgets.cpp ^
    %BASE%\libs\imgui\imgui_tables.cpp
set CompilerOutputsProgram=-Fmparticlegenerator.map -Feparticlegenerator.exe
set Includes=-I %BASE%\libs\SDL\include -I %BASE%\libs\SDL_image\include -I %BASE%\libs\imgui -I %CPP_COMMON%
set Libs=-LIBPATH:%BASE%\libs\SDL\lib\x64 -LIBPATH:%BASE%\libs\SDL_image\lib\x64 SDL2.lib SDL2main.lib SDL2_image.lib winmm.lib particlegenerator.lib

cl %CompilerFlags% %CompilerOutputsProgram% %Sources% %Includes% %MyFlags% ^
-link -subsystem:console -opt:ref -PDB:particlegenerator%random%.pdb %Libs%


popd

echo Done.
