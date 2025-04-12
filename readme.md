# Celestial

Celestial is a game about space exploration and sandbox-style gameplay!

This is the source code. Sorry, this is not open source. How did you get access to this anyway?

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