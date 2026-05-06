#include <math.h>
#include "vm.h"



static const OpFunc opcode_handlers[] = {
    fn_nop,
    fn_put_none,
    fn_put_bool,
    fn_load_imm_gid,
    fn_load_local,
    // fn_ref_local,
    fn_store_local,
    fn_put_k,
    fn_dup,
    fn_pop,
    fn_mul,
    fn_div,
    fn_add,
    fn_sub,
    fn_eq,
    fn_ne,
    fn_lt,
    fn_gt,
    fn_jmp,
    fn_jmp_false,
    fn_jmp_if,
    fn_call,
    fn_put_callee,
    fn_ret
};

VMStatus fn_nop(VMState *s, const Instruction *ip, const Value* cvp, Value *stack) {
    ip++;

    TAILCALL
    return vm_dispatch(s, ip, cvp, stack);
}

VMStatus fn_put_none(VMState *s, const Instruction *ip, const Value *cvp, Value *stack) {
    s->sp++;
    stack[s->sp] = make_value_none();
    ip++;

    TAILCALL
    return vm_dispatch(s, ip, cvp, stack);
}

VMStatus fn_put_bool(VMState *s, const Instruction *ip, const Value *cvp, Value *stack) {
    s->sp++;
    stack[s->sp] = make_value_bool(ip->flag & 1);
    ip++;

    TAILCALL
    return vm_dispatch(s, ip, cvp, stack);
}

VMStatus fn_load_imm_gid(VMState *s, const Instruction *ip, const Value *cvp, Value *stack) {
    s->sp++;
    stack[s->sp] = make_value_int(ip->wide); // ? push ID of procedure's chunk
    ip++;

    TAILCALL
    return vm_dispatch(s, ip, cvp, stack);
}

VMStatus fn_load_local(VMState *s, const Instruction *ip, const Value *cvp, Value *stack) {
    s->sp++;
    stack[s->sp] = stack[s->bp + ip->wide];
    ip++;

    TAILCALL
    return vm_dispatch(s, ip, cvp, stack);
}

VMStatus fn_store_local(VMState *s, const Instruction *ip, const Value *cvp, Value *stack) {
    stack[s->bp + ip->wide] = stack[s->sp];
    s->sp--;
    ip++;

    TAILCALL
    return vm_dispatch(s, ip, cvp, stack);
}

VMStatus fn_put_k(VMState *s, const Instruction *ip, const Value *cvp, Value *stack) {
    s->sp++;
    stack[s->sp] = cvp[ip->wide];
    ip++;

    TAILCALL
    return vm_dispatch(s, ip, cvp, stack);
}

VMStatus fn_dup(VMState *s, const Instruction *ip, const Value *cvp, Value *stack) {
    s->sp++;
    stack[s->sp] = cvp[s->sp - 1];
    ip++;

    TAILCALL
    return vm_dispatch(s, ip, cvp, stack);
}

VMStatus fn_pop(VMState *s, const Instruction *ip, const Value *cvp, Value *stack) {
    s->sp -= ip->flag;
    ip++;

    TAILCALL
    return vm_dispatch(s, ip, cvp, stack);
}

VMStatus fn_mul(VMState *s, const Instruction *ip, const Value *cvp, Value *stack) {
    Value *lhs = stack + s->sp - 1;
    const Value *rhs = stack + s->sp;

    if (lhs->tag != rhs->tag) {
        *lhs = make_value_real(NAN);
    } else {
        switch (lhs->tag) {
        case vtag_int:
            *lhs = make_value_int(lhs->data.i * rhs->data.i);
            break;
        case vtag_real:
            *lhs = make_value_real(lhs->data.f * rhs->data.f);
            break;
        default:
            *lhs = make_value_real(NAN);
            break;
        }
    }

    s->sp--;
    ip++;

    TAILCALL
    return vm_dispatch(s, ip, cvp, stack);
}

VMStatus fn_div(VMState *s, const Instruction *ip, const Value *cvp, Value *stack) {
    Value *lhs = stack + s->sp - 1;
    const Value *rhs = stack + s->sp;

    if (lhs->tag != rhs->tag) {
        *lhs = make_value_real(NAN);
    } else {
        switch (lhs->tag) {
        case vtag_int:
            *lhs = (rhs->data.i != 0)
                ? make_value_int(lhs->data.i / rhs->data.i)
                : make_value_real(NAN);
            break;
        case vtag_real:
            *lhs = (rhs->data.f != 0.0f)
                ? make_value_real(lhs->data.f / rhs->data.f)
                : make_value_real(NAN);
            break;
        default:
            *lhs = make_value_real(NAN);
            break;
        }
    }

    s->sp--;
    ip++;

    TAILCALL
    return vm_dispatch(s, ip, cvp, stack);
}

VMStatus fn_add(VMState *s, const Instruction *ip, const Value *cvp, Value *stack) {
    Value *lhs = stack + s->sp - 1;
    const Value *rhs = stack + s->sp;

    if (lhs->tag != rhs->tag) {
        *lhs = make_value_real(NAN);
    } else {
        switch (lhs->tag) {
        case vtag_int:
            *lhs = make_value_int(lhs->data.i + rhs->data.i);
            break;
        case vtag_real:
            *lhs = make_value_real(lhs->data.f + rhs->data.f);
            break;
        default:
            *lhs = make_value_real(NAN);
            break;
        }
    }

    s->sp--;
    ip++;

    TAILCALL
    return vm_dispatch(s, ip, cvp, stack);
}

VMStatus fn_sub(VMState *s, const Instruction *ip, const Value *cvp, Value *stack) {
    Value *lhs = stack + s->sp - 1;
    const Value *rhs = stack + s->sp;

    if (lhs->tag != rhs->tag) {
        *lhs = make_value_real(NAN);
    } else {
        switch (lhs->tag) {
        case vtag_int:
            *lhs = make_value_int(lhs->data.i - rhs->data.i);
            break;
        case vtag_real:
            *lhs = make_value_real(lhs->data.f - rhs->data.f);
            break;
        default:
            *lhs = make_value_real(NAN);
            break;
        }
    }

    s->sp--;
    ip++;

    TAILCALL
    return vm_dispatch(s, ip, cvp, stack);
}

VMStatus fn_eq(VMState *s, const Instruction *ip, const Value *cvp, Value *stack) {
    Value *lhs = stack + s->sp - 1;
    const Value *rhs = stack + s->sp;

    if (lhs->tag != rhs->tag) {
        *lhs = make_value_bool(0);
    } else {
        switch (lhs->tag) {
        case vtag_nil:
            *lhs = make_value_bool(1);
            break;
        case vtag_bool:
            *lhs = make_value_bool(lhs->data.byte == rhs->data.byte);
            break;
        case vtag_int:
            *lhs = make_value_bool(lhs->data.i == rhs->data.i);
            break;
        case vtag_real:
            *lhs = make_value_bool(lhs->data.f == rhs->data.f);
            break;
        default:
            *lhs = make_value_bool(0);
            break;
        }
    }

    s->sp--;
    ip++;

    TAILCALL
    return vm_dispatch(s, ip, cvp, stack);
}

VMStatus fn_ne(VMState *s, const Instruction *ip, const Value *cvp, Value *stack) {
    Value *lhs = stack + s->sp - 1;
    const Value *rhs = stack + s->sp;

    if (lhs->tag != rhs->tag) {
        *lhs = make_value_bool(0);
    } else {
        switch (lhs->tag) {
        case vtag_nil:
            *lhs = make_value_bool(0);
            break;
        case vtag_bool:
            *lhs = make_value_bool(lhs->data.byte != rhs->data.byte);
            break;
        case vtag_int:
            *lhs = make_value_bool(lhs->data.i != rhs->data.i);
            break;
        case vtag_real:
            *lhs = make_value_bool(lhs->data.f != rhs->data.f);
            break;
        default:
            *lhs = make_value_bool(0);
            break;
        }
    }

    s->sp--;
    ip++;

    TAILCALL
    return vm_dispatch(s, ip, cvp, stack);
}

VMStatus fn_lt(VMState *s, const Instruction *ip, const Value *cvp, Value *stack) {
    Value *lhs = stack + s->sp - 1;
    const Value *rhs = stack + s->sp;

    if (lhs->tag != rhs->tag) {
        *lhs = make_value_bool(0);
    } else {
        switch (lhs->tag) {
        case vtag_nil:
            *lhs = make_value_bool(0);
            break;
        case vtag_bool:
            *lhs = make_value_bool(lhs->data.byte < rhs->data.byte);
            break;
        case vtag_int:
            *lhs = make_value_bool(lhs->data.i < rhs->data.i);
            break;
        case vtag_real:
            *lhs = make_value_bool(lhs->data.f < rhs->data.f);
            break;
        default:
            *lhs = make_value_bool(0);
            break;
        }
    }

    s->sp--;
    ip++;

    TAILCALL
    return vm_dispatch(s, ip, cvp, stack);
}

VMStatus fn_gt(VMState *s, const Instruction *ip, const Value *cvp, Value *stack) {
    Value *lhs = stack + s->sp - 1;
    const Value *rhs = stack + s->sp;

    if (lhs->tag != rhs->tag) {
        *lhs = make_value_bool(0);
    } else {
        switch (lhs->tag) {
        case vtag_nil:
            *lhs = make_value_bool(0);
            break;
        case vtag_bool:
            *lhs = make_value_bool(lhs->data.byte > rhs->data.byte);
            break;
        case vtag_int:
            *lhs = make_value_bool(lhs->data.i > rhs->data.i);
            break;
        case vtag_real:
            *lhs = make_value_bool(lhs->data.f > rhs->data.f);
            break;
        default:
            *lhs = make_value_bool(0);
            break;
        }
    }

    s->sp--;
    ip++;

    TAILCALL
    return vm_dispatch(s, ip, cvp, stack);
}

VMStatus fn_jmp(VMState *s, const Instruction *ip, const Value *cvp, Value *stack) {
    ip += ip->wide;

    TAILCALL
    return vm_dispatch(s, ip, cvp, stack);
}

VMStatus fn_jmp_false(VMState *s, const Instruction *ip, const Value *cvp, Value *stack) {
    const Value *temp = stack + s->sp;

    switch (temp->tag) {
    case vtag_nil:
        ip += ip->wide;
        break;
    case vtag_bool:
        ip += (temp->data.byte == 0) ? ip->wide : 1;
        break;
    case vtag_int:
        ip += (temp->data.i == 0) ? ip->wide : 1;
        break;
    case vtag_real:
        ip += (temp->data.f == 0.0f) ? ip->wide : 1;
        break;
    default:
        s->sp--;
        ip++;
        break;
    }

    TAILCALL
    return vm_dispatch(s, ip, cvp, stack);
}

VMStatus fn_jmp_if(VMState *s, const Instruction *ip, const Value *cvp, Value *stack) {
    const Value *temp = stack + s->sp;

    switch (temp->tag) {
    case vtag_nil:
        ip++;
        break;
    case vtag_bool:
        ip += (temp->data.byte != 0) ? ip->wide : 1;
        break;
    case vtag_int:
        ip += (temp->data.i != 0) ? ip->wide : 1;
        break;
    case vtag_real:
        ip += (temp->data.f != 0.0f) ? ip->wide : 1;
        break;
    default:
        s->sp--;
        ip++;
        break;
    }

    TAILCALL
    return vm_dispatch(s, ip, cvp, stack);
}

VMStatus fn_call(VMState *s, const Instruction *ip, const Value *cvp, Value *stack) {
    const int16_t arg_count = ip->wide;

    if (stack[s->sp - arg_count].tag != vtag_int) {
        return vm_status_err_bad_call;
    }
    
    const Chunk *callee_chunk = s->prgm->chunks.data + stack[s->sp - arg_count].data.i;
    const Instruction *caller_ret_ip = ip + 1;
    const Value *caller_cvp = cvp;
    int caller_bp = s->bp;
    int callee_bp = s->sp - arg_count;

    s->bp = callee_bp;

    // ? For speed and simplicity, use the native stack to track VM call recursions...
    s->depth++;
    vm_dispatch(s, callee_chunk->code.data, callee_chunk->constants.data, stack);

    stack[callee_bp] = stack[s->sp];
    s->sp = callee_bp;
    s->bp = caller_bp;
    ip = caller_ret_ip;
    cvp = caller_cvp;

    if (s->depth == 0) {
        return s->status;
    }

    TAILCALL
    return vm_dispatch(s, ip, cvp, stack);
}

VMStatus fn_put_callee(VMState *s, const Instruction *ip, const Value *cvp, Value *stack) {
    s->sp++;
    stack[s->sp] = stack[s->bp]; // ? assume a function ID is always at CALLEE_BP + 0
    ip++;

    TAILCALL
    return vm_dispatch(s, ip, cvp, stack);
}

VMStatus fn_ret(VMState *s, const Instruction *ip, const Value *cvp, Value *stack) {
    s->depth--;

    if (s->depth < 1) {
        if (s->status == vm_status_pending) {
            s->status = vm_status_ok;
        }

        stack[0] = stack[s->sp];
        s->sp = 0;
        s->bp = 0;
    }

    return s->status;
}

VMStatus vm_dispatch(VMState *s, const Instruction *ip, const Value *cvp, Value *stack) {
    TAILCALL
    return opcode_handlers[ip->op](s, ip, cvp, stack);
}



VMState make_vm(const Program *program, int locals_max, uint8_t depth_max) {
    const Chunk *entry_chunk = program->chunks.data + program->entry_id;
    Value *stack_buffer = calloc(locals_max, sizeof(Value));

    return (VMState) {
        .prgm = program,
        .ip = entry_chunk->code.data,
        .cvp = entry_chunk->constants.data,
        .stack = stack_buffer,
        .sp = 0,
        .bp = 0,
        .depth = 1,
        .status = (stack_buffer != NULL) ? vm_status_pending : vm_status_err_abort
    };
}

VMStatus vm_status(const VMState *s) {
    return s->status;
}

Value vm_result(const VMState *s) {
    return s->stack[0];
}

VMStatus vm_run(VMState *s) {
    return vm_dispatch(s, s->ip, s->cvp, s->stack);
}
