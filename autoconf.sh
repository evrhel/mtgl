#!/bin/bash

cd mtgl

mkdir include

cd -P include

# khrplatform.h
mkdir KHR
wget -O KHR/khrplatform.h https://registry.khronos.org/EGL/api/KHR/khrplatform.h

# mtgl/src
cd ../src

mkdir GL

# glcorearb.h
wget -O GL/glcorearb.h https://registry.khronos.org/OpenGL/api/GL/glcorearb.h

if [[ "$OS_TYPE" == "win32"* ]]; then
    # windows specific

    # wglext.h
    mkdir GL
    wget -O GL/wglext.h https://registry.khronos.org/OpenGL/api/GL/wglext.h
fi