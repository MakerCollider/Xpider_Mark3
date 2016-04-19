CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (xpiderctl)

######WARINING###############################################
#BIN NAME MUST BE THE SAME AS YOUR build.sh and .bb file!!!!#
#############################################################

# cross compile param
# SET (EDISON_SYSROOT "$ENV{HOME}/bin/edison-sdk-ww25/sysroots/core2-32-poky-linux")
# SET (SYS_INC ${EDISON_SYSROOT}/usr/include)
# SET (SYS_LIB ${EDISON_SYSROOT}/usr/lib)
# INCLUDE_DIRECTORIES(${SYS_INC} ${SYS_INC}/mraa)
# LINK_DIRECTORIES(${EDISON_SYSROOT}/usr/lib ${EDISON_SYSROOT}/lib)

# set project version
SET (XPIDERCTL_VERSION_MAJOR 0)
SET (XPIDERCTL_VERSION_MINOR 1)
SET (XPIDERCTL_VERSION_PATCH 0)
SET (XPIDERCTL_VERSION_STRING ${XPIDERCTL_VERSION_MAJOR}.${XPIDERCTL_VERSION_MINOR}.${XPIDERCTL_VERSION_PATCH})

# set options
OPTION (BUILD_SHARED_LIBS "Build shared libs" ON)

# set source list
SET (SRC_LIST "src/xpiderctl.c")

# set compile param
ADD_LIBRARY (${PROJECT_NAME} ${SRC_LIST})
TARGET_LINK_LIBRARIES (${PROJECT_NAME} mraa)
SET_TARGET_PROPERTIES (${PROJECT_NAME} PROPERTIES VERSION ${XPIDERCTL_VERSION_STRING} SOVERSION ${XPIDERCTL_VERSION_MAJOR})

# set install param
INSTALL (TARGETS ${PROJECT_NAME} DESTINATION lib)
INSTALL (FILES ${PROJECT_SOURCE_DIR}/src/xpiderctl.h DESTINATION include/xpiderctl)
INSTALL (FILES ${PROJECT_SOURCE_DIR}/src/config.h DESTINATION include/xpiderctl)