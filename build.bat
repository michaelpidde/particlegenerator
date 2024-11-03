@echo off

set BASE=D:\code\particlegenerator
set IMGUI=D:\code\particlegenerator\libs\imgui

rmdir /S /Q %BASE%\build
mkdir %BASE%\build
pushd %BASE%\build

xcopy /Y /D %BASE%\libs\SDL2-2.30.8\lib\x64\SDL2.dll
xcopy /Y /D %BASE%\libs\SDL2_image-2.8.2\lib\x64\SDL2_image.dll
xcopy /Y /D %BASE%\sprite sprite\

REM -std:c++20 Use C++ 2020 standard
REM -MD link with MSVCRT.LIB (Microsoft C and C++ (MSVC) runtime libraries)
REM -W4 Turn on warning level 4
REM -WX Treat warnings as errors
REM -EHa- Turn off C++ exceptions
REM -Zi Output debug info
set CompilerFlags=-nologo -std:c++20 -MD -W4 -WX -EHa- -Zi
set CompilerOutputs=-Fmparticlegenerator.map -Feparticlegenerator.exe

REM del /q *.pdb 2> NUL

set Sources=%BASE%\main.cpp
set Includes=-I %BASE%\libs\SDL2-2.30.8\include -I %BASE%\libs\SDL2_image-2.8.2\include -I %BASE%\libs\imgui
set Libs=-LIBPATH:%BASE%\libs\SDL2-2.30.8\lib\x64 -LIBPATH:%BASE%\libs\SDL2_image-2.8.2\lib\x64 SDL2.lib SDL2main.lib SDL2_image.lib winmm.lib

cl %CompilerFlags% %CompilerOutputs% %Sources% %Includes% ^
-link -subsystem:console -PDB:particlegenerator%random%.pdb %Libs%


popd

echo Done.
