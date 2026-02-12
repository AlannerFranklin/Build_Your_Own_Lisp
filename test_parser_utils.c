#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* 
   test_parser_utils.c
   Mock implementations of lval functions for testing parser.c independently.
   This file replaces lval.c to avoid linking against mpc dependencies.
*/

#include "pool.h"

lval* lval_num(long x) {
    lval* v = lval_alloc();
    v->type = LVAL_NUM;
    v->num = x;
    v->count = 0;
    v->cell = NULL;
    v->file_rc = NULL;
    return v;
}

lval* lval_err(char* fmt, ...) {
    lval* v = lval_alloc();
    v->type = LVAL_ERR;
    v->count = 0;
    v->cell = NULL;
    v->file_rc = NULL;
    
    va_list va;
    va_start(va, fmt);
    v->err = malloc(512);
    vsnprintf(v->err, 511, fmt, va);
    v->err[511] = '\0';
    va_end(va);
    
    return v;
}

lval* lval_sym(char* s) {
    lval* v = lval_alloc();
    v->type = LVAL_SYM;
    v->sym = malloc(strlen(s) + 1);
    strcpy(v->sym, s);
    v->count = 0;
    v->cell = NULL;
    v->file_rc = NULL;
    return v;
}

lval* lval_str(char* s) {
    lval* v = lval_alloc();
    v->type = LVAL_STR;
    v->str = malloc(strlen(s) + 1);
    strcpy(v->str, s);
    v->count = 0;
    v->cell = NULL;
    v->file_rc = NULL;
    return v;
}

lval* lval_sexpr(void) {
    lval* v = lval_alloc();
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    v->file_rc = NULL;
    return v;
}

lval* lval_qexpr(void) {
    lval* v = lval_alloc();
    v->type = LVAL_QEXPR;
    v->count = 0;
    v->cell = NULL;
    v->file_rc = NULL;
    return v;
}

void lval_del(lval* v) {
    if (!v) return;
    switch (v->type) {
        case LVAL_NUM: break;
        case LVAL_ERR: free(v->err); break;
        case LVAL_SYM: free(v->sym); break;
        case LVAL_STR: free(v->str); break;
        case LVAL_QEXPR:
        case LVAL_SEXPR:
            for (int i = 0; i < v->count; i++) {
                lval_del(v->cell[i]);
            }
            free(v->cell);
            break;
        /* Ignoring FILE, FUN etc for this parser test */
    }
    lval_release(v);
}

lval* lval_add(lval* v, lval* x) {
    v->count++;
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    v->cell[v->count-1] = x;
    return v;
}
