# PracticeJam3

Entry for https://itch.io/jam/practice-jam-2025-3

It's written in C because why not

## Conventions

macros: `SCREAMING_SNAKE_CASE_BECAUSE_MACROS_SHOULD_BE_OBVIOUS`

symbols (anything that goes in the linker table): `[namespaceInCamelCase]_[subsystemInCamelCase]_[nameInCamelCase]`

variables:
- global: it's a symbol, see symbols
    - Note: global variables generally belong in the static state struct
- static: treat it as a symbol for now
- argument: `camelCase`
- local: `camelCase`
- member: `camelCase`

Ok so just make everything camel case, unless it's a symbol which uses camel case with underscore namespace separators

## Build

cmake + clang 18 (for desktop) or cmake + emscripten 4.0.6 (for web)
- The build of emscripten that comes with debian (3.something or whatever) is too old for SDL3 and will not work.
- older version of clang probably work but I haven't tried them
