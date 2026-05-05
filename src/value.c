#include <stdio.h>
#include "value.h"

void Value_dud(Value* ins) {}
void Value_new(MAYBE_UNUSED Value *ins) {}
void Value_copy(MAYBE_UNUSED Value *ins, MAYBE_UNUSED const Value *other) {}
void Value_move(MAYBE_UNUSED Value *ins, MAYBE_UNUSED Value *other) {}
void Value_del(MAYBE_UNUSED Value *ins) {}

void print_value(const Value *v) {
    switch (v->tag) {
        case vtag_nil: printf("nil"); break;
        case vtag_bool: printf("%b", v->data.byte); break;
        case vtag_int: printf("%d", v->data.i); break;
        case vtag_real: printf("%f", v->data.f); break;
        default: printf("(unknown)");
    }
}

IMPL_VEC(Value)
