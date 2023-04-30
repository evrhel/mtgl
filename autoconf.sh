#!/bin/bash

echo $OSTYPE

cd example

mkdir -p dependencies

cd dependencies

mkdir -p include

# ./example/dependencies/include
cd include

# khrplatform.h
mkdir -p KHR
curl https://registry.khronos.org/EGL/api/KHR/khrplatform.h --output KHR/khrplatform.h

# ./example/dependencies
cd ..

# ./example
cd ..

# ./
cd ..

# ./mtgl
cd mtgl

# ./mtgl/src
cd src

mkdir -p GL

# glcorearb.h
curl https://registry.khronos.org/OpenGL/api/GL/glcorearb.h --output GL/glcorearb.h

if [[ "$OSTYPE" == "msys"* ]]; then
    # windows specific

    # wglext.h
    curl https://registry.khronos.org/OpenGL/api/GL/wglext.h --output GL/wglext.h
fi