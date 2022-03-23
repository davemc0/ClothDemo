# ClothDemo Solution

cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

set(EXE_NAME ClothDemo)

project(${EXE_NAME})

set(CMAKE_CXX_STANDARD 11 CACHE STRING "" FORCE)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# Warning level, all warnings as errors, build in parallel, disable security warnings, latest language support
add_compile_options(/W3 /WX /MP /sdl- /std:c++latest)

set(__ARCH "x64")

# GLEW
set(GLEW_USE_STATIC_LIBS TRUE)
# set(GLEW_VERBOSE TRUE)

set(CMAKE_PREFIX_PATH "../Goodies/glew-2.2.0")
set(CMAKE_LIBRARY_PATH "../Goodies/glew-2.2.0/lib/Release/x64")

find_package(GLEW REQUIRED)

add_definitions(-DGLEW_STATIC)
include_directories(${GLEW_INCLUDE_DIRS})
link_libraries(${GLEW_STATIC_LIBRARIES})

# FreeGLUT

set(GLUT_INCLUDE_DIR "../Goodies/freeglut-3.2.2/include")
set(GLUT_LIBRARIES "optimized;C:/Users/davemc/source/repos/Goodies/freeglut-3.2.2/lib/Release/freeglut_static.lib;debug;C:/Users/davemc/source/repos/Goodies/freeglut-3.2.2/lib/Debug/freeglut_staticd.lib")

add_definitions(-DFREEGLUT_STATIC)
include_directories(${GLUT_INCLUDE_DIR})
link_libraries(${GLUT_LIBRARIES})

set(SOURCES
    Cloth.cpp Cloth.h Constraint.h ClothDemo.cpp
)

source_group("src"  FILES ${SOURCES})

add_subdirectory(${PROJECT_ROOT_DIR}/../DMcTools ${CMAKE_CURRENT_BINARY_DIR}/DMcTools)

add_executable(${EXE_NAME} ${SOURCES})

set_target_properties(${EXE_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_ROOT_DIR} )
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXE_NAME})

target_link_libraries(${EXE_NAME} PRIVATE DMcTools)
