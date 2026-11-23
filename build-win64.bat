@echo off
setlocal

cd /d "%~dp0" || exit /b 1

set "VCPKG_ROOT="
set /p "VCPKG_ROOT=Enter the vcpkg root directory [C:\vcpkg]: "
if not defined VCPKG_ROOT set "VCPKG_ROOT=C:\vcpkg"

rem Allow paths pasted with surrounding quotes.
set "VCPKG_ROOT=%VCPKG_ROOT:"=%"

if not exist "%VCPKG_ROOT%\vcpkg.exe" (
    >&2 echo Could not find vcpkg.exe in "%VCPKG_ROOT%".
    exit /b 1
)

"%VCPKG_ROOT%\vcpkg.exe" install openssl:x64-windows
if errorlevel 1 exit /b %errorlevel%

cmake -S . -B build -A x64 "-DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows
if errorlevel 1 exit /b %errorlevel%

cmake --build build --config Release --parallel
exit /b %errorlevel%
