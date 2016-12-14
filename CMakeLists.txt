CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(Projekat)

LINK_DIRECTORIES(
	"/home/student/pputvios1/vezbe/TDP_API/tdp_api"
	"$ENV{SDK_ROOTFS}/home/galois/lib/"
	"$ENV{SDK_ROOTFS}/home/galois/lib/directfb-1.4-6-libs"
)
	
	
FILE(GLOB_RECURSE Projekat_SRC_FILES "${PROJECT_SOURCE_DIR}/src/*.c")

ADD_EXECUTABLE("${PROJECT_NAME}" "${Projekat_SRC_FILES}")
TARGET_INCLUDE_DIRECTORIES("${PROJECT_NAME}" PUBLIC
		"/home/student/pputvios1/vezbe/TDP_API/tdp_api"
		"$ENV{SDK_ROOTFS}/usr/include/"
        "$ENV{SDK_ROOTFS}/usr/include/directfb/"
		"$ENV{SDK_GALOIS}/Common/include/"
		"$ENV{SDK_GALOIS}/OSAL/include/"
		"$ENV{SDK_GALOIS}/OSAL/include/CPU1/"
		"$ENV{SDK_GALOIS}/PE/Common/include/"
		"${PROJECT_SOURCE_DIR}/include")
TARGET_LINK_LIBRARIES("${PROJECT_NAME}" PUBLIC
    "directfb"
    "direct"
    "fusion"
    "rt"
	"tdp"
	"OSAL"
	"shm"
	"PEAgent")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} $ENV{CFLAGS} -std=c99 --sysroot=$ENV{SDK_ROOTFS} -D__LINUX__ -O0 -Wno-psabi")
