set -e

EMCCFLAGS="
    -O3 -g2
    -std=c++2b
    -flto
    -fno-exceptions
    --no-entry
    -s INITIAL_MEMORY=128KB
    -s ERROR_ON_UNDEFINED_SYMBOLS=0
    -s SUPPORT_ERRNO=0
    -s SUPPORT_LONGJMP=0
"

emcc sha256.cpp $EMCCFLAGS -o src/assets/sha256-simd.wasm -msimd128
emcc sha256.cpp $EMCCFLAGS -o src/assets/sha256.wasm

# TODO: use emscripten 1.40.1 to generate asm.js
emcc sha256.cpp $EMCCFLAGS -o asmjs-tmp.js -s WASM=0
