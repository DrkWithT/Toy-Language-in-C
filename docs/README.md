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
 - Add for-loop variation with `BREAK;` and `CONTINUE;` **OK**
 - Add negative number literals. **OK**
 - Add object display methods. **OK**

#### v0.5.x
 - Add immutable strings as separate, interned values. **OK**
   - Create string type. **OK**
   - Add more library functions:
      - `stoi`, `stof` **OK**

#### v0.6.x:
 - Add "dict objects": **OK**
 - Add `foo["bar"]` syntax for accessing any keys of objects vs. `::`. **OK**
 - Add compiler support for not duplicating string constants. **OK**
 - Add compiler support for dict literals. **OK**
 - Add `make_dict_dud` opcode to VM & generation. **OK**

#### v0.7.x:
 - Make unified API to register native functions & manipulate VM state. **WIP**
 - Support shared object library builds for 

#### v0.8.x:
 - Add support for bytecode serialization / deserialization _with_ required author hash to run properly.
   - Invoke UB if incorrect.
