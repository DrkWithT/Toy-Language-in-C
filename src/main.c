#include <stdio.h>
#include <string.h>

#include "mystr.h"
#include "bytecode.h"
#include "compiler.h"

static const LexItem special_lexicals[] = {
    (LexItem) {.literal = "NIL", .tag = tk_none},
    (LexItem) {.literal = "TRUE", .tag = tk_true},
    (LexItem) {.literal = "FALSE", .tag = tk_false},
    (LexItem) {.literal = "LET", .tag = tk_keyword_let},
    (LexItem) {.literal = "IF", .tag = tk_keyword_if},
    (LexItem) {.literal = "ELSE", .tag = tk_keyword_else},
    (LexItem) {.literal = "WHILE", .tag = tk_keyword_while},
    (LexItem) {.literal = "RET", .tag = tk_keyword_ret},
    (LexItem) {.literal = "FUN", .tag = tk_keyword_fun},
    (LexItem) {.literal = "*", .tag = tk_os_times},
    (LexItem) {.literal = "/", .tag = tk_os_slash},
    (LexItem) {.literal = "+", .tag = tk_os_plus},
    (LexItem) {.literal = "-", .tag = tk_os_minus},
    (LexItem) {.literal = "==", .tag = tk_os_equals},
    (LexItem) {.literal = "!=", .tag = tk_os_bang_equals},
    (LexItem) {.literal = "<", .tag = tk_os_lesser},
    (LexItem) {.literal = ">", .tag = tk_os_greater}
};


mystr read_file(const char *fname) {
    FILE *fs = fopen(fname, "r");

    if (!fs) {
        return (mystr) {
            .data = NULL,
            .capacity = 0,
            .length = 0
        };
    }

    char chunk[64];
    mystr source;
    mystr_res(&source, 64);

    size_t rc = 0;

    while (1) {
        rc = fread(chunk, sizeof(char), 32, fs);

        if (rc == 0) {
            break;
        }

        chunk[rc] = '\0';
        mystr_append_raw(&source, chunk, rc);
    }

    if (ferror(fs)) {
        perror("File read failed.");
        return (mystr) {
            .data = NULL,
            .capacity = 0,
            .length = 0
        };
    }

    fclose(fs);
    return source;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: ./toyscript [-h | -v | <file name>]\n-h: help\n-v: show version\n");
        return 1;
    }

    mystr source_str = read_file(argv[1]);

    puts("Read source is:\n");
    puts(mystr_raw(&source_str));

    charspan source_view;
    charspan_new(&source_view, mystr_raw(&source_str), mystr_len(&source_str));

    Lexer tokenizer = make_lexer(&source_view, special_lexicals);
    Compiler compiler = make_compiler();

    Program program;
    program_dud(&program);

    if (!compiler_do_source(&compiler, &tokenizer, &source_view, &program)) {
        perror("Please check all compile errors above.");
        return 1;
    }

    program_del(&program);
    compiler_del(&compiler);
    charspan_del(&source_view);
    mystr_del(&source_str);
    return 0;
}
