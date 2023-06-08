cmake_minimum_required(VERSION 3.23)
project(socpu)

set(CMAKE_CXX_STANDARD 23)
add_compile_options(-stdlib=libc++ -fexperimental-library)
add_link_options(-stdlib=libc++ -fexperimental-library)

include(cmake/CPM.cmake)
CPMAddPackage("gh:TheLartians/Ccache.cmake@1.2.4")
CPMAddPackage("gh:Neargye/magic_enum@0.8.2")

add_subdirectory(extern/DCsim)
add_subdirectory(extern/SOASM)

add_library(soarchv2_soisv1 src/soarchv2/ucode.cpp src/soarchv2/soisv1.cpp)
target_include_directories(soarchv2_soisv1 PUBLIC include extern/include "${magic_enum_SOURCE_DIR}/include")
target_link_libraries(soarchv2_soisv1 libsoasm)

add_executable(socpu main.cpp)
target_link_libraries(socpu libdcsim soarchv2_soisv1)