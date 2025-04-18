cmake_minimum_required(VERSION 3.10)
project(MySimulator)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Specify source files
set(SOURCES
    cpp/ramulator2_wrapper.cpp
    cpp/main.cpp
)

#### External libraries ####
include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "Skip updating the external dependencies after populating them for the first time")

message("Configuring yaml-cpp...")
option(YAML_CPP_BUILD_CONTRIB "Enable yaml-cpp contrib in library" OFF)
option(YAML_CPP_BUILD_TOOLS "Enable parse tools" OFF)
option(YAML_BUILD_SHARED_LIBS "Build yaml-cpp as a shared library" OFF)
FetchContent_Declare(
  yaml-cpp                             
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG        yaml-cpp-0.7.0                        
  SOURCE_DIR     ${CMAKE_SOURCE_DIR}/ext/yaml-cpp
)
FetchContent_MakeAvailable(yaml-cpp)
include_directories(${yaml-cpp_SOURCE_DIR}/include)
message("Done configuring yaml-cpp.")

message("Configuring spdlog...")
FetchContent_Declare(
  spdlog                             
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.11.0     
  SOURCE_DIR     ${CMAKE_SOURCE_DIR}/ext/spdlog
)
FetchContent_MakeAvailable(spdlog)
include_directories(${spdlog_SOURCE_DIR}/include)
message("Done configuring spdlog.")

message("Configuring argparse...")
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
    GIT_TAG        v2.9     
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/ext/argparse
)
FetchContent_MakeAvailable(argparse)
include_directories(${argparse_SOURCE_DIR}/include)
message("Done configuring argparse.")
##################################

# Specify include directories for Ramulator 2.0
include_directories(
    ${CMAKE_SOURCE_DIR}/ext/ramulator2/src
)

# Specify your own include directory
include_directories(${CMAKE_SOURCE_DIR}/cpp)

# Create the executable
add_executable(MySimulator ${SOURCES})

# Link the external libraries and the Ramulator library
set(RAMULATOR_LIBRARY ${CMAKE_SOURCE_DIR}/ext/ramulator2/libramulator.so)

add_library(ramulator_wrapper ${SOURCES})
target_link_libraries(ramulator_wrapper PUBLIC ${RAMULATOR_LIBRARY})
install (TARGETS ramulator_wrapper DESTINATION .)

target_link_libraries(MySimulator 
    PRIVATE ${RAMULATOR_LIBRARY}
    PUBLIC yaml-cpp          # Link yaml-cpp
    PUBLIC spdlog::spdlog    # Link spdlog
    PRIVATE argparse          # Link argparse
)