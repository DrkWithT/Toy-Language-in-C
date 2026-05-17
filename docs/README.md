## README

### Brief
A very trivial scripting language implemented in C11. Only for educational purposes.

### Usage - Building
 - Requires CMake 4.2 (but 3.16+ is fine)
 - Make, Ninja, or another build tool
 - Clang is preferred
 - Usage: `./project.sh help`

### Basic Features
 - BASIC like but...
    - No line numbers or GOTO.
    - VAR declarations supporting multiple defs per line.
    - Control Flow with IF/ELSE or WHILE statements.
    - Procedural functions.
    - Fixed arrays.
    - Rest parameters (sugar for array-view arguments)

### Roadmap
#### v0.1.x
 - Add logical operator (&&, ||) support. **OK**
 - Add assignment operator.
    - Add variable assignment. **OK**
 - Add loops support. **OK**

#### v0.2.x
 - Add object base. **OK**
 - Add object heap. **OK**
 - Add simple, fixed size list objects. **OK**
 - Add mark & sweep GC. **OK**

#### v0.3.x
 - Add native function library for: **OK**
   - I/O: print(...args)
   - Math: powf(), sqrtf(), clamp(), floorf(), and ceilf()

#### v0.4.x
 - Add negative number literals. **TODO**
 - Add for-loop variation with `BREAK;` and `CONTINUE;` **WIP**
 - DEBUG statement

#### v0.5.x
 - Add immutable strings as separate, interned values. **WIP**
   - Create string type.
   - Create string pool.
   - Add `FUN readln(fd, delim) -> string`.
   - Add `FUN fopen(fpath), fclose(fd), fgetc(fd)`
