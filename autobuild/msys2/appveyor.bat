rem Matrix-driven Appveyor CI script for synfig
rem Currently only does MSYS2 builds.
rem https://www.appveyor.com/docs/installed-software#mingw-msys-cygwin
rem Needs the following vars:
rem    MSYSTEM: MINGW64 or MINGW32

rem Set the paths appropriately
PATH C:\msys64\%MSYSTEM%\bin;C:\msys64\usr\bin;%PATH%

cd %APPVEYOR_BUILD_FOLDER%
echo %APPVEYOR_BUILD_FOLDER%
pwd

bash -c "./1-setup-windows-msys2.sh"
rem bash -c "./2-build-production.sh"
bash -c "mkdir build && cd build && cmake -GNinja .. -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_C_COMPILER_LAUNCHER=ccache && ninja"
