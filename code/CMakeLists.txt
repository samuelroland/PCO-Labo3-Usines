# Thanks to Aubry (in pair with resources.qrc)
cmake_minimum_required(VERSION 3.5)

project(PCO_Labo_3 VERSION 0.1 LANGUAGES CXX)

# General settings
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Qt-related MOC, RCC, and UIC file generation
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Generate CXX settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wconversion -Wsign-conversion -pedantic")

# Configure compiler flags
if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
    add_compile_options(-g3 -fPIE -pie)
elseif (CMAKE_BUILD_TYPE MATCHES Release)
    add_definitions(-DNDEBUG)
    add_compile_options(-Ofast)
endif ()

# Find Qt5. We can assume Qt5 from then on
find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

# Source files
file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/*.cpp ${CMAKE_SOURCE_DIR}/*.h ${CMAKE_SOURCE_DIR}/*.ui ${CMAKE_SOURCE_DIR}/*.qrc)

# Include directories
include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME} ${SOURCES} ${RESOURCE_FILES})
target_link_libraries(${PROJECT_NAME} Qt5::Widgets pcosynchro) 

# Setup GTest via Git clone, setup target tests with pcosynchro, gtest default main, qt widgets
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

option(GTEST_CONTEXT, true)

## TODO: auto include all files under tests/*
## TODO: auto list all sources files except main.cpp (to avoid main() refedifinition)
add_executable(
  tests
  tests/tests.cpp
  display.cpp extractor.cpp factory.cpp mainwindow.cpp seller.cpp utils.cpp wholesale.cpp windowinterface.cpp
)

target_link_libraries(
  tests
  GTest::gtest_main
  Qt5::Widgets
  pcosynchro
)

## #define GTEST_CONTEXT so tests do not use call to the non existant interface
target_compile_definitions(tests PUBLIC -DGTEST_CONTEXT)

include(GoogleTest)
gtest_discover_tests(tests)
