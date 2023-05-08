#!/usr/bin/env bash

set -e

if [ ! -d "emsdk" ]; then
    git clone https://github.com/emscripten-core/emsdk.git
    ./emsdk/emsdk install latest
    ./emsdk/emsdk activate latest

fi
source "./emsdk/emsdk_env.sh" >/dev/null

export BUILD_DIR="build-emcc"

export CC=emcc
export CXX=em++
export AR=emar
export RANLIB=emranlib
export STRIP=emstrip
unset PKG_CONFIG

export BUILD_SYSTEM="meson"

if [ $BUILD_SYSTEM = "meson" ]; then

    export COMMON_EMSCRIPTEN_OPTIONS="'-fexceptions', '-sEXCEPTION_CATCHING_ALLOWED=[..]'"

    # TODO see if ALLOW_MEMORY_GROWTH is needed, but if we load ttf's and music it likely is and we don't have to debug OOm crahses, that aren't handled by some library, which is a pain in the a**
    export LINK_EMSCRIPTEN_OPTIONS="$COMMON_EMSCRIPTEN_OPTIONS, '-sEXPORT_ALL=1', '-sUSE_GLFW=3', '-sUSE_WEBGPU=1', '-sWASM=1', '-sALLOW_MEMORY_GROWTH=1', '-sNO_EXIT_RUNTIME=0', '-sASSERTIONS=1'"
    export COMPILE_EMSCRIPTEN_OPTIONS="$COMMON_EMSCRIPTEN_OPTIONS"

    cat <<EOF >"./android/crossbuilt-web.ini"
[host_machine]
system = 'emscripten'
cpu_family = 'wasm32'
cpu = 'wasm32'
endian = 'little'

[binaries]
c = '$CC'
cpp = '$CXX'
strip = 'emstrip'
ar      = 'emar'
ranlib  = 'emranlib'

exe_wrapper = 'node'

[properties]
needs_exe_wrapper = true
sys_root = './emsdk/upstream/emscripten/cache/sysroot'

[built-in options]
c_std = 'c11'
cpp_std = 'c++20'
c_args = [$COMPILE_EMSCRIPTEN_OPTIONS]
c_link_args = [$LINK_EMSCRIPTEN_OPTIONS]
cpp_args = [$COMPILE_EMSCRIPTEN_OPTIONS]
cpp_link_args = [$LINK_EMSCRIPTEN_OPTIONS]


EOF

    meson setup "$BUILD_DIR" \
        --cross-file "./android/crossbuilt-web.ini" \
        -Dcpp_args=-DAUDIO_PREFER_MP3 \
        -Ddefault_library=static \
        -Dfreetype2:zlib=disabled  # TODI, since it's statically linked no duplicates are allowed, solve that

    meson compile -C "$BUILD_DIR"

else

    if [ ! -d "vcpkg" ]; then

        git clone https://github.com/Microsoft/vcpkg.git
        ./vcpkg/bootstrap-vcpkg.sh

    fi

    export EMSCRIPTEN_ARGS=("USE_PTHREADS=1" "USE_SDL=2" "USE_SDL_MIXER=2" "USE_VORBIS=1" "USE_LIBPNG" "USE_MPG123=1" "USE_ZLIB=1" "SDL2_MIXER_FORMATS=['wav','flac','mp3']" "USE_SDL_TTF=2")

    export C_FLAGS=""

    for ARG in ${EMSCRIPTEN_ARGS[@]}; do

        export C_FLAGS="$C_FLAGS -s \"$ARG\" "
    done

    export C_FLAGS="$C_FLAGS -pthread"

    emcmake cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake \
        -DVCPKG_MANIFEST_DIR=android \
        "-DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=$PWD/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" \
        "-DVCPKG_CXX_FLAGS=$C_FLAGS" "-DVCPKG_C_FLAGS=$C_FLAGS" \
        "-DCMAKE_CXX_FLAGS=$C_FLAGS" "-DCMAKE_C_FLAGS=$C_FLAGS"

    cmake --build "$BUILD_DIR" -j 1 --config Release

fi
