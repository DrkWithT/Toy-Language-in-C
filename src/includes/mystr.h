#ifndef TOYSCRIPT_MYSTR_H
#define TOYSCRIPT_MYSTR_H

#include <stdint.h>
#include <stddef.h>

static const size_t alisp_mystr_default_capacity_v = 16;

//? NOTE: Forward declaration of mystr:
typedef struct alisp_mystr_t mystr;

typedef struct alisp_mycharspan_t {
    const char* data;
    size_t length;
} charspan;

void charspan_dud(charspan *self);
void charspan_new(charspan *self, const char *s, size_t n);
void charspan_from_str(charspan *self, const mystr *s);
void charspan_copy(charspan *self, const charspan *other);
void charspan_del(charspan *self);

int8_t charspan_empty(const charspan *self);
size_t charspan_len(const charspan *self);
int8_t charspan_equals_raw(const charspan *self, const char *s, size_t n);
int8_t charspan_equals_charspan(const charspan *self, const charspan *other);



typedef struct alisp_mystr_t {
    char *data;
    size_t length;
    size_t capacity;
} mystr;

void mystr_dud(mystr *self);
void mystr_res(mystr *self, size_t n);
void mystr_new(mystr *self, const char *s);
mystr* mystr_copy(mystr *self, const mystr *other);
mystr* mystr_move(mystr *self, mystr *other);
void mystr_del(mystr *self);

const char *mystr_raw(const mystr *self);

int8_t mystr_empty(const mystr *self);
size_t mystr_len(const mystr *self);
int8_t mystr_append_raw(mystr *self, const char *s, size_t n);
int8_t mystr_append_charspan(mystr *self, const charspan *span, size_t n);
int8_t mystr_append_mystr(mystr *self, const mystr *other);

int8_t mystr_equals_raw(const mystr *self, const char *s, size_t n);
int8_t mystr_equals_charspan(const mystr *self, const charspan *span);
int8_t mystr_equals_mystr(const mystr *self, const mystr *other);

#endif
