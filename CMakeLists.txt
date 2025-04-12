# Cmake is likely a temporary solution - it's kinda just annoying
# Note: this project only officially supports the clang compiler.
# All other compilers are not guaranteed to work in any capacity.
cmake_minimum_required(VERSION 3.28)

project(Celestial
    VERSION "0.0.0.0"
    DESCRIPTION "game"
    HOMEPAGE_URL "idk"
    LANGUAGES C
)

add_executable(Celestial
    src/arena.c
    src/main.c
    src/render.c

    src/arena.h
    src/main.h
    src/render.h
)

set_target_properties(Celestial PROPERTIES C_STANDARD 11)

add_subdirectory(SDL)

target_link_libraries(Celestial SDL3::SDL3)

target_include_directories(Celestial PUBLIC SDL/include)
