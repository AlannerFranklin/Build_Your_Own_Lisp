#include "config.h"

void vec_push(lval_vec* v, lval* x) {
  if (v->count >= v->capacity) {
    v->capacity = v->capacity ? v->capacity * 2 : 1024;
    v->items = realloc(v->items, sizeof(lval*) * v->capacity);
  }
  v->items[v->count++] = x;
}

void vec_free(lval_vec* v) {
  free(v->items);
}