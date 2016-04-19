cmake_minimum_required(VERSION 2.8)
PROJECT (xpider_group)

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

# set source list
SET (SRC_LIST "src/xpider_group.cpp"
              "src/cJSON.c"
              "src/spideraction.cpp")

#set compile options
ADD_EXECUTABLE(${PROJECT_NAME} ${SRC_LIST})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} mraa pthread mosquitto mosquittopp xpiderctl)
