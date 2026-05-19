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
 - Add "struct" objects:
   - Support struct syntax:
      - `mut` vs. regular fields
      - `new` vs. `del` methods
   - Support `this` keyword syntax.
   - Create struct object type:
      - Fixed collection of `str-id` values to properties: `Value` with bit-flags (deep mutability??)
 - Add struct "tables" which track where to access a property in a struct's buffer.
   - Enables monomorphic Inline Cache optimization.
 - Add simple generation for structs... no inheritance.
 - Add more object opcodes e.g `mk_struct <prop-count> <layout-ID>`, `add_field <is-mut> 0` (requires pushed object ref), `get_field <field-ID> <layout-ID>` (requires pushed object ref), `set_field <field-ID> <layout-ID>` (requires pushed object ref)

#### v0.7.x:
 - DEBUG statement, dumping VM state for troubleshooting.
 - Make unified API to register native functions & manipulate VM state.
