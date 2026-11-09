cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(nolockscreen)
include("${VITASDK}/share/vita.cmake" REQUIRED)

option(NOLOCKSCREEN_SELF_DISARM "Disable the taiHEN entry before patching" OFF)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -Wextra -Werror -O3 -nostdlib")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

set(NOLOCKSCREEN_SOURCES
  main.c
  pattern.c
)
set(NOLOCKSCREEN_LIBRARIES
  SceKernelModulemgr_stub
  taihen_stub
)

if(NOLOCKSCREEN_SELF_DISARM)
  list(APPEND NOLOCKSCREEN_SOURCES
    config.c
    config_io.c
    self_disarm.c
  )
  list(APPEND NOLOCKSCREEN_LIBRARIES
    SceIofilemgr_stub
    SceLibKernel_stub
  )
  add_definitions(-DNOLOCKSCREEN_SELF_DISARM=1)
endif()

add_executable(nolockscreen ${NOLOCKSCREEN_SOURCES})
target_link_libraries(nolockscreen ${NOLOCKSCREEN_LIBRARIES})

vita_create_self(nolockscreen.suprx nolockscreen CONFIG exports.yml UNSAFE)
