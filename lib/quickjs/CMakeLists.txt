cmake_minimum_required(VERSION 3.8)
project(quickjs)
set(QJS_SRCS
  quickjs.c
  libregexp.c
  libunicode.c
  cutils.c
  quickjs-libc.c
  libbf.c
)
add_library(quickjs STATIC ${QJS_SRCS})
target_compile_options(quickjs PRIVATE
  -D_GNU_SOURCE
  -DCONFIG_VERSION="2020-11-08"
  -DCONFIG_BIGNUM
  -DDUMP_LEAKS
)
if (APPLE)
target_compile_options(quickjs PRIVATE
  -Wno-incompatible-pointer-types-discards-qualifiers
)
else()
target_compile_options(quickjs PRIVATE
  -Wno-discarded-qualifiers
)
endif()
