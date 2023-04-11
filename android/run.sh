#!/usr/bin/env bash

set -e

mkdir -p toolchains

export NDK_VER="25c"

export BASE_PATH="$PWD/toolchains/android-ndk-r$NDK_VER"

if [ ! -d "$BASE_PATH" ]; then

    cd toolchains

    wget -q "https://dl.google.com/android/repository/android-ndk-r$NDK_VER-linux.zip"

    unzip -q "android-ndk-r$NDK_VER-linux.zip"

    cd ..
fi

ARCH_LIST=("arm" "aarch64")
ARM_VERSION_LIST=("armv7a" "aarch64")
ARM_NAME_LIST=("androideabi" "android")
ARM_TARET_ARCH_LIST=("armeabi-v7a" "arm64-v8a")

for INDEX in "${!ARCH_LIST[@]}"; do

    export ARCH=${ARCH_LIST[$INDEX]}
    export ARM_VERSION=${ARM_VERSION_LIST[$INDEX]}
    export ARM_NAME=${ARM_NAME_LIST[$INDEX]}
    export ARM_TARET_ARCH=${ARM_TARET_ARCH_LIST[$INDEX]}
    export ARM_TRIPLE=$ARM_VERSION-linux-$ARM_NAME$SDK_VERSION

    export SDK_VERSION="33"

    export ANDROID_NDK_HOME="$BASE_PATH"
    export ANDROID_NDK="$BASE_PATH"

    export SYM_LINK_PATH=sysroot_sym-$ARM_VERSION

    export HOST_ROOT="$BASE_PATH/toolchains/llvm/prebuilt/linux-x86_64"
    # export SYS_ROOT="${HOST_ROOT}/sysroot"
    export SYS_ROOT="${HOST_ROOT}/$SYM_LINK_PATH"
    export BIN_DIR="$HOST_ROOT/bin"
    export PATH="$BIN_DIR:$PATH"

    LIB_PATH="${SYS_ROOT}/usr/lib/${ARCH}-linux-$ARM_NAME:${SYS_ROOT}/usr/lib/${ARCH}-linux-$ARM_NAME/${SDK_VERSION}"
    INC_PATH="${SYS_ROOT}/usr/include"

    export LIBRARY_PATH="$SYS_ROOT/usr/lib/$ARCH-linux-$ARM_NAME/$SDK_VERSION"

    LAST_DIR=$PWD

    if [ -d "$BASE_PATH/toolchains/llvm/prebuilt/linux-x86_64/$SYM_LINK_PATH/" ]; then

        rm -rf "$BASE_PATH/toolchains/llvm/prebuilt/linux-x86_64/$SYM_LINK_PATH/"
    fi

    mkdir -p "$BASE_PATH/toolchains/llvm/prebuilt/linux-x86_64/$SYM_LINK_PATH/usr/lib"

    cd "$BASE_PATH/toolchains/llvm/prebuilt/linux-x86_64/$SYM_LINK_PATH/usr/"

    ln -s "$BASE_PATH/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/local" "$BASE_PATH/toolchains/llvm/prebuilt/linux-x86_64/$SYM_LINK_PATH/usr/"

    ln -s "$BASE_PATH/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include" "$BASE_PATH/toolchains/llvm/prebuilt/linux-x86_64/$SYM_LINK_PATH/usr/"

    find "$BASE_PATH/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/${ARCH}-linux-$ARM_NAME/" -maxdepth 1 -name "*.so" -exec ln -s "{}" "$BASE_PATH/toolchains/llvm/prebuilt/linux-x86_64/$SYM_LINK_PATH/usr/lib/" \;

    find "$BASE_PATH/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/${ARCH}-linux-$ARM_NAME/" -maxdepth 1 -name "*.a" -exec ln -s "{}" "$BASE_PATH/toolchains/llvm/prebuilt/linux-x86_64/$SYM_LINK_PATH/usr/lib/" \;

    find "$BASE_PATH/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/${ARCH}-linux-$ARM_NAME/$SDK_VERSION/" -maxdepth 1 -name "*.a" -exec ln -s "{}" "$BASE_PATH/toolchains/llvm/prebuilt/linux-x86_64/$SYM_LINK_PATH/usr/lib/" \;

    find "$BASE_PATH/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/${ARCH}-linux-$ARM_NAME/$SDK_VERSION/" -maxdepth 1 -name "*.so" -exec ln -s "{}" "$BASE_PATH/toolchains/llvm/prebuilt/linux-x86_64/$SYM_LINK_PATH/usr/lib/" \;

    find "$BASE_PATH/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/${ARCH}-linux-$ARM_NAME/$SDK_VERSION/" -maxdepth 1 -name "*.o" -exec ln -s "{}" "$BASE_PATH/toolchains/llvm/prebuilt/linux-x86_64/$SYM_LINK_PATH/usr/lib/" \;

    cd "$LAST_DIR"

    export BUILD_DIR="build-$ARM_TARET_ARCH"

    export CC=$ARM_VERSION-linux-$ARM_NAME$SDK_VERSION-clang
    export CPP=$ARM_VERSION-linux-$ARM_NAME$SDK_VERSION-clang++
    export CXX=$CPP
    export LD=llvm-ld
    export AS=llvm-as
    export AR=llvm-ar
    export RANLIB=llvm-ranlib

    cat <<EOF >"./android/crossbuilt-$ARM_TARET_ARCH.ini"
[host_machine]
system = 'android'
cpu_family = '$ARCH'
cpu = '$ARM_VERSION'
endian = 'little'

[constants]
android_ndk = '$BIN_DIR'
toolchain = '$BIN_DIR/$ARM_VERSION-linux-$ARM_NAME$SDK_VERSION'

[binaries]
c = '$ARM_VERSION-linux-$ARM_NAME$SDK_VERSION-clang'
cpp = '$ARM_VERSION-linux-$ARM_NAME$SDK_VERSION-clang++'
ar      = 'llvm-ar'
as      = 'llvm-as'
ranlib  = 'llvm-ranlib'
ld      = 'llvm-link'
strip   = 'llvm-strip'
objcopy = 'llvm-objcop'
pkgconfig = 'false'
llvm-config = 'llvm-config'

[built-in options]
c_std = 'c11'
cpp_std = 'c++20'
c_args = ['--sysroot=$BASE_PATH/toolchains/llvm/prebuilt/linux-x86_64/$SYM_LINK_PATH','-fPIE','-fPIC','--target=$ARM_VERSION-none-linux-$ARM_NAME','-DHAVE_USR_INCLUDE_MALLOC_H','-D_MALLOC_H', '-I$BASE_PATH/sources/android/cpufeatures/']
cpp_args = ['--sysroot=$BASE_PATH/toolchains/llvm/prebuilt/linux-x86_64/$SYM_LINK_PATH','-fPIE','-fPIC','--target=$ARM_VERSION-none-linux-$ARM_NAME']
c_link_args = ['-fPIE']
cpp_link_args = ['-fPIE']
prefix = '$SYS_ROOT'
libdir = '$LIB_PATH'

[properties]
pkg_config_libdir = '$LIB_PATH'
sys_root = '$BASE_PATH/toolchains/llvm/prebuilt/linux-x86_64/$SYM_LINK_PATH'

EOF

    if [ ! -d "$PWD/subprojects/cpu-features" ]; then
        mkdir -p "$PWD/subprojects/cpu-features/src/"
        mkdir -p "$PWD/subprojects/cpu-features/include/"
        ln -s "$BASE_PATH/sources/android/cpufeatures/cpu-features.c" "$PWD/subprojects/cpu-features/src/cpu-features.c"
        ln -s "$BASE_PATH/sources/android/cpufeatures/cpu-features.h" "$PWD/subprojects/cpu-features/include/cpu-features.h"
        cat <<EOF >"$PWD/subprojects/cpu-features/meson.build"
project('cpu-features', 'c')

meson.override_dependency(
    'cpu-features',
    declare_dependency(
        sources: files('src/cpu-features.c'),
        compile_args: [
            '-Wno-declaration-after-statement',
            '-Wno-error',
        ],
        include_directories: include_directories('include'),
    ),
)

EOF
    fi

    export LIBRARY_PATH="$LIBRARY_PATH:usr/lib/$ARCH-linux-$ARM_NAME/$SDK_VERSION:$LIB_PATH"

    meson setup "$BUILD_DIR" \
        "--prefix=$SYS_ROOT" \
        "--includedir=$INC_PATH" \
        "--libdir=usr/lib/$ARCH-linux-$ARM_NAME/$SDK_VERSION" \
        --cross-file "./android/crossbuilt-$ARM_TARET_ARCH.ini" \
        -Dsdl2:use_hidapi=disabled \
        -Dsdl2:test=false

    meson compile -C "$BUILD_DIR"

done
