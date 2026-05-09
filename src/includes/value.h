#ifndef TOYSCRIPT_VALUE_H
#define TOYSCRIPT_VALUE_H

#include <stdint.h>
#include "basics.h"
#include "vec.h"



typedef enum vm_vtag_t : uint8_t {
    vtag_nil,
    vtag_bool,
    vtag_int,
    vtag_real,
    // vtag_strid,     // ? Holds an ID into an interned string pool.
    vtag_obj_id        // ? Holds an ID into an object pool.
} ValTag;

// typedef enum vm_vflag_t : uint8_t {
//     vflag_frozen = 0b00,
//     vflag_writable = 0b01,
//     vflag_configurable = 0b10,
//     vflag_mutable = vflag_writable | vflag_configurable,
// } ValFlag;

typedef struct vm_value_t {
    union {
        int8_t byte;
        int16_t obj_id;
        int i;          // ? This can be a 32-bit signed scalar / ID into a string or object pool.
        float f;
    } data;

    ValTag tag;
    // ValFlag flags;
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

static inline Value make_value_obj(int16_t obj_id) {
    return (Value) {
        .data = {
            .obj_id = obj_id
        },
        .tag = vtag_obj_id
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