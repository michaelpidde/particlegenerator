mkdir libs

mkdir tmp
pushd tmp

curl -L -o SDL.zip "https://github.com/libsdl-org/SDL/releases/download/release-2.30.8/SDL2-devel-2.30.8-VC.zip"
tar -xf SDL.zip
xcopy /Q /E /Y SDL2-2.30.8 ..\libs\SDL\

curl -L -o SDL_image.zip "https://github.com/libsdl-org/SDL_image/releases/download/release-2.8.2/SDL2_image-devel-2.8.2-VC.zip"
tar -xf SDL_image.zip
xcopy /Q /E /Y SDL2_image-2.8.2 ..\libs\SDL_image\

curl -L -o imgui.zip "https://github.com/ocornut/imgui/archive/refs/tags/v1.91.5.zip"
tar -xf imgui.zip
xcopy /Q /E /Y imgui-1.91.5 ..\libs\imgui\

popd
rmdir /S /Q tmp