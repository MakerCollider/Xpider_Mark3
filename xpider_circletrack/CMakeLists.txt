CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(xpider_circletrack)

######WARINING###############################################
#BIN NAME MUST BE THE SAME AS YOUR build.sh and .bb file!!!!#
#############################################################

# cross compile param
# SET (EDISON_SYSROOT "$ENV{HOME}/bin/edison-sdk-ww25/sysroots/core2-32-poky-linux")
# SET (SYS_INC ${EDISON_SYSROOT}/usr/include)
# SET (SYS_LIB ${EDISON_SYSROOT}/usr/lib)
# INCLUDE_DIRECTORIES(${SYS_INC} ${SYS_INC}/mraa)
# LINK_DIRECTORIES(${EDISON_SYSROOT}/usr/lib ${EDISON_SYSROOT}/lib)

# set make options
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# set opencv lib
FIND_PACKAGE(OpenCV REQUIRED)

#set compile options
ADD_EXECUTABLE(${PROJECT_NAME} src/${PROJECT_NAME}.cpp)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${OpenCV_LIBS} mraa pthread xpiderctl)