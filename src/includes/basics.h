#ifndef TOYSCRIPT_BASICS_H
#define TOYSCRIPT_BASICS_H

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#define MAYBE_UNUSED __attribute((unused))

#define FATAL_ABORT(title, file, line, msg)\
fprintf(stderr, "Error at %s:%d\n%s: %s\n", file, line, title, msg);\
exit(1);\

// ! NOTE: Only pass __FILE__ or __LINE__ for file and line respectively when using this macro.

#define DUD_OBJS_N(type, name_of_p, n, file, line)\
name_of_p = calloc((size_t)n, sizeof(type));\
if (name_of_p == NULL) {\
    FATAL_ABORT("Alloc Error", file, line, "Failed to initialize buffer of objects.");\
}\
for (size_t name_of_p##_##i = 0; name_of_p##_##i < n; name_of_p##_##i++) {\
    type##_dud(name_of_p + name_of_p##_##i);\
}\
\

#define ALLOC_TYPE(type) ((type*)malloc(sizeof(type)))

#endif