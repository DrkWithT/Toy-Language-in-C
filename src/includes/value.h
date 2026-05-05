#ifndef TOYSCRIPT_VALUE_H
#define TOYSCRIPT_VALUE_H

#include <stdint.h>
#include "basics.h"
#include "vec.h"



typedef enum vm_vtag_t {
    vtag_nil,
    vtag_bool,
    vtag_int,
    vtag_real
} ValTag;

typedef struct vm_value_t {
    union {
        int8_t byte;
        int i;
        float f;
    } data;

    ValTag tag;
} Value;

static inline Value make_value_none() {
    return (Value) {
        .data = {
            .byte = 0
        },
        .tag = vtag_nil
    };
}

static inline Value make_value_bool(int8_t b) {
    return (Value) {
        .data = {
            .byte = b
        },
        .tag = vtag_bool
    };
}

static inline Value make_value_int(int i) {
    return (Value) {
        .data = {
            .i = i
        },
        .tag = vtag_int
    };
}

static inline Value make_value_real(float f) {
    return (Value) {
        .data = {
            .f = f
        },
        .tag = vtag_real
    };
}

void Value_dud(Value* ins);
void Value_new(MAYBE_UNUSED Value *ins);
void Value_copy(MAYBE_UNUSED Value *ins, MAYBE_UNUSED const Value *other);
void Value_move(MAYBE_UNUSED Value *ins, MAYBE_UNUSED Value *other);
void Value_del(MAYBE_UNUSED Value *ins);
void print_value(const Value *v);

STUB_VEC(Value)



#endif