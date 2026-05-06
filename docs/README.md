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
    - Control Flow with IF/ELSE or WHILE statements.
    - Procedural functions.
    - Fixed arrays.
    - Rest parameters (sugar for array-view arguments)

### Roadmap
#### v0.1.0
 - Add logical operator (&&, ||) support. **WIP**
 - Add (compound) assignment operators.
    - Add value pointer reference semantics (with mutability flag).
    - Add `ref_local` opcode. `get_local` gets and resolves any stack value.
 - Add loops support.

#### v0.2.0
 - Add object base.
 - Add object heap.
 - Add immutable strings as char list objects.
 - Add native function library for console I/O.
 - Add file I/O.

#### v0.3.0
 - Add simple, fixed size list objects.
 - Add mark & sweep GC.
