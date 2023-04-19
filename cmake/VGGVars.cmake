# This file will output some cmake varibles for configure control and global macros for in C/C++ detection.
# Each cmake 
# 
# --------- CMake Variables Inputs ---------
# VGG_VAR_PLATFORM_TARGET
# VGG_VAR_ARCH_TYPE
# VGG_VAR_GL_BACKEND


# Above cmake variables could generate corresponding C/C++ macro.
# --------- C/C++ Macros ---------
# VGG_OS_MACOS
#     VGG_TARGET_IOS_SIM
#     VGG_TARGET_IOS
#     VGG_TARGET_MACOS
# VGG_OS_WIN
#     VGG_TARGET_WIN
# VGG_OS_LINUX
#
# VGG_ARCH_ARM
# VGG_ARCH_X86
# VGG_OPTIMIAZED


list(APPEND VGG_APPLE_TARGET_STR_LIST "iOS" "macOS" "iOS-simulator")
list(APPEND VGG_WIN_TARGET_LIST "Windows-x86_64")
list(APPEND VGG_ANDROID_TARGET_LIST "Android_arm7" "Android_arm8" "Android_x86" "Android_x64")
list(APPEND VGG_LINUX_TARGET_LIST "amd64")
list(APPEND VGG_ARCH_TYPE_LIST "X86" "ARM")
list(APPEND VGG_GL_BACKENDS_LIST "EGL" "SDL")
set(VGG_VAR_PLATFORM_TARGET "macOS" CACHE STRING "" FORCE)

# Variables checking
if(NOT ${VGG_VAR_PLATFORM_TARGET} IN_LIST VGG_APPLE_TARGET_STR_LIST 
				AND NOT ${VGG_VAR_PLATFORM_TARGET} IN_LIST VGG_WIN_TARGET_LIST 
				AND NOT ${VGG_VAR_PLATFORM_TARGET} IN_LIST VGG_ANDROID_TARGET_LIST
				AND NOT ${VGG_VAR_PLATFORM_TARGET} IN_LIST VGG_LINUX_TARGET_LIST)
		message(FATAL_ERROR "Please specifies a platform target by VGG_VAR_PLATFORM_TARGET. ${VGG_VAR_PLATFORM_TARGET}\n Candidate platforms: ${VGG_APPLE_TARGET_STR_LIST} ${VGG_WIN_TARGET_LIST} ${VGG_ANDROID_TARGET_LIST} ${VGG_LINUX_TARGET_LIST}")
endif()

if(NOT ${VGG_VAR_GL_BACKEND} IN_LIST VGG_GL_BACKENDS_LIST)
		message(FATAL_ERROR "Please specifies a gl target by VGG_VAR_GL_BACKEND. ${VGG_VAR_GL_BACKEND}\n Candidate backends: ${VGG_GL_BACKENDS_LIST}")
endif()


if(APPLE)
	set(VGG_VAR_OS_STR "macOS" CACHE STRING "" FORCE)
	add_compile_definitions(VGG_OS_MACOS)
elseif(WIN32)
	set(VGG_VAR_OS_STR "Windows" CACHE STRING "" FORCE)
	add_compile_definitions(VGG_OS_WIN NOMINMAX)
elseif(UNIX)
    # NOTE: When android.toolchain.cmake specified on windows, this branch also reached, A better way for platform detection is necessary.
	set(VGG_VAR_OS_STR "Linux" CACHE STRING "" FORCE)
	add_compile_definitions(VGG_OS_LINUX)
endif()

if(VGG_VAR_ARCH_TYPE STREQUAL "X86")
	add_compile_definitions(VGG_ARCH_X86)
elseif(VGG_VAR_ARCH_TYPE STREQUAL "ARM")
	add_compile_definitions(VGG_ARCH_ARM)
endif()

if(VGG_VAR_GL_BACKEND STREQUAL "EGL")
	if(VGG_VAR_OS_STR STREQUAL "macOS" )	
		message(FATAL_ERROR "macOS does not support EGL")	
	elseif()
		add_compile_definitions(VGG_GL_BACKEND_EGL)
	endif()
endif()


if(VGG_VAR_OS_STR STREQUAL "macOS")
		if(NOT ${VGG_VAR_PLATFORM_TARGET} IN_LIST VGG_APPLE_TARGET_STR_LIST)
				message(FATAL_ERROR "VGG_VAR_PLATFORM_TARGET: ${VGG_VAR_PLATFORM_TARGET} is not compatible with ${VGG_VAR_OS_STR}")
	endif()
	# PLATFORM 是ios.toolchain.cmake 的变量
	if(VGG_VAR_PLATFORM_TARGET STREQUAL "iOS")
		add_compile_definitions(VGG_TARGET_IOS)
		set(PLATFORM "OS64")
		# TODO::检查签名是否存在
elseif(VGG_VAR_PLATFORM_TARGET STREQUAL "iOS-simulator")
		add_compile_definitions(VGG_TARGET_IOS_SIM)
		set(PLATFORM "SIMULATOR64")
elseif(VGG_VAR_PLATFORM_TARGET STREQUAL "macOS")
		add_compile_definitions(VGG_TARGET_MACOS)
		set(PLATFORM "MACOS")
	endif()
elseif(VGG_VAR_OS_STR STREQUAL "Windows")
		if(${VGG_VAR_PLATFORM_TARGET} IN_LIST VGG_WIN_TARGET_LIST)
		# Windows Target
		# message(FATAL_ERROR "VGG_VAR_PLATFORM_TARGET: ${VGG_VAR_PLATFORM_TARGET} is not compatible with ${VGG_VAR_OS_STR}")
		add_compile_definitions(VGG_TARGET_WIN)

elseif(${VGG_VAR_PLATFORM_TARGET} IN_LIST VGG_ANDROID_TARGET_LIST)
		# Android target
		if(${VGG_VAR_PLATFORM_TARGET} MATCHES "Android_arm7")
			add_compile_definitions(VGG_TARGET_ANDROID_ARM7)
			set(ANDROID_ABI "armeabi-v7a with NEON")
	elseif(${VGG_VAR_PLATFORM_TARGET} MATCHES "Android_arm8")
			add_compile_definitions(VGG_TARGET_ANDROID_ARM8)
			set(ANDROID_ABI "arm64-v8a")
	elseif(${VGG_VAR_PLATFORM_TARGET} MATCHES "Android_x86")
			add_compile_definitions(VGG_TARGET_ANDROID_X86)
			set(ANDROID_ABI "x86")
	elseif(${VGG_VAR_PLATFORM_TARGET} MATCHES "Android_x64")
			add_compile_definitions(VGG_TARGET_ANDROID_X64)
			set(ANDROID_ABI "x86_64")
		endif()
	elseif()
			message(FATAL_ERROR "VGG_VAR_PLATFORM_TARGET: ${VGG_VAR_PLATFORM_TARGET} is not compatible with ${VGG_VAR_OS_STR}")
	endif()
elseif(VGG_VAR_OS_STR STREQUAL "Linux")

else()
		message(FATAL_ERROR "Unsupported OS: ${VGG_VAR_OS_STR}")
endif()

# VGG_OPTIMIAZED
if(CMAKE_BUILD_TYPE MATCHES "Debug" OR CMAKE_CONFIGURATION_TYPES MATCHES "Debug")
	add_compile_definitions(VGG_OPTIMIAZED)
elseif(CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo" OR CMAKE_CONFIGURATION_TYPES MATCHES "RelWithDebInfo")
	add_compile_definitions(VGG_OPTIMIAZED)
elseif(CMAKE_BUILD_TYPE MATCHES "Release" OR CMAKE_CONFIGURATION_TYPES MATCHES "Release")
endif()
