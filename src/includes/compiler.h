#ifndef TOYSCRIPT_COMPILER_H
#define TOYSCRIPT_COMPILER_H

#include "mystr.h"
#include "lex.h"
#include "bytecode.h"



int charspan_atoi(const charspan *s);

#define DEFAULT_SYMBOL_CAPACITY 8

typedef enum symbol_domain_t : uint8_t {
    symbol_constant,
    symbol_local,
    symbol_func
} Domain;

typedef struct symbol_info_t {
    charspan name;
    int16_t id;
    Domain domain;
} SymbolInfo;

SymbolInfo make_symbol_info(charspan name_v, int16_t id, Domain d);

typedef struct symbol_table_t {
    SymbolInfo *infos;
    int length;
    int capacity;
    int16_t next_local_id;      // ? reused for local IDs
    int16_t next_global_id;     // ? reused for global / constant IDs
} SymbolTable;

SymbolTable make_symbol_table();
void symbol_table_del(SymbolTable *self);
void symbol_table_clear(SymbolTable *self);
const SymbolInfo *symbol_table_find(const SymbolTable *symbols, const charspan *s);
const SymbolInfo *symbol_table_push(SymbolTable *symbols, const SymbolInfo *info);

typedef struct compiler_t {
    SymbolTable globals;
    SymbolTable locals;
    Token prev;
    Token curr;
    int errors;
    int16_t chunk_idx;  // ? 0 indexes top level code, 1+ indexes a code chunk per procedure, applying only for compiling a FUN decl.
} Compiler;

Compiler make_compiler();
void compiler_del(Compiler *self);

int8_t compiler_match_curr(const Compiler *self, TkTag tag);
int8_t compiler_match_prev(const Compiler *self, TkTag tag);
Token compiler_advance_tk(Compiler *self, Lexer *lexer, const charspan *s);
void compiler_eat_tk(Compiler *self, Lexer *lexer, const charspan *s);
void compiler_warn(Compiler *self, const char *msg, const Token *tk, const charspan *s);

size_t compiler_emit_op(Compiler *self, Program *pg, Opcode op);
size_t compiler_emit_op_unflagged(Compiler *self, Program *pg, Opcode op, int16_t wide);
size_t compiler_emit_op_flagged(Compiler *self, Program *pg, Opcode op, uint8_t flags, int16_t wide);

const SymbolInfo *compiler_resolve_name(const Compiler *self, const charspan *s);
const SymbolInfo *compiler_record_function(Compiler *self, Program *pg, const charspan *s, int chunk_id);
const SymbolInfo *compiler_record_local(Compiler *self, Program *pg, const charspan *s);
const SymbolInfo *compiler_record_constant(Compiler *self, Program *pg, const charspan *s_symbol, Value v);

int8_t compiler_do_literal(Compiler *self, Lexer *lexer, const charspan *s, Program *pg);
int8_t compiler_do_call(Compiler *self, Lexer *lexer, const charspan *s, Program *pg);
int8_t compiler_do_factor(Compiler *self, Lexer *lexer, const charspan *s, Program *pg);
int8_t compiler_do_sum(Compiler *self, Lexer *lexer, const charspan *s, Program *pg);
int8_t compiler_do_equality(Compiler *self, Lexer *lexer, const charspan *s, Program *pg);
int8_t compiler_do_compare(Compiler *self, Lexer *lexer, const charspan *s, Program *pg);
int8_t compiler_do_and(Compiler *self, Lexer *lexer, const charspan *s, Program *pg);
int8_t compiler_do_or(Compiler *self, Lexer *lexer, const charspan *s, Program *pg);

int8_t compiler_do_vars(Compiler *self, Lexer *lexer, const charspan *s, Program *pg);
int8_t compiler_do_ifs(Compiler *self, Lexer *lexer, const charspan *s, Program *pg);
// int8_t compiler_do_while(Compiler *self, Lexer *lexer, const charspan *s, Program *pg);
int8_t compiler_do_ret(Compiler *self, Lexer *lexer, const charspan *s, Program *pg);
int8_t compiler_do_expr_stmt(Compiler *self, Lexer *lexer, const charspan *s, Program *pg);
int8_t compiler_do_func(Compiler *self, Lexer *lexer, const charspan *s, Program *pg);
int8_t compiler_do_nestable_stmt(Compiler *self, Lexer *lexer, const charspan *s, Program *pg);
int8_t compiler_do_block(Compiler *self, Lexer *lexer, const charspan *s, Program *pg);
int8_t compiler_do_stmt(Compiler *self, Lexer *lexer, const charspan *s, Program *pg);

int8_t compiler_do_source(Compiler *self, Lexer *lexer, const charspan *s, Program *pg);

#endif
