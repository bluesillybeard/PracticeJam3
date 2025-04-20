# Cmake is likely a temporary solution - it's kinda just annoying
# Note: this project only officially supports the clang compiler.
# All other compilers are not guaranteed to work in any capacity.
cmake_minimum_required(VERSION 3.28)

set(JAM_EXTRA_COMPILE_OPTIONS "" CACHE STRING "Extra options for the compiler")
set(JAM_EXTRA_LINK_OPTIONS "" CACHE STRING "Extra options for the linker")

project(PracticeJam3
    VERSION "0.0.0.0"
    DESCRIPTION "game"
    HOMEPAGE_URL "idk"
    LANGUAGES C
)

add_executable(PracticeJam3
    src/ext/arena.c
    src/ext/stb_image.c
    src/game.c
    src/main.c
    src/render.c

    src/ext/arena.h
    src/ext/stb_image.h
    src/game.h
    src/main.h
    src/render.h
)

set_target_properties(PracticeJam3 PROPERTIES C_STANDARD 11)

target_compile_options(PracticeJam3 PRIVATE ${JAM_EXTRA_COMPILE_OPTIONS})
target_link_options(PracticeJam3 PRIVATE ${JAM_EXTRA_LINK_OPTIONS})

# Stupid STB not following strict compiler errors. I went through and "fixed" stb_image manually,
# But at some point I have to cut my losses and just dump them into a separate component with loose settings.
add_library(LooseyGooseyFiles
    src/ext/stb_rect_pack.c
    src/ext/stb_truetype.c

    src/ext/stb_rect_pack.h
    src/ext/stb_truetype.h
)

set_target_properties(LooseyGooseyFiles PROPERTIES C_STANDARD 11)

target_link_libraries(PracticeJam3 PRIVATE LooseyGooseyFiles)

add_subdirectory(SDL)

target_link_libraries(PracticeJam3 PUBLIC SDL3::SDL3)

target_include_directories(PracticeJam3 PUBLIC SDL/include)

target_link_libraries(PracticeJam3 PUBLIC cglm)

add_subdirectory(cglm)

if(EMSCRIPTEN)

target_link_options(PracticeJam3 PRIVATE --embed-file ${CMAKE_CURRENT_SOURCE_DIR}/asset@asset/ )

else()

# All the errors and warnings, we want them all!
target_compile_options(PracticeJam3 PRIVATE "-Werror" "-pedantic-errors" "-Wall" "-Wextra" "-Wconversion" "-Wno-unused-function")

endif()