#!/bin/bash

if [ "$1" = "clean" ];then
    cd src && ndk-build clean
    exit
fi

(cd src && ndk-build)

ARCHS="
    arm
    arm64
    x86
    x86_64
    riscv64
"
test -d "magisk_module/bin" && rm -rf magisk_module/bin
for ARCH in $ARCHS
do
    mkdir -p "magisk_module/bin/$ARCH"
done

cp README.md magisk_module/
cp LICENSE magisk_module/
cp src/libs/armeabi-v7a/sudo magisk_module/bin/arm
cp src/libs/arm64-v8a/sudo magisk_module/bin/arm64
cp src/libs/x86/sudo magisk_module/bin/x86
cp src/libs/x86_64/sudo magisk_module/bin/x86_64
cp src/libs/riscv64/sudo magisk_module/bin/riscv64

(cd magisk_module && zip -r ../sudo-ndk.zip -x "*/\.*" -x "\.*" -- *)