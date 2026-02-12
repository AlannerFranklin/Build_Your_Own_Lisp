#ifndef POOL_H
#define POOL_H

/* Forward declaration to break dependency cycle */
struct lval;
typedef struct lval lval;

/* Initialize the memory pool (optional) */
void lval_pool_init(void);

/* Print pool statistics for verification */
void lval_pool_print_stats(void);

/* Allocate an lval from the pool (replaces malloc(sizeof(lval))) */
lval* lval_alloc(void);

/* Return an lval to the pool (replaces free(v) for the struct only) */
void lval_release(lval* v);

/* Cleanup all memory in the pool (call at program exit) */
void lval_pool_cleanup(void);

/* Dump memory pool statistics to a log file */
void lval_pool_dump_log(const char* filename);

#endif
