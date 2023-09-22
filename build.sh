#!/bin/sh

BUILD_PACKAGES="missing"
if [[ -z "${BUILD_TYPE}" ]]; then
  BUILD_TYPE="Release"
elif [[ "$BUILD_TYPE" == "Debug" ]]; then
  # BUILD_PACKAGES="*"
  BUILD_FLAGS="'-fsanitize=address,leak'"
fi

if [[ ! -z "${CLEAN}" ]] || [[ "$BUILD_PACKAGES" == "*" ]]; then
rm -rf build
fi

conan install . \
  --build=${BUILD_PACKAGES} \
  -c tools.build:cflags="[${BUILD_FLAGS}]" \
  -c tools.build:cxxflags="[${BUILD_FLAGS}]" \
  -s build_type=${BUILD_TYPE} \
  -s compiler.cppstd=20 \
  --output-folder=build
cd build
if [[ "$OSTYPE" == "cygwin" || "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
  cmake .. -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
  cmake --build . --config ${BUILD_TYPE} || exit 1
  cd ${BUILD_TYPE}
  if [[ ! -z "${PACKAGE}" ]] && type -P "upx"; then
    upx navigator.exe
  fi
else
  cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
  cmake --build . || exit 1
  if [[ ! -z "${PACKAGE}" ]] && type -P "upx"; then
    upx navigator
  fi
fi
./navigator $1
