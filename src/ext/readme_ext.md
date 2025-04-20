# EXT

This is where all of the external "embeddable" libraries go

## STB

A collection of generally useful libraries

- License: MIT
- URL: https://github.com/nothings/stb
- Files: stb_image.h, stb_truetype.h, stb_rect_pack.h

Other notes:
- stb_image.h was modified to function within this build system before, but the other two were isolated into their own library with different settings. See the file itself for more details.

## ARENA

Bump-allocators with a side of extreme convenience

- License: MIT-like
- URL: https://github.com/tsoding/arena
- Files: arena.h

# MINI_UTF8

A super duper tiny header-only library to decoding UTF8 text

- License: MIT
- URL: https://codeberg.org/gnarz/mini-utf8/src/branch/master
- files: mini_utf8.h

Other notes:
- It has been modified to function within this build system - unlike STB and Arena, mini_utf8 cannot be split for compilation with alternate settings. See the file itself for more details.
