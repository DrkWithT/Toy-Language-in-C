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
            .next_global_id = 0
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
        .errors = 0
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
    fprintf(stderr, "Compile Err %dat line %d:\n\tnote: %s", self->errors, tk->line, msg);
}

size_t compiler_emit_op(Program *pg, Opcode op) {
    AnyVec_Instruction *code_ref = &AnyVec_Chunk_getm(&pg->chunks, AnyVec_Chunk_len(&pg->chunks) - 1)->code;
    Instruction temp = {
        .op = op,
        .flag = 0,
        .wide = 0
    };

    AnyVec_Instruction_push(code_ref, &temp);

    return AnyVec_Instruction_len(code_ref);
}

size_t compiler_emit_op_unflagged(Program *pg, Opcode op, int16_t wide) {
    AnyVec_Instruction *code_ref = &AnyVec_Chunk_getm(&pg->chunks, AnyVec_Chunk_len(&pg->chunks) - 1)->code;
    Instruction temp = {
        .op = op,
        .flag = 0,
        .wide = wide
    };

    AnyVec_Instruction_push(code_ref, &temp);

    return AnyVec_Instruction_len(code_ref);
}

size_t compiler_emit_op_flagged(Program *pg, Opcode op, uint8_t flags, int16_t wide) {
    AnyVec_Instruction *code_ref = &AnyVec_Chunk_getm(&pg->chunks, AnyVec_Chunk_len(&pg->chunks) - 1)->code;
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
    AnyVec_Value *constants = &AnyVec_Chunk_getm(&pg->chunks, AnyVec_Chunk_len(&pg->chunks) - 1)->constants;
    const int next_const_id = AnyVec_Value_len(constants);

    AnyVec_Value_push(constants, &v);

    SymbolInfo new_info = {
        .name = *s_symbol,
        .id = next_const_id,
        .domain = symbol_local
    };

    return symbol_table_push(&self->locals, &new_info);
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
            compiler_emit_op(pg, op_put_none);
            compiler_eat_tk(self, lexer, s);
            return 1;
        case tk_true: case tk_false:
            compiler_emit_op_flagged(pg, op_put_bool, curr_ref->tag == 1, 0);
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
        default:
            compiler_warn(self, "Unexpected token in literal, expected none, true, false, or a name.", curr_ref, s);
            return 0;
    }

    if (temp_locus == NULL) {
        compiler_warn(self, "Undeclared name found here.", &self->prev, s);
        return 0;
    }

    switch (temp_locus->domain) {
        case symbol_constant:
            compiler_emit_op_unflagged(pg, op_put_k, temp_locus->id);
            break;
        case symbol_local:
            compiler_emit_op_unflagged(pg, op_load_local, temp_locus->id);
            break;
        case symbol_func:
        default:
            compiler_emit_op_unflagged(pg, op_load_imm_gid, temp_locus->id);
            break;
    }

    return 1;
}

int8_t compiler_do_call(Compiler *self, Lexer *lexer, const charspan *s, Program *pg) {
    const Token callee_name = self->curr;
    int arg_count = 0;

    if (!compiler_do_literal(self, lexer, s, pg)) {
        fprintf(stderr, "Note: See call at line %d.\n", self->curr.line);
        return 0;
    }

    if (!compiler_match_curr(self, tk_lparen)) {
        compiler_warn(self, "Unexpected token starting call args, expected '('.", &callee_name, s);
        return 0;
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
    compiler_emit_op_unflagged(pg, op_call, arg_count);

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
        compiler_emit_op(pg, op);
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
        compiler_emit_op(pg, op);
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
        compiler_emit_op(pg, op);
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
        compiler_emit_op(pg, op);
    }

    return 1;
}

int8_t compiler_do_vars(Compiler *self, Lexer *lexer, const charspan *s, Program *pg) {
    compiler_eat_tk(self, lexer, s); // ? skip LET

    while (!compiler_match_curr(self, tk_eof)) {
        if (compiler_match_curr(self, tk_semicolon)) {
            break;
        }

        if (!compiler_match_curr(self, tk_identifier)) {
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

        if (!compiler_do_compare(self, lexer, s, pg)) {
            return 0;
        }

        if (compiler_match_curr(self, tk_comma)) {
            compiler_eat_tk(self, lexer, s);
        }
    }

    compiler_eat_tk(self, lexer, s);

    return 0;
}

// int8_t compiler_do_ifs(Compiler *self, Lexer *lexer, const charspan *s, Program *pg);
// int8_t compiler_do_while(Compiler *self, Lexer *lexer, const charspan *s, Program *pg);

int8_t compiler_do_ret(Compiler *self, Lexer *lexer, const charspan *s, Program *pg) {
    compiler_eat_tk(self, lexer, s); // ? skip RET

    if (!compiler_do_compare(self, lexer, s, pg)) {
        fprintf(stderr, "Note: See return-result expression at line %d.\n", self->curr.line);
        return 0;
    }
    compiler_emit_op(pg, op_ret);

    if (!compiler_match_curr(self, tk_semicolon)) {
        compiler_warn(self, "Expected ';' after return statement.", &self->curr, s);
        return 0;
    }
    compiler_eat_tk(self, lexer, s);

    return 1;
}

int8_t compiler_do_expr_stmt(Compiler *self, Lexer *lexer, const charspan *s, Program *pg) {
    if (!compiler_do_compare(self, lexer, s, pg)) {
        fprintf(stderr, "See expr-stmt at line %d.\n", self->curr.line);
        return 0;
    }

    if (!compiler_match_curr(self, tk_semicolon)) {
        compiler_warn(self, "Expected ';' after expr-stmt.", &self->curr, s);
        return 0;
    }
    compiler_eat_tk(self, lexer, s);

    return 0;
}

// int8_t compiler_do_func(Compiler *self, Lexer *lexer, const charspan *s, Program *pg);

int8_t compiler_do_source(Compiler *self, Lexer *lexer, const charspan *s, Program *pg) {
    while (!compiler_match_curr(self, tk_eof)) {
        int8_t emission_ok = 0;

        switch (self->curr.tag) {
            case tk_keyword_let:
                emission_ok = compiler_do_vars(self, lexer, s, pg);
                break;
            case tk_keyword_ret:
                emission_ok = compiler_do_ret(self, lexer, s, pg);
                break;
            default:
                emission_ok = compiler_do_expr_stmt(self, lexer, s, pg);
                break;
        }

        if (!emission_ok) {
            // ? Recover parsing by skipping to a LET declaration as a synchronization point.
            while (!compiler_match_curr(self, tk_eof)) {
                if (compiler_match_curr(self, tk_keyword_let)) {
                    break;
                }

                compiler_eat_tk(self, lexer, s);
            }
        }
    }

    fprintf(stderr, "Compilation finished with \x1b[1;31m%d\x1b[0m errors.", self->errors);

    return self->errors == 0;
}
