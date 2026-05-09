## README

### Brief
A very trivial scripting language implemented in C11. Only for educational purposes.

### Usage - Building
 - Requires CMake 4.2 (but 3.30+ is fine)
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
#### v0.1.0
 - Add logical operator (&&, ||) support. **OK**
 - Add assignment operator.
    - Add variable assignment. **OK**
 - Add loops support. **OK**

#### v0.2.0
 - Add object base. **OK**
 - Add object heap. **OK**
 - Add simple, fixed size list objects. **OK**
 - Add mark & sweep GC. **OK**

#### v0.3.0
 - Add native function library for: **WIP**
   - I/O: print(...args)
   - Time functions: now()
   - Math: pow(), sqrt(), clamp(), floor(), and ceil()

#### v0.4.0
 - Add immutable strings as separate, interned values.
   - Create string type.
   - Create string pool.
   - Add `FUN readln(delim) -> string`.
 - Add simple structs and impls??
   - Add `mut` / not-mut members, using runtime mutability checks.
