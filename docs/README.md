## README

### Brief
A very trivial scripting language implemented in C11. Only for educational purposes.

### Usage - Building
 - Requires CMake 4.2 (but 3.30+ is fine)
 - Make, Ninja, or another build tool
 - Clang is preferred
 - Build: `cmake --fresh -S . -B build && cmake --build build`
 - Clean: `rm -rf ./build/toyscript`

### Basic Features
 - BASIC like but...
    - No line numbers or GOTO.
    - VAR declarations supporting multiple defs per line.
    - Control Flow with COND-CASE-ELSE or WHILE statements.
    - Procedural functions.
    - Fixed arrays.

### Roadmap
 - Make string wrapper, vec, and map utils.
 - Make lexer.
