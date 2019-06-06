rem Matrix-driven Appveyor CI script for libmypaint
rem Currently only does MSYS2 builds.
rem https://www.appveyor.com/docs/installed-software#mingw-msys-cygwin
rem Needs the following vars:
rem    MSYS2_ARCH:  x86_64 or i686
rem    MSYSTEM:  MINGW64 or MINGW32

rem Set the paths appropriately
PATH C:\msys64\%MSYSTEM%\bin;C:\msys64\usr\bin;%PATH%

rem Upgrade the MSYS2 platform and install dependencies

cd %APPVEYOR_BUILD_FOLDER%
echo %APPVEYOR_BUILD_FOLDER%
pwd

bash -c "./autobuild/msys2/1_install_requirements.sh"
bash -c "./autobuild/msys2/2_build_mlt.sh"
bash -c "./autobuild/msys2/3_build_synfig.sh"
