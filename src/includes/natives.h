#ifndef TOYSCRIPT_NATIVES_H
#define TOYSCRIPT_NATIVES_H



#include <stdio.h>
#include <math.h>
#include "obj_list.h"
#include "vm.h"



static inline void native_print_object(const ObjHeap *heap, int16_t id) {
    ObjPtr target_ref = heap_get(heap, id); // ? This pointer views the ObjBase and "vtable" portion of an object.

    // ? NOTE: This is temporary logic to display lists. Dicts types will be supported later!
    // ! Before downcasts, the tag must be checked for soundness.
    if (target_ref->meta.tag == otag_list) {
        const List *target_list = (const List *)target_ref;

        for (size_t i = 0; i < target_list->data.length; i++) {
            print_value(target_list->data.data + i);
        }
    } else {
        printf("(unknown)");
    }
}

/*
 * Invariants: 
 * 1. Returns NONE in STACK[CALLEE_BP].
 * 2. The convention is followed for the VM:
 *      CALLEE_BP = SP - ARGC
 *      LOCALS[N] = STACK[CALLEE_BP + 1 + N]
 * Stack Layout: of print(1, 2, 3)
 * | Value(Int(3)) | <--- SP <--- CALLEE_BP + 3
 * | Value(Int(2)) |
 * | Value(Int(1)) | <--- LOCAL_1 <--- CALLEE_BP + 1
 * | Value(Fun-ID) | <--- CALLEE_BP = SP - ARGC = SP - 3 <--- PUT "none"
 */
static inline VMStatus native_print(VMState *s, int argc) {
    const int callee_bp = s->sp - argc;

    for (int i = 0; i < argc; i++) {
        const Value *arg_ref = s->stack + callee_bp + 1 + i;

        if (arg_ref->tag == vtag_obj_id) {
            native_print_object(&s->heap, arg_ref->data.obj_id);
        } else {
            print_value(arg_ref);
        }

        printf(" ");
    }

    printf("\n");

    s->stack[callee_bp] = make_value_none();
    s->sp = callee_bp;

    return vm_status_pending;
}

static inline VMStatus native_powf(VMState *s, int argc) {
    const int callee_bp = s->sp - argc;
    const Value a0 = s->stack[callee_bp + 1];
    const Value a1 = s->stack[callee_bp + 2];

    if (a0.tag != vtag_real || a1.tag != vtag_real) {
        s->stack[callee_bp] = make_value_real(NAN);
    } else {
        s->stack[callee_bp] = make_value_real(powf(a0.data.f, a1.data.f));
    }

    s->sp = callee_bp;

    return vm_status_pending;
}

static inline VMStatus native_sqrtf(VMState *s, int argc) {
    const int callee_bp = s->sp - argc;
    const Value a0 = s->stack[callee_bp + 1];

    if (a0.tag != vtag_real) {
        s->stack[callee_bp] = make_value_real(NAN);
    } else {
        s->stack[callee_bp] = make_value_real(sqrtf(a0.data.f));
    }

    s->sp = callee_bp;

    return vm_status_pending;
}

static inline float clamp_f32(float v, float low, float high) {
    if (v < low) {
        return low;
    } else if (v > high) {
        return high;
    } else {
        return v;
    }
}

static inline VMStatus native_clampf(VMState *s, int argc) {
    const int callee_bp = s->sp - argc;
    const Value a0 = s->stack[callee_bp + 1];
    const Value a1 = s->stack[callee_bp + 2];
    const Value a2 = s->stack[callee_bp + 3];

    if (a0.tag != vtag_real || a1.tag != vtag_real || a2.tag != vtag_real) {
        s->stack[callee_bp] = make_value_real(NAN);
    } else {
        s->stack[callee_bp] = make_value_real(clamp_f32(a0.data.f, a1.data.f, a2.data.f));
    }

    s->sp = callee_bp;

    return vm_status_pending;
}

static inline VMStatus native_floorf(VMState *s, int argc) {
    const int callee_bp = s->sp - argc;
    const Value a0 = s->stack[callee_bp + 1];

    if (a0.tag != vtag_real) {
        s->stack[callee_bp] = make_value_real(NAN);
    } else {
        s->stack[callee_bp] = make_value_real(floorf(a0.data.f));
    }

    s->sp = callee_bp;

    return vm_status_pending;
}

static inline VMStatus native_ceilf(VMState *s, int argc) {
    const int callee_bp = s->sp - argc;
    const Value a0 = s->stack[callee_bp + 1];

    if (a0.tag != vtag_real) {
        s->stack[callee_bp] = make_value_real(NAN);
    } else {
        s->stack[callee_bp] = make_value_real(ceilf(a0.data.f));
    }

    s->sp = callee_bp;

    return vm_status_pending;
}

#endif
