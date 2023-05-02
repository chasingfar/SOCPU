cmake_minimum_required(VERSION 3.23)
project(socpu)

set(CMAKE_CXX_STANDARD 23)
add_compile_options(-stdlib=libc++ -fexperimental-library)
add_link_options(-stdlib=libc++ -fexperimental-library)

include(cmake/CPM.cmake)
CPMAddPackage("gh:TheLartians/Ccache.cmake@1.2.4")
CPMAddPackage("gh:Neargye/magic_enum@0.8.2")
include_directories("${magic_enum_SOURCE_DIR}/include")
include_directories(extern/include)

#add_subdirectory("${PROJECT_SOURCE_DIR}/../dcsim" "${PROJECT_SOURCE_DIR}/extern/dcsim")
#include_directories("${PROJECT_SOURCE_DIR}/../dcsim/include")
add_subdirectory(extern/DCsim)
add_subdirectory(extern/SOASM)
include_directories(extern/DCsim/include)
include_directories(extern/SOASM/include)

include_directories(include)
add_executable(socpu main.cpp src/soarchv2/ucode.cpp src/soarchv2/soisv1.cpp)
#target_link_libraries(ezcpu dcsim)