#!/bin/bash
set -e

IFS= read -r -p "Enter the vcpkg root directory [C:/vcpkg]: " VCPKG_ROOT
VCPKG_ROOT="${VCPKG_ROOT:-C:/vcpkg}"

if [[ ! -f "$VCPKG_ROOT/vcpkg.exe" ]]; then
  printf 'Could not find vcpkg.exe in "%s".\n' "$VCPKG_ROOT" >&2
  exit 1
fi

"$VCPKG_ROOT/vcpkg.exe" install openssl:x64-windows

cmake -S . -B build -G "Visual Studio 17 2022" -A x64 \
  "-DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=x64-windows

cmake --build build --config Release --parallel
