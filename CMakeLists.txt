# Cmake is likely a temporary solution - it's kinda just annoying
# Note: this project only officially supports the clang compiler.
# All other compilers are not guaranteed to work in any capacity.
cmake_minimum_required(VERSION 3.28)

project(PracticeJam3
    VERSION "0.0.0.0"
    DESCRIPTION "game"
    HOMEPAGE_URL "idk"
    LANGUAGES C
)

add_executable(PracticeJam3
    src/arena.c
    src/main.c
    src/render.c

    src/arena.h
    src/main.h
    src/render.h
)

set_target_properties(PracticeJam3 PROPERTIES C_STANDARD 11)

add_subdirectory(SDL)

target_link_libraries(PracticeJam3 SDL3::SDL3)

target_include_directories(PracticeJam3 PUBLIC SDL/include)

if(EMSCRIPTEN)

target_link_options(PracticeJam3 PRIVATE --embed-file ${CMAKE_CURRENT_SOURCE_DIR}/asset@asset/ )

else()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/asset/emscript.bmp ${CMAKE_CURRENT_BINARY_DIR}/asset/emscript.bmp COPYONLY)

endif()