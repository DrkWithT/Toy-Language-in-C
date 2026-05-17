#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "mystr.h"
#include "bytecode.h"
#include "compiler.h"
#include "vm.h"

#include "natives.h"



#define VM_STACK_MAX 1024
#define VM_DEPTH_MAX 64


static const char *project_name = " _____         _____         _     _  \n"
"|_   _|___ _ _|   __|___ ___|_|___| |_ \n"
"  | | | . | | |__   |  _|  _| | . |  _|\n"
"  |_| |___|_  |_____|___|_| |_|  _|_|  \n"
"          |___|               |_|      \n";

static const LexItem special_lexicals[] = {
    (LexItem) {.literal = "NIL", .tag = tk_none},
    (LexItem) {.literal = "TRUE", .tag = tk_true},
    (LexItem) {.literal = "FALSE", .tag = tk_false},
    (LexItem) {.literal = "LET", .tag = tk_keyword_let},
    (LexItem) {.literal = "IF", .tag = tk_keyword_if},
    (LexItem) {.literal = "ELSE", .tag = tk_keyword_else},
    (LexItem) {.literal = "WHILE", .tag = tk_keyword_while},
    (LexItem) {.literal = "FOR", .tag = tk_keyword_for},
    (LexItem) {.literal = "BREAK", .tag = tk_keyword_break},
    (LexItem) {.literal = "CONTINUE", .tag = tk_keyword_continue},
    (LexItem) {.literal = "RET", .tag = tk_keyword_ret},
    (LexItem) {.literal = "FUN", .tag = tk_keyword_fun},
    (LexItem) {.literal = "END", .tag = tk_keyword_end},
    (LexItem) {.literal = "*", .tag = tk_os_times},
    (LexItem) {.literal = "/", .tag = tk_os_slash},
    (LexItem) {.literal = "+", .tag = tk_os_plus},
    (LexItem) {.literal = "-", .tag = tk_os_minus},
    (LexItem) {.literal = "==", .tag = tk_os_equals},
    (LexItem) {.literal = "!=", .tag = tk_os_bang_equals},
    (LexItem) {.literal = "<", .tag = tk_os_lesser},
    (LexItem) {.literal = ">", .tag = tk_os_greater},
    (LexItem) {.literal = "&&", .tag = tk_os_and},
    (LexItem) {.literal = "||", .tag = tk_os_or},
    (LexItem) {.literal = ":=", .tag = tk_os_bind_equals},
    (LexItem) {.literal = "::", .tag = tk_os_access_of},
    (LexItem) {.literal = ":", .tag = tk_colon}
};

// ? Stores a corresponding native function per index, 1-to-1 against toyscript_native_names.
static const NativeFn toyscript_natives[] = {
    native_print,
    native_powf,
    native_sqrtf,
    native_clampf,
    native_floorf,
    native_ceilf,
    native_console_readln,
    native_console_reset
    // TODO: add more.
};

// ? Stores name to position info for native functions so that the compiler properly generates native calls.
static const charspan toyscript_native_names[] = {
    (charspan) {
        .data = "print",
        .length = 5
    },
    (charspan) {
        .data = "powf",
        .length = 4
    },
    (charspan) {
        .data = "sqrtf",
        .length = 5
    },
    (charspan) {
        .data = "clampf",
        .length = 6
    },
    (charspan) {
        .data = "floorf",
        .length = 6
    },
    (charspan) {
        .data = "ceilf",
        .length = 5
    },
    (charspan) {
        .data = "creadln",
        .length = 7
    },
    (charspan) {
        .data = "creset",
        .length = 6
    }
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

    size_t total_rc = 0;
    size_t rc = 0;

    while (1) {
        rc = fread(chunk, sizeof(char), 32, fs);

        if (rc == 0) {
            break;
        }

        total_rc += rc;

        chunk[rc] = '\0';
        mystr_append_raw(&source, chunk, rc);
    }

    if (ferror(fs)) {
        fprintf(stderr, "File read failed.\n");
        return (mystr) {
            .data = NULL,
            .capacity = 0,
            .length = 0
        };
    } else if (total_rc == 0) {
        mystr_del(&source);
    }

    fclose(fs);
    return source;
}



int main(int argc, char *argv[]) {
    const char *source_fname = NULL;
    int8_t show_version = 0;
    int8_t show_help = 0;
    int8_t dump_bc = 0;
    int8_t allow_run = 0;

    if (argc < 2) {
        fprintf(stderr, "usage: ./toyscript [-h | -v | [-d | -r] <file name>]\n-h: help\n-v: show version\n");
        return 1;
    }

    for (int16_t arg_i = 0; arg_i < argc - 1; arg_i++) {
        if (!strcmp(argv[1 + arg_i], "-v")) { show_version = 1;}
        else if (!strcmp(argv[1 + arg_i], "-h")) { show_help = 1; }
        else if (!strcmp(argv[1 + arg_i], "-d")) { dump_bc = 1; }
        else if (!strcmp(argv[1 + arg_i], "-r")) { allow_run = 1; }
        else { source_fname = argv[1 + arg_i]; }
    }

    if (show_version) {
        printf("\x1b[1;33m%s\x1b[0m\n\nv0.4.9\t By: DrkWithT (GitHub)", project_name);
        return 0;
    } else if (show_help) {
        printf("usage: ./toyscript [-h | -v | [-d | -r] <file name>]\n-h: help\n-v: show version\n");
        return 0;
    }

    mystr source_str = (source_fname != NULL) ? read_file(source_fname) : (mystr) {.data = NULL, .length = 0, .capacity = 0};

    if (mystr_empty(&source_str)) {
        fprintf(stderr, "Read Error: The file at \x1b[1;29m%s\x1b[0m could not be read.", source_fname);
        return 1;
    }

    charspan source_view;
    charspan_new(&source_view, mystr_raw(&source_str), mystr_len(&source_str));

    Lexer tokenizer = make_lexer(&source_view, special_lexicals);
    Compiler compiler = make_compiler();

    // TODO: add more library functions for time, math, I/O.
    compiler_map_native(&compiler, &toyscript_native_names[0]);
    compiler_map_native(&compiler, &toyscript_native_names[1]);
    compiler_map_native(&compiler, &toyscript_native_names[2]);
    compiler_map_native(&compiler, &toyscript_native_names[3]);
    compiler_map_native(&compiler, &toyscript_native_names[4]);
    compiler_map_native(&compiler, &toyscript_native_names[5]);
    compiler_map_native(&compiler, &toyscript_native_names[6]);
    compiler_map_native(&compiler, &toyscript_native_names[7]);

    Program program;
    program_dud(&program);

    if (!compiler_do_source(&compiler, &tokenizer, &source_view, &program)) {
        return 1;
    }
    
    if (dump_bc) {
        dump_program(&program);
    }

    if (!allow_run) {
        return 0;
    }

    VMState vm = make_vm(&program, toyscript_natives, VM_STACK_MAX, VM_DEPTH_MAX, DEFAULT_HEAP_CAPACITY);

    struct timeval begin, end;
    gettimeofday(&begin, NULL);
    const VMStatus status = vm_run(&vm);
    gettimeofday(&end, NULL);

    const Value ans = vm_result(&vm);

    puts("Result:");
    print_value(&ans, &vm);

    const int end_usec = end.tv_sec * 1000000 + end.tv_usec;
    const int begin_usec = begin.tv_sec * 1000000 + begin.tv_usec;
    printf("\nDONE in %d ms\n", abs(end_usec - begin_usec) / 1000);

    dispose_vm(&vm);
    program_del(&program);
    compiler_del(&compiler);
    charspan_del(&source_view);
    mystr_del(&source_str);

    return (status == vm_status_ok) ? 0 : 1;
}
