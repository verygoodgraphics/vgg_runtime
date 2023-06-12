PREFIX=out/Linux/Shared/Debug

./bin/gn gen ${PREFIX} --args=" \
is_official_build=false \
is_component_build=true \
# skia_use_freetype=true \
# skia_enable_svg=true \
# skia_use_zlib=true \
# skia_canvaskit_enable_paragraph=true \
# skia_enable_skparagraph=true \
# skia_use_libwebp_encode=true \
# skia_use_icu=true \
# skia_enable_skunicode=true \
# skia_enable_gpu=true \
# \
# skia_use_expat=false \
# skia_use_piex=false \
# skia_use_lua=false \
# skia_use_dng_sdk=false \
# skia_use_fontconfig=false \
# skia_use_libheif=false \
# skia_use_expat=false \
# skia_use_vulkan=false \
# skia_enable_pdf=false \
"

ninja -C ${PREFIX}
