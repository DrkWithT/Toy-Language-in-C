#include <stdint.h>
#include <stdio.h>

#include "compiler.h"



// ! This is UNCHECKED, assuming that the charspan is for a valid integer lexeme after tokenization.
int charspan_atoi(const charspan *s) {
    int result = 0;
    int base = 1;

    if (s->length > 8) {
        return 0;
    }

    for (int i = s->length - 1; i >= 0; i--, base *= 10) {
        const int digit = s->data[i] - '0';

        result += digit * base;
    }

    return result;
}

SymbolInfo make_symbol_info(charspan name_v, int16_t id, Domain d) {
    return (SymbolInfo) {
        .name = name_v,
        .id = id,
        .domain = d
    };
}

SymbolTable make_symbol_table() {
    SymbolInfo *temp_infos = calloc(DEFAULT_SYMBOL_CAPACITY, sizeof(SymbolInfo));

    if (temp_infos != NULL) {   
        return (SymbolTable) {
            .infos = temp_infos,
            .length = 0,
            .capacity = DEFAULT_SYMBOL_CAPACITY,
            .next_local_id = 1,     // ? Start from BP + 1 since BP holds the callee.
            .next_global_id = 1     // ? Start from 1 since chunks 1+ are for other procedures.
        };
    }

    return (SymbolTable) {
        .infos = NULL,
        .length = 0,
        .capacity = 0,
        .next_local_id = 0,
        .next_global_id = 0
    };
}

void symbol_table_del(SymbolTable *self) {
    if (self->infos != NULL) {
        free(self->infos);
        self->infos = NULL;
    }
}

void symbol_table_clear(SymbolTable *self) {
    const int entry_n = self->length;

    for (int i = 0; i < entry_n; i++) {
        self->infos[i] = (SymbolInfo) {
            .name = {
                .data = NULL,
                .length = 0
            },
            .id = 0,
            .domain = symbol_constant
        };
    }
}

const SymbolInfo *symbol_table_find(const SymbolTable *symbols, const charspan *s) {
    const SymbolInfo *infos_begin = symbols->infos;
    const int entry_n = symbols->length;

    for (int entry_pos = 0; entry_pos < entry_n; entry_pos++) {
        if (charspan_equals_charspan(s, &infos_begin[entry_pos].name)) {
            return infos_begin + entry_pos;
        }
    }

    return NULL;
}

const SymbolInfo *symbol_table_push(SymbolTable *symbols, const SymbolInfo *info) {
    const int next_pos = symbols->length;
    const int old_capacity = symbols->capacity;

    if (next_pos >= old_capacity) {
        const int new_capacity = (old_capacity * 3) / 2;
        SymbolInfo *temp_data = realloc(symbols->infos, sizeof(SymbolInfo) * new_capacity);

        if (temp_data != NULL) {   
            symbols->infos = temp_data;
            symbols->capacity = new_capacity;
        } else {
            return NULL;
        }
    }

    symbols->infos[next_pos] = *info;
    symbols->length++;

    return symbols->infos + next_pos;
}



Compiler make_compiler() {
    return (Compiler) {
        .globals = make_symbol_table(),
        .locals = make_symbol_table(),
        .curr = (Token) {
            .begin = 0,
            .length = 0,
            .line = 0,
            .tag = tk_unknown
        },
        .prev = (Token) {
            .begin = 0,
            .length = 0,
            .line = 0,
            .tag = tk_unknown
        },
        .errors = 0,
        .chunk_idx = 0,
        .saved_local_id = 0,
        .flags = cgen_no_flags
    };
}

void compiler_del(Compiler *self) {
    symbol_table_del(&self->globals);
    symbol_table_del(&self->locals);
}

int8_t compiler_match_curr(const Compiler *self, TkTag tag) {
    return self->curr.tag == tag;
}

int8_t compiler_match_prev(const Compiler *self, TkTag tag) {
    return self->prev.tag == tag;
}

Token compiler_advance_tk(Compiler *self, Lexer *lexer, const charspan *s) {
    Token temp;

    do {
        temp = lexer_next(lexer, s);

        switch (temp.tag) {
            case tk_spaces: case tk_comment: continue;
            default: break;
        }

        break;
    } while (1);

    return temp;
}

void compiler_eat_tk(Compiler *self, Lexer *lexer, const charspan *s) {
    self->prev = self->curr;
    self->curr = compiler_advance_tk(self, lexer, s);
}

void compiler_warn(Compiler *self, const char *msg, const Token *tk, const charspan *s) {
    self->errors++;
    fprintf(stderr, "Compile Err #%d at line %d:\n\tnote: %s\n", self->errors, tk->line, msg);
}

void compiler_flag_on(Compiler *self, CodegenFlag flag) {
    self->flags |= flag;
}

void compiler_flag_off(Compiler *self, CodegenFlag flag) {
    self->flags &= ~(uint8_t)flag;
}

int8_t compiler_flag_of(const Compiler *self, CodegenFlag flag) {
    switch (flag) {
    case cgen_assign_to: return (self->flags & flag);
    case cgen_access_of: return (self->flags & flag) >> 1;
    case cgen_lhs_local: return (self->flags & flag) >> 2;
    default: return 0;
    }
}

size_t compiler_emit_op(Compiler *self, Program *pg, Opcode op) {
    AnyVec_Instruction *code_ref = &AnyVec_Chunk_getm(&pg->chunks, self->chunk_idx)->code;
    Instruction temp = {
        .op = op,
        .flag = 0,
        .wide = 0
    };

    AnyVec_Instruction_push(code_ref, &temp);

    return AnyVec_Instruction_len(code_ref);
}

size_t compiler_emit_op_unflagged(Compiler *self, Program *pg, Opcode op, int16_t wide) {
    AnyVec_Instruction *code_ref = &AnyVec_Chunk_getm(&pg->chunks, self->chunk_idx)->code;
    Instruction temp = {
        .op = op,
        .flag = 0,
        .wide = wide
    };

    AnyVec_Instruction_push(code_ref, &temp);

    return AnyVec_Instruction_len(code_ref);
}

size_t compiler_emit_op_flagged(Compiler *self, Program *pg, Opcode op, uint8_t flags, int16_t wide) {
    AnyVec_Instruction *code_ref = &AnyVec_Chunk_getm(&pg->chunks, self->chunk_idx)->code;
    Instruction temp = {
        .op = op,
        .flag = flags,
        .wide = wide
    };

    AnyVec_Instruction_push(code_ref, &temp);

    return AnyVec_Instruction_len(code_ref);
}



const SymbolInfo *compiler_resolve_name(const Compiler *self, const charspan *s) {
    const SymbolInfo *temp = symbol_table_find(&self->globals, s);

    if (temp != NULL) {
        return temp;
    }

    return symbol_table_find(&self->locals, s);
}

const SymbolInfo *compiler_record_function(Compiler *self, Program *pg, const charspan *s, int chunk_id) {
    const SymbolInfo *result = symbol_table_find(&self->globals, s);
    if (result != NULL) {
        return result;
    }

    SymbolInfo new_info = {
        .name = *s,
        .id = self->globals.next_global_id++,
        .domain = symbol_func
    };

    return symbol_table_push(&self->globals, &new_info);
}

const SymbolInfo *compiler_record_local(Compiler *self, Program *pg, const charspan *s) {
    const SymbolInfo *result = symbol_table_find(&self->locals, s);
    if (result != NULL) {
        return result;
    }

    SymbolInfo new_info = {
        .name = *s,
        .id = self->locals.next_local_id++,
        .domain = symbol_local
    };

    return symbol_table_push(&self->locals, &new_info);
}

const SymbolInfo *compiler_record_constant(Compiler *self, Program *pg, const charspan *s_symbol, Value v) {
    const SymbolInfo *result = symbol_table_find(&self->locals, s_symbol);
    if (result != NULL) {
        return result;
    }

    // ? C++ equivalent: ... = m_chunks.back().constants;
    AnyVec_Value *constants = &AnyVec_Chunk_getm(&pg->chunks, self->chunk_idx)->constants;
    const int next_const_id = AnyVec_Value_len(constants);

    AnyVec_Value_push(constants, &v);

    SymbolInfo new_info = {
        .name = *s_symbol,
        .id = next_const_id,
        .domain = symbol_constant
    };

    return symbol_table_push(&self->locals, &new_info);
}



int8_t compiler_do_list(Compiler *self, Lexer *lexer, const charspan *s, Program *pg) {
    compiler_eat_tk(self, lexer, s); // ? SKIP '['

    int16_t item_count = 0;

    while (!compiler_match_curr(self, tk_eof)) {
        if (compiler_match_curr(self, tk_rbrack)) {
            break;
        }

        if (!compiler_do_or(self, lexer, s, pg)) {
            fprintf(stderr, "Note: See list item #%d around line %d", item_count, self->prev.line);
            return 0;
        }

        item_count++;

        if (compiler_match_curr(self, tk_comma)) {
            compiler_eat_tk(self, lexer, s);
        }
    }
    compiler_eat_tk(self, lexer, s);

    compiler_emit_op_unflagged(self, pg, op_mk_list, item_count);

    return 1;
}

int8_t compiler_do_literal(Compiler *self, Lexer *lexer, const charspan *s, Program *pg) {
    const Token *curr_ref = &self->curr;
    const charspan lexeme = {
        .data = s->data + curr_ref->begin,
        .length = curr_ref->length
    };

    const SymbolInfo *temp_locus = NULL;

    switch (curr_ref->tag) {
        case tk_none:
            compiler_emit_op(self, pg, op_put_none);
            compiler_eat_tk(self, lexer, s);
            return 1;
        case tk_true: case tk_false:
            compiler_emit_op_flagged(self, pg, op_put_bool, curr_ref->tag == 1, 0);
            compiler_eat_tk(self, lexer, s);
            return 1;
        case tk_integer:
            temp_locus = compiler_record_constant(
                self,
                pg,
                &lexeme,
                make_value_int(
                    charspan_atoi(&lexeme)
                )
            );
            compiler_eat_tk(self, lexer, s);
            break;
        case tk_identifier:
            temp_locus = compiler_resolve_name(
                self,
                &lexeme
            );
            compiler_eat_tk(self, lexer, s);
            break;
        case tk_lparen:
            compiler_eat_tk(self, lexer, s);
            if (!compiler_do_or(self, lexer, s, pg)) {
                fprintf(stderr, "Note: See parenthesized expr at around line %d.\n", self->prev.line);
                return 0;
            }
            if (!compiler_match_curr(self, tk_rparen)) {
                compiler_warn(self, "Expected ')' closing parenthesized expr.", &self->curr, s);
                return 0;
            }
            compiler_eat_tk(self, lexer, s);
            return 1;
        case tk_lbrack:
            return compiler_do_list(self, lexer, s, pg);
        default:
            compiler_warn(self, "Unexpected token in literal, expected none, true, false, or a name.", curr_ref, s);
            return 0;
    }

    if (temp_locus == NULL) {
        compiler_warn(self, "Undeclared name found here.", &self->prev, s);
        return 0;
    }

    // todo: add case for assignment LHS's of table accesses...
    if (compiler_flag_of(self, cgen_assign_to) && temp_locus->domain == symbol_local) {
        compiler_flag_on(self, cgen_lhs_local);
        self->saved_local_id = temp_locus->id;
        return 1;
    }

    switch (temp_locus->domain) {
        case symbol_constant:
            compiler_emit_op_unflagged(self, pg, op_put_k, temp_locus->id);
            break;
        case symbol_local:
            compiler_emit_op_unflagged(self, pg, op_load_local, temp_locus->id);
            break;
        case symbol_func:
        default:
            compiler_emit_op_unflagged(self, pg, op_load_imm_gid, temp_locus->id);
            break;
    }

    return 1;
}

int8_t compiler_do_lhs(Compiler *self, Lexer *lexer, const charspan *s, Program *pg) {
    if (!compiler_do_literal(self, lexer, s, pg)) {
        fprintf(stderr, "Note: See LHS of access-of expression at line %d.\n", self->curr.line);
        return 0;
    }

    if (!compiler_match_prev(self, tk_identifier) || !compiler_match_curr(self, tk_os_access_of)) {
        return 1;
    }

    const int8_t lhs_is_in_assign_lhs = compiler_flag_of(self, cgen_lhs_local);
    if (lhs_is_in_assign_lhs) {
        compiler_emit_op_flagged(self, pg, op_load_local, 0, self->saved_local_id);
        compiler_flag_off(self, cgen_lhs_local);
    }

    while (!compiler_match_curr(self, tk_eof)) {
        if (!compiler_match_curr(self, tk_os_access_of)) {
            break;
        }
        compiler_eat_tk(self, lexer, s);

        if (!compiler_do_literal(self, lexer, s, pg)) {
            fprintf(stderr, "Note: See RHS of access-of expression at line %d.\n", self->curr.line);
            return 0;
        }

        if (compiler_match_curr(self, tk_os_access_of)) {
            compiler_emit_op(self, pg, op_get_idx);
        } else if (lhs_is_in_assign_lhs) {
            compiler_flag_on(self, cgen_access_of);
        } else {
            compiler_emit_op(self, pg, op_get_idx);
        }
    }

    return 1;
}

int8_t compiler_do_call(Compiler *self, Lexer *lexer, const charspan *s, Program *pg) {
    const Token callee_name = self->curr;
    int arg_count = 0;

    if (!compiler_do_lhs(self, lexer, s, pg)) {
        fprintf(stderr, "Note: See call at line %d.\n", self->curr.line);
        return 0;
    }

    if (!compiler_match_curr(self, tk_lparen)) {
        return 1;
    }
    compiler_eat_tk(self, lexer, s);

    while (!compiler_match_curr(self, tk_eof)) {
        if (compiler_match_curr(self, tk_rparen)) {
            break;
        }

        if (!compiler_do_compare(self, lexer, s, pg)) {
            fprintf(stderr, "Note: See call at line %d.\n", callee_name.line);
            return 0;
        }

        arg_count++;

        if (compiler_match_curr(self, tk_comma)) {
            compiler_eat_tk(self, lexer, s);
        }
    }

    compiler_eat_tk(self, lexer, s);
    compiler_emit_op_unflagged(self, pg, op_call, arg_count);

    return 1;
}

int8_t compiler_do_factor(Compiler *self, Lexer *lexer, const charspan *s, Program *pg) {
    if (!compiler_do_call(self, lexer, s, pg)) {
        fprintf(stderr, "Note: See factor LHS at line %d.\n", self->curr.line);
        return 0;
    }

    Opcode op;

    while (!compiler_match_curr(self, tk_eof)) {
        const Token curr = self->curr;

        switch (curr.tag) {
            case tk_os_times: op = op_mul; break;
            case tk_os_slash: op = op_div; break;
            default: op = op_nop; break;
        }

        if (op == op_nop) {
            break;
        }
        compiler_eat_tk(self, lexer, s);

        if (!compiler_do_call(self, lexer, s, pg)) {
            fprintf(stderr, "Note: See factor RHS at line %d.\n", self->curr.line);
            return 0;
        }
        compiler_emit_op(self, pg, op);
    }

    return 1;
}

int8_t compiler_do_sum(Compiler *self, Lexer *lexer, const charspan *s, Program *pg) {
    if (!compiler_do_factor(self, lexer, s, pg)) {
        fprintf(stderr, "Note: See sum LHS at line %d.\n", self->curr.line);
        return 0;
    }

    Opcode op;

    while (!compiler_match_curr(self, tk_eof)) {
        const Token curr = self->curr;

        switch (curr.tag) {
            case tk_os_plus: op = op_add; break;
            case tk_os_minus: op = op_sub; break;
            default: op = op_nop; break;
        }

        if (op == op_nop) {
            break;
        }
        compiler_eat_tk(self, lexer, s);

        if (!compiler_do_factor(self, lexer, s, pg)) {
            fprintf(stderr, "Note: See sum RHS at line %d.\n", self->curr.line);
            return 0;
        }
        compiler_emit_op(self, pg, op);
    }

    return 1;
}

int8_t compiler_do_equality(Compiler *self, Lexer *lexer, const charspan *s, Program *pg) {
    if (!compiler_do_sum(self, lexer, s, pg)) {
        fprintf(stderr, "Note: See equality LHS at line %d.\n", self->curr.line);
        return 0;
    }

    Opcode op;

    while (!compiler_match_curr(self, tk_eof)) {
        const Token curr = self->curr;

        switch (curr.tag) {
            case tk_os_equals: op = op_eq; break;
            case tk_os_bang_equals: op = op_ne; break;
            default: op = op_nop; break;
        }

        if (op == op_nop) {
            break;
        }
        compiler_eat_tk(self, lexer, s);

        if (!compiler_do_sum(self, lexer, s, pg)) {
            fprintf(stderr, "Note: See equality RHS at line %d.\n", self->curr.line);
            return 0;
        }
        compiler_emit_op(self, pg, op);
    }

    return 1;
}

int8_t compiler_do_compare(Compiler *self, Lexer *lexer, const charspan *s, Program *pg) {
    if (!compiler_do_equality(self, lexer, s, pg)) {
        fprintf(stderr, "Note: See compare LHS at line %d.\n", self->curr.line);
        return 0;
    }

    Opcode op;

    while (!compiler_match_curr(self, tk_eof)) {
        const Token curr = self->curr;

        switch (curr.tag) {
            case tk_os_lesser: op = op_lt; break;
            case tk_os_greater: op = op_gt; break;
            default: op = op_nop; break;
        }

        if (op == op_nop) {
            break;
        }
        compiler_eat_tk(self, lexer, s);

        if (!compiler_do_equality(self, lexer, s, pg)) {
            fprintf(stderr, "Note: See compare RHS at line %d.\n", self->curr.line);
            return 0;
        }
        compiler_emit_op(self, pg, op);
    }

    return 1;
}

int8_t compiler_do_and(Compiler *self, Lexer *lexer, const charspan *s, Program *pg) {
    if (!compiler_do_compare(self, lexer, s, pg)) {
        fprintf(stderr, "Note: See and-expression LHS at line %d.\n", self->curr.line);
        return 0;
    }

    if (!compiler_match_curr(self, tk_os_and)) {
        return 1;
    }
    compiler_eat_tk(self, lexer, s);

    const int16_t falsy_jmp_pos = pg->chunks.data[self->chunk_idx].code.length;
    compiler_emit_op_flagged(self, pg, op_jmp_false, 0, 0);
    compiler_emit_op_flagged(self, pg, op_pop, 1, 0); // ? Pop LHS if true, keeping our VM's "single result value" invariant.

    if (!compiler_do_compare(self, lexer, s, pg)) {
        fprintf(stderr, "Note: See and-expression RHS at line %d.\n", self->curr.line);
        return 0;
    }

    const int16_t falsy_jmp_end = pg->chunks.data[self->chunk_idx].code.length;
    compiler_emit_op(self, pg, op_nop);

    pg->chunks.data[self->chunk_idx].code.data[falsy_jmp_pos].wide = falsy_jmp_end - falsy_jmp_pos;

    return 1;
}

int8_t compiler_do_or(Compiler *self, Lexer *lexer, const charspan *s, Program *pg) {
    if (!compiler_do_and(self, lexer, s, pg)) {
        fprintf(stderr, "Note: See or-expression LHS at line %d.\n", self->curr.line);
        return 0;
    }

    if (!compiler_match_curr(self, tk_os_or)) {
        return 1;
    }
    compiler_eat_tk(self, lexer, s);

    const int16_t truthy_jmp_pos = pg->chunks.data[self->chunk_idx].code.length;
    compiler_emit_op_flagged(self, pg, op_jmp_if, 0, 0);
    compiler_emit_op_flagged(self, pg, op_pop, 1, 0); // ? Pop LHS if true, keeping our VM's "single result value" invariant.

    if (!compiler_do_and(self, lexer, s, pg)) {
        fprintf(stderr, "Note: See or-expression RHS at line %d.\n", self->curr.line);
        return 0;
    }

    const int16_t truthy_jmp_end = pg->chunks.data[self->chunk_idx].code.length;
    compiler_emit_op(self, pg, op_nop);

    pg->chunks.data[self->chunk_idx].code.data[truthy_jmp_pos].wide = truthy_jmp_end - truthy_jmp_pos;

    return 1;
}

int8_t compiler_do_vars(Compiler *self, Lexer *lexer, const charspan *s, Program *pg) {
    compiler_eat_tk(self, lexer, s); // ? skip LET

    while (!compiler_match_curr(self, tk_eof)) {
        if (compiler_match_curr(self, tk_semicolon)) {
            break;
        } else if (!compiler_match_curr(self, tk_identifier)) {
            compiler_warn(self, "Expected name in variable declaration here.", &self->curr, s);
            return 0;
        }

        const Token var_name = self->curr;
        const charspan raw_name = {
            .data = s->data + var_name.begin,
            .length = var_name.length
        };

        compiler_eat_tk(self, lexer, s);

        const SymbolInfo *var_locus = compiler_record_local(self, pg, &raw_name);

        if (!compiler_match_curr(self, tk_colon)) {
            compiler_warn(self, "Expected ':' before variable initializer.", &self->curr, s);
            return 0;
        }
        compiler_eat_tk(self, lexer, s);

        if (!compiler_do_or(self, lexer, s, pg)) {
            return 0;
        }

        if (compiler_match_curr(self, tk_comma)) {
            compiler_eat_tk(self, lexer, s);
        }
    }

    compiler_eat_tk(self, lexer, s);

    return 1;
}

int8_t compiler_do_ifs(Compiler *self, Lexer *lexer, const charspan *s, Program *pg) {
    compiler_eat_tk(self, lexer, s); // ? skip IF

    const int cmp_line = self->curr.line;
    if (!compiler_do_or(self, lexer, s, pg)) {
        fprintf(stderr, "Note: See if-statement at line %d.\n", cmp_line);
        return 0;
    }

    const int16_t jump_else_pos = pg->chunks.data[self->chunk_idx].code.length;
    compiler_emit_op_flagged(self, pg, op_jmp_false, 0, 0);
    compiler_emit_op_flagged(self, pg, op_pop, 1, 0);

    if (!compiler_do_block(self, lexer, s, pg)) {
        fprintf(stderr, "Note: See in if-block at around line %d.\n", self->curr.line);
        return 0;
    }
    
    const int16_t jump_skip_else_pos = pg->chunks.data[self->chunk_idx].code.length;
    compiler_emit_op_unflagged(self, pg, op_jmp, 0);

    // * BEGIN ELSE clause ... * //

    if (!compiler_match_curr(self, tk_keyword_else)) {
        compiler_warn(self, "Expected 'else' in if-else-statement.", &self->curr, s);
        return 0;
    }
    compiler_eat_tk(self, lexer, s); // ? consume leading 'ELSE' of ELSE body

    const int16_t begin_else_pos = jump_skip_else_pos + 1;
    if (!compiler_do_block(self, lexer, s, pg)) {
        fprintf(stderr, "Note: See in if-block at around line %d.\n", self->curr.line);
        return 0;
    }

    const int16_t end_ifs_pos = pg->chunks.data[self->chunk_idx].code.length;
    compiler_emit_op(self, pg, op_nop);
    pg->chunks.data[self->chunk_idx].code.data[jump_else_pos].wide = begin_else_pos - jump_else_pos;
    pg->chunks.data[self->chunk_idx].code.data[jump_skip_else_pos].wide = end_ifs_pos - jump_skip_else_pos;

    return 1;
}

int8_t compiler_do_while(Compiler *self, Lexer *lexer, const charspan *s, Program *pg) {
    compiler_eat_tk(self, lexer, s); // ? skip WHILE

    const int16_t while_check_pos = pg->chunks.data[self->chunk_idx].code.length;
    if (!compiler_do_or(self, lexer, s, pg)) {
        fprintf(stderr, "Note: See while-loop condition around line %d.\n", self->curr.line);
        return 0;
    }

    const int16_t while_jmp_out_pos = pg->chunks.data[self->chunk_idx].code.length;
    compiler_emit_op_flagged(self, pg, op_jmp_false, 0, 0); // ? flags = 0 ==> forward jump applies!

    if (!compiler_do_block(self, lexer, s, pg)) {
        fprintf(stderr, "Note: See while-body around line %d.\n", self->curr.line);
        return 0;
    }

    const int16_t while_jmp_back_pos = pg->chunks.data[self->chunk_idx].code.length;
    compiler_emit_op_flagged(self, pg, op_jmp, 1, 0); // ? flags = 1 ==> backwards jump applies!
    const int16_t while_exit_pos = pg->chunks.data[self->chunk_idx].code.length;
    compiler_emit_op(self, pg, op_pop); // ? pop off check after loop quits WHEN it's FALSE

    pg->chunks.data[self->chunk_idx].code.data[while_jmp_back_pos].wide = while_jmp_back_pos - while_check_pos;
    pg->chunks.data[self->chunk_idx].code.data[while_jmp_out_pos].wide = while_exit_pos - while_jmp_out_pos;

    return 1;
}

int8_t compiler_do_ret(Compiler *self, Lexer *lexer, const charspan *s, Program *pg) {
    compiler_eat_tk(self, lexer, s); // ? skip RET

    if (!compiler_do_or(self, lexer, s, pg)) {
        fprintf(stderr, "Note: See return-result expression at line %d.\n", self->curr.line);
        return 0;
    }
    compiler_emit_op(self, pg, op_ret);

    if (!compiler_match_curr(self, tk_semicolon)) {
        compiler_warn(self, "Expected ';' after return statement.", &self->curr, s);
        return 0;
    }
    compiler_eat_tk(self, lexer, s);

    return 1;
}

int8_t compiler_do_expr_stmt(Compiler *self, Lexer *lexer, const charspan *s, Program *pg) {
    // ? Process / emit LHS first...
    compiler_flag_on(self, cgen_assign_to);
    if (!compiler_do_or(self, lexer, s, pg)) {
        compiler_flag_off(self, cgen_assign_to);
        fprintf(stderr, "See expr-stmt at line %d.\n", self->curr.line);
        return 0;
    }
    compiler_flag_off(self, cgen_assign_to);

    if (compiler_match_curr(self, tk_os_bind_equals)) {
        compiler_eat_tk(self, lexer, s);

        if (!compiler_do_or(self, lexer, s, pg)) {
            fprintf(stderr, "Note: see RHS of assignment at line %d.\n", self->curr.line);
            return 0;
        }

        // ? If we have consumed only a name = <value>, emit a simple update of that local slot.
        if (compiler_flag_of(self, cgen_lhs_local)) {
            compiler_emit_op_flagged(self, pg, op_store_local, 0, self->saved_local_id);
            compiler_flag_off(self, cgen_lhs_local);
        } else if (compiler_flag_of(self, cgen_access_of)) {
            compiler_emit_op(self, pg, op_set_idx);
            compiler_flag_off(self, cgen_access_of);
        } else {
            compiler_warn(self, "Invalid LHS of assignment, expected a name or key-access expression around line %d.\n", &self->prev, s);
            // return 0;
        }
    }

    if (!compiler_match_curr(self, tk_semicolon)) {
        compiler_warn(self, "Expected ';' after expr-stmt.", &self->curr, s);
        return 0;
    }
    compiler_eat_tk(self, lexer, s);

    return 1;
}

int8_t compiler_do_nestable_stmt(Compiler *self, Lexer *lexer, const charspan *s, Program *pg) {
    switch (self->curr.tag) {
    case tk_keyword_let:
        return compiler_do_vars(self, lexer, s, pg);
    case tk_keyword_if:
        return compiler_do_ifs(self, lexer, s, pg);
    case tk_keyword_while:
        return compiler_do_while(self, lexer, s, pg);
    case tk_keyword_ret:
        return compiler_do_ret(self, lexer, s, pg);
    default:
        return compiler_do_expr_stmt(self, lexer, s, pg);
    }
}

int8_t compiler_do_block(Compiler *self, Lexer *lexer, const charspan *s, Program *pg) {
    if (!compiler_match_curr(self, tk_colon)) {
        compiler_warn(self, "Expected ':' starting block.", &self->curr, s);
        return 0;
    }
    compiler_eat_tk(self, lexer, s);

    while (!compiler_match_curr(self, tk_eof)) {
        if (compiler_match_curr(self, tk_keyword_end)) {
            break;
        }

        const int stmt_line = self->curr.line;
        if (!compiler_do_nestable_stmt(self, lexer, s, pg)) {
            fprintf(stderr, "Note: See nested statemnt in block body around line %d\n", stmt_line);
            return 0;
        }
    }
    compiler_eat_tk(self, lexer, s);

    return 1;
}

int8_t compiler_do_func(Compiler *self, Lexer *lexer, const charspan *s, Program *pg) {
    compiler_eat_tk(self, lexer, s); // ? skip FUN

    if (!compiler_match_curr(self, tk_identifier)) {
        compiler_warn(self, "Expected name for FUN declaration.", &self->curr, s);
        return 0;
    }
    compiler_eat_tk(self, lexer, s);

    const charspan name_lexeme = {
        .data = s->data + self->prev.begin,
        .length = self->prev.length
    };
    Chunk temp_chunk;
    Chunk_dud(&temp_chunk);
    AnyVec_Chunk_push(&pg->chunks, &temp_chunk);

    // ? Put index to this procedure's code chunk and reset it to 0 again once we return to top-level code... This works since there's only 1 global scope & 1 nested, local scope per procedure.
    const int16_t old_chunk_idx = 0;
    self->chunk_idx = pg->chunks.length - 1;
    compiler_record_function(self, pg, &name_lexeme, self->chunk_idx);

    if (!compiler_match_curr(self, tk_lparen)) {
        return 0;
    }
    compiler_eat_tk(self, lexer, s);

    while (!compiler_match_curr(self, tk_eof)) {
        if (compiler_match_curr(self, tk_rparen)) {
            break;
        } else if (!compiler_match_curr(self, tk_identifier)) {
            compiler_warn(self, "Expected name in params list here.", &self->curr, s);
            return 0;
        }
        
        // ? Eat checked identifier token here, as it's simpler to process it as self->curr.
        const charspan param_name = {
            .data = s->data + self->curr.begin,
            .length = self->curr.length
        };
        compiler_record_local(self, pg, &param_name);
        compiler_eat_tk(self, lexer, s);

        if (compiler_match_curr(self, tk_comma)) {
            compiler_eat_tk(self, lexer, s);
        }
    }
    compiler_eat_tk(self, lexer, s);

    if (!compiler_do_block(self, lexer, s, pg)) {
        fprintf(stderr, "Note: See in FUN declaration around line %d.\n", self->curr.line);
        return 0;
    } else {
        symbol_table_clear(&self->locals);
        self->chunk_idx = 0;
    }

    return 1;
}

int8_t compiler_do_stmt(Compiler *self, Lexer *lexer, const charspan *s, Program *pg) {
    switch (self->curr.tag) {
    case tk_keyword_let:
        return compiler_do_vars(self, lexer, s, pg);
    case tk_keyword_if:
        return compiler_do_ifs(self, lexer, s, pg);
    case tk_keyword_while:
        return compiler_do_while(self, lexer, s, pg);
    case tk_keyword_ret:
        return compiler_do_ret(self, lexer, s, pg);
    case tk_keyword_fun:
        return compiler_do_func(self, lexer, s, pg);
    default:
        return compiler_do_expr_stmt(self, lexer, s, pg);
    }
}

int8_t compiler_do_source(Compiler *self, Lexer *lexer, const charspan *s, Program *pg) {
    compiler_eat_tk(self, lexer, s); // ? remove the unknown token placeholder by getting the 1st token into self->curr... this is needed for correct parsing --> bytecode

    // ! IMPORTANT: push an empty bytecode chunk so that an OOB terminate doesn't happen when ~ L248 tries to push a constant, etc. for AnyVec_<type>.
    Chunk temp;
    Chunk_new(&temp); // ? initialize empty chunk
    AnyVec_Chunk_push(&pg->chunks, &temp); // ? copy the empty chunk into this Vec, but don't touch temp again... just did scuffed destructive moves??

    while (!compiler_match_curr(self, tk_eof)) {
        if (!compiler_do_stmt(self, lexer, s, pg)) {
            // ? Recover parsing by skipping to a LET declaration as a synchronization point.
            while (!compiler_match_curr(self, tk_eof)) {
                if (compiler_match_curr(self, tk_keyword_let)) {
                    break;
                }

                compiler_eat_tk(self, lexer, s);
            }
        }
    }

    compiler_emit_op(self, pg, op_ret); // ! NOTE: emit a redundant RET in case the user forgets one- otherwise the VM reads an invalid IP!
    pg->entry_id = 0;

    fprintf(stderr, "Compilation finished with \x1b[1;31m%d\x1b[0m errors.\n\n", self->errors);

    return self->errors == 0;
}
