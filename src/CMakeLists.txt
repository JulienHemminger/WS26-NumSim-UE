cmake_minimum_required(VERSION 3.12)
project(numsim)

# Automatically collect all source and header files recursively
file(GLOB_RECURSE SOURCES 
    CONFIGURE_DEPENDS 
    "*.cpp" 
    "*.h"
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})

find_package(VTK)
message("If VTK was found on the system: VTK_FOUND: ${VTK_FOUND}")
message("The directory of VTK: VTK_DIR: ${VTK_DIR}")
message("The include directory of VTK: VTK_INCLUDE_DIRS: ${VTK_INCLUDE_DIRS}")

if (VTK_FOUND)
  include_directories(${VTK_INCLUDE_DIRS})
  target_link_libraries(${PROJECT_NAME} ${VTK_LIBRARIES})
endif(VTK_FOUND)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/../build)

add_compile_options(-Wall -Wextra)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message("CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")