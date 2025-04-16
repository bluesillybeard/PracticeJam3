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

cmake + clang (for desktop) or cmake + emscripten (for web)
