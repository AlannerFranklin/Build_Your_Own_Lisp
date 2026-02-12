#include "config.h"
#include <stdlib.h>
#include <string.h>

lenv* lenv_new(void) {
  lenv* e = malloc(sizeof(lenv));
  e->par = NULL;
  e->count = 0;
  e->syms = NULL;
  e->vals = NULL;
  return e;
}

void lenv_del(lenv* e) {
  for (int i = 0;i < e->count;i++) {
    free(e->syms[i]);
    lval_del(e->vals[i]);
  }
  free(e->syms);
  free(e->vals);
  free(e);
}

lenv* lenv_copy(lenv* e) {
  lenv* n = malloc(sizeof(lenv));
  n->par = e->par;
  n->count = e->count;
  n->syms = malloc(sizeof(char*) * n->count);
  n->vals = malloc(sizeof(lval*) * n->count);
  for (int i = 0; i < n->count; i++) {
    n->syms[i] = malloc(strlen(e->syms[i]) + 1);
    strcpy(n->syms[i], e->syms[i]);
    n->vals[i] = lval_copy(e->vals[i]);
  }
  return n;
}

lval* lenv_get(lenv* e, lval* k) {
  while(e) {
    for (int i = 0;i < e->count; i++) {
      if (strcmp(e->syms[i], k->sym) == 0) {
        return lval_copy(e->vals[i]);
      }
    }
    e = e->par;
  }
  return lval_err("Unbound Symbol '%s'", k->sym);
}

void lenv_put(lenv* e, lval* k, lval* v) {
  /* Iterate over all items in environment */
  /* This is to see if variable already exists */
  for (int i = 0;i < e->count;i++) {
    /* If variable is found delete item at that position */
    /* And replace with variable supplied by user */
    if (strcmp(e->syms[i], k->sym) == 0) {
      lval_del(e->vals[i]);
      e->vals[i] = lval_copy(v);
      return;
    }
  }
  /* If no existing entry found allocate space for new entry */
  e->count++;
  e->vals = realloc(e->vals, sizeof(lval*) * e->count);
  e->syms = realloc(e->syms, sizeof(char*) * e->count);

  /* Copy contents of lval and symbol string into new location */
  e->vals[e->count-1] = lval_copy(v);
  e->syms[e->count-1] = malloc(strlen(k->sym)+1);
  strcpy(e->syms[e->count-1], k->sym);
}

void lenv_add_builtin(lenv* e, char* name, lbuiltin func) {
  lval* k = lval_sym(name);
  lval* v = lval_fun(func);
  v->sym = malloc(strlen(name) + 1); // 存储函数名
  strcpy(v->sym, name);
  lenv_put(e, k, v);
  lval_del(k);
  lval_del(v);
}

void lenv_add_builtins(lenv* e) {
  /* List Functions */
  lenv_add_builtin(e, "list", builtin_list);
  lenv_add_builtin(e, "head", builtin_head);
  lenv_add_builtin(e, "tail", builtin_tail);
  lenv_add_builtin(e, "eval", builtin_eval);
  lenv_add_builtin(e, "join", builtin_join);
  lenv_add_builtin(e, "cons", builtin_cons);
  lenv_add_builtin(e, "len", builtin_len);
  lenv_add_builtin(e, "init", builtin_init);
  lenv_add_builtin(e, "def", builtin_def);
  lenv_add_builtin(e, "=",   builtin_put);

  /* Mathematical Functions */
  lenv_add_builtin(e, "+", builtin_add);
  lenv_add_builtin(e, "-", builtin_sub);
  lenv_add_builtin(e, "*", builtin_mul);
  lenv_add_builtin(e, "/", builtin_div);
  
  /* Variable Functions */
  lenv_add_builtin(e, "add", builtin_add);
  lenv_add_builtin(e, "sub", builtin_sub);
  lenv_add_builtin(e, "mul", builtin_mul);
  lenv_add_builtin(e, "div", builtin_div);

  /* Exit Function */
  lenv_add_builtin(e, "exit", builtin_exit);
  lenv_add_builtin(e, "printenv", builtin_printenv);
  
  /* Lambda Function */
  lenv_add_builtin(e, "\\", builtin_lambda);
  lenv_add_builtin(e, "fun", builtin_fun);

  /* Comparison Functions */
  lenv_add_builtin(e, "if", builtin_if);
  lenv_add_builtin(e, "==", builtin_eq);
  lenv_add_builtin(e, "!=", builtin_ne);
  lenv_add_builtin(e, ">",  builtin_gt);
  lenv_add_builtin(e, "<",  builtin_lt);
  lenv_add_builtin(e, ">=", builtin_ge);
  lenv_add_builtin(e, "<=", builtin_le);
  lenv_add_builtin(e, "or", builtin_or);
  lenv_add_builtin(e, "and", builtin_and);
  lenv_add_builtin(e, "not", builtin_not);
  lenv_add_builtin(e, "true", builtin_true);
  lenv_add_builtin(e, "false", builtin_false);

  /* String Functions */
  lenv_add_builtin(e, "load", builtin_load);
  lenv_add_builtin(e, "error", builtin_error);
  lenv_add_builtin(e, "print", builtin_print);
  lenv_add_builtin(e, "read", builtin_read);
  lenv_add_builtin(e, "show", builtin_show);

  /* File Functions */
  lenv_add_builtin(e, "fopen", builtin_fopen);
  lenv_add_builtin(e, "fclose", builtin_fclose);
  lenv_add_builtin(e, "fread", builtin_fread);
  lenv_add_builtin(e, "fwrite", builtin_fwrite);
  lenv_add_builtin(e, "fseek", builtin_fseek);
  lenv_add_builtin(e, "ftell", builtin_ftell);
  lenv_add_builtin(e, "rewind", builtin_rewind);
}

void lenv_def(lenv* e, lval* k, lval* v) {
    /* Iterate till e has no parent */
    while (e->par) { e = e->par; }
    /* Put value in e */
    lenv_put(e, k, v);
}