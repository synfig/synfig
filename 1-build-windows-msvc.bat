@echo off

REM Specify your build type here ('Debug' for Debug builds and 'Release' for Release builds)
set BUILD_CONFIG=Release

set "ORIGINAL_DIR=%cd%"

if not defined VCPKG_ROOT (
    echo [ERROR]: VCPKG_ROOT not defined. Define VCPKG_ROOT to the path of cloned vcpkg repository.
    exit /b 1 
)

echo vcpkg found at: %VCPKG_ROOT%

mkdir mbuild
cd mbuild

cmake -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake" ^
      -DVCPKG_OVERLAY_PORTS="../vcpkg-ports" ^
      -DCMAKE_BUILD_TYPE=%BUILD_CONFIG% ^
      -DVCPKG_INSTALLED_DIR="../../synfig_vcpkg_installed" ..

echo Building Synfig Studio using %NUMBER_OF_PROCESSORS% threads...
cmake --build . -j %NUMBER_OF_PROCESSORS% --config %BUILD_CONFIG%

cd /d "%ORIGINAL_DIR%"