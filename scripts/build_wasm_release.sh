EM_VERSION="3.1.17"
source ~/Code/emsdk/emsdk_env.sh

./bin/gn gen ./out/Build-wasm-Release/Release --args="cc=\"${EMSDK}/upstream/emscripten/emcc\" extra_cflags_cc=[\"-frtti\",\"-s\"] cxx=\"${EMSDK}/upstream/emscripten/em++\" extra_cflags=[\"-Wno-unknown-warning-option\",\"-s\",\"-s\"] \
is_debug=false \
is_official_build=true \
is_component_build=false \
target_cpu=\"wasm\" \
\
skia_use_egl=true \
skia_use_freetype=true \
skia_use_zlib=true \
skia_canvaskit_enable_paragraph=true \
skia_enable_skparagraph=true \
skia_use_libwebp_encode=true \
skia_use_icu=true \
skia_enable_skunicode=true \
skia_enable_gpu=true \
\
skia_use_vulkan=false \
skia_use_expat=false \
skia_use_piex=false \
skia_use_lua=false \
skia_use_dng_sdk=false \
skia_use_fontconfig=false \
skia_use_libheif=false \
skia_use_expat=false \
skia_use_vulkan=false \
skia_enable_pdf=false"

ninja -k 0 -C out/Build-wasm-Release/Release
