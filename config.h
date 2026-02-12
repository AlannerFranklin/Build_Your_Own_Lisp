#ifndef CONFIG_H
#define CONFIG_H

#include "mpc.h"

/* Forward Declarations */
struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;

#include "pool.h"

/* Parser Declarations */
extern mpc_parser_t* Lispy;

/* Function Pointer Type */
typedef lval*(*lbuiltin)(lenv*, lval*);

/* Enum of lval types */
enum { LVAL_NUM, LVAL_DEC, LVAL_ERR, LVAL_SYM, LVAL_STR,
        LVAL_SEXPR, LVAL_QEXPR, LVAL_FUN, LVAL_FILE };

/*dynamic array*/
typedef struct {
  lval** items;
  int count;
  int capacity;
} lval_vec;


/* FILE Struct */
typedef struct {
  FILE* file;
  int ref_count;
  char* mode;
} lval_file_t;


/* lval Struct */
struct lval {
  int type;

  /* Basic */
  long num;
  double dec;
  char* err;
  char* sym;
  char* str;

  /* Function */
  lbuiltin builtin;
  lenv* env;
  lval* formals;//形参
  lval* body;

  /* Expression */
  int count;
  lval** cell;

  /* 使用共享的文件结构体指针 */
  lval_file_t* file_rc;
};

/* lenv Struct */
struct lenv {
  lenv* par;
  int count;
  char** syms;
  lval** vals;
};

/* --- Function Declarations --- */

/* Constructors */
lval* lval_num(long x);
lval* lval_dec(double x);
lval* lval_err(char* fmt, ...);
lval* lval_sym(char* s);
lval* lval_sexpr(void);
lval* lval_qexpr(void);
lval* lval_fun(lbuiltin func);
int lval_eq(lval* x, lval* y);
char* lval_str_unescape(char* s);
char* lval_str_escape(char* s);

/* Destructor */
void lval_del(lval* v);

/* Copy */
lval* lval_copy(lval* v);

/* Operations */
lval* lval_add(lval* v, lval* x);
lval* lval_offer(lval* v, lval* x);
lval* lval_pop(lval* v, int i);
lval* lval_take(lval* v, int i);
lval* lval_join(lval* x, lval* y);
lval* lval_lambda(lval* formals, lval* body);
//lval* lval_call(lenv* e, lval* f, lval* a);
lval* lval_str(char* s);


/* Reading & Printing */
lval* lval_read(mpc_ast_t* t);
void lval_print(lval* v);
void lval_println(lval* v);
void lval_expr_print(lval* v, char open, char close);
char* ltype_name(int t);
void lval_print_str(lval* v);
lval* lval_read_str(mpc_ast_t* t);

/* Evaluation */
lval* lval_eval(lenv* e, lval* v);
//lval* lval_eval_sexpr(lenv* e, lval* v);

/* Environment Functions */
lenv* lenv_new(void);
void lenv_del(lenv* e);
lval* lenv_get(lenv* e, lval* k);
void lenv_put(lenv* e, lval* k, lval* v);
void lenv_add_builtin(lenv* e, char* name, lbuiltin func);
void lenv_add_builtins(lenv* e);
lenv* lenv_copy(lenv* e);
void lenv_def(lenv* e, lval* k, lval* v);

/* Builtin Functions */
lval* builtin_list(lenv* e, lval* a);
lval* builtin_head(lenv* e, lval* a);
lval* builtin_tail(lenv* e, lval* a);
lval* builtin_join(lenv* e, lval* a);
lval* builtin_eval(lenv* e, lval* a);
lval* builtin_len(lenv* e, lval* a);
lval* builtin_cons(lenv* e, lval* a);
lval* builtin_init(lenv* e, lval* a);
lval* builtin_def(lenv* e, lval* a);
lval* builtin_add(lenv* e, lval* a);
lval* builtin_sub(lenv* e, lval* a);
lval* builtin_mul(lenv* e, lval* a);
lval* builtin_div(lenv* e, lval* a);
lval* builtin_exit(lenv* e, lval* a);
lval* builtin_printenv(lenv* e, lval* a);
lval* builtin_fun(lenv* e, lval* a);
lval* builtin_op(lenv* e, lval* a, char* op);
lval* builtin_lambda(lenv* e, lval* a);
lval* builtin_put(lenv* e, lval* a);
lval* builtin_def(lenv* e, lval* a);
lval* builtin_gt(lenv* e, lval* a);
lval* builtin_lt(lenv* e, lval* a);
lval* builtin_ge(lenv* e, lval* a);
lval* builtin_le(lenv* e, lval* a);
lval* builtin_ord(lenv* e, lval* a, char* op);
lval* builtin_cmp(lenv* e, lval* a, char* op);
lval* builtin_eq(lenv* e, lval* a);
lval* builtin_ne(lenv* e, lval* a);
lval* builtin_if(lenv* e, lval* a);
lval* builtin_or(lenv* e, lval* a);
lval* builtin_and(lenv* e, lval* a);
lval* builtin_not(lenv* e, lval* a);
lval* builtin_true(lenv* e, lval* a);
lval* builtin_false(lenv* e, lval* a);
lval* builtin_load(lenv* e, lval* a);
lval* builtin_error(lenv* e, lval* a);
lval* builtin_print(lenv* e, lval* a);
lval* builtin_read(lenv* e, lval* a);
lval* builtin_show(lenv* e, lval* a);

/* File Functions */
lval* builtin_fopen(lenv* e, lval* a);
lval* builtin_fclose(lenv* e, lval* a);
lval* builtin_fread(lenv* e, lval* a);
lval* builtin_fwrite(lenv* e, lval* a);
lval* builtin_fseek(lenv* e, lval* a);
lval* builtin_ftell(lenv* e, lval* a);
lval* builtin_rewind(lenv* e, lval* a);

/*FILE FUNCTIONS */
lval* lval_file(char* mode);
lval* builtin_fopen(lenv* e, lval* a);
lval* builtin_fclose(lenv* e, lval* a);
lval* builtin_fread(lenv* e, lval* a);
lval* builtin_fwrite(lenv* e, lval* a);
lval* builtin_fseek(lenv* e, lval* a);
lval* builtin_ftell(lenv* e, lval* a);
lval* builtin_rewind(lenv* e, lval* a);

/* Parser Declaration */
lval* lval_parse(char* input);

/*dynamic array function*/
void vec_push(lval_vec* v, lval* x);
void vec_free(lval_vec* v);

/*parser*/
typedef enum {
  TOK_LPAREN,   // (
  TOK_RPAREN,   // )
  TOK_LBRACE,   // {
  TOK_RBRACE,   // }
  TOK_SYM,      // 符号: +, add, x
  TOK_NUM,      // 数字: 123, 4.56
  TOK_STR,      // 字符串: "hello"
  TOK_EOF,      // 结束符
  TOK_ERR       // 词法错误
} TokenType;

/* Token 结构体 */
typedef struct {
  TokenType type;
  char* start;  // Token 在源码中的起始位置指针
  int length;   // Token 的长度
  char* error;  // 如果是错误，存储错误信息
} Token;

/* Tokenizer 状态机 */
typedef struct {
  char* src;    // 源代码字符串
  int pos;      // 当前解析到的位置索引
  int len;      // 源代码总长度
} Tokenizer;

/*parser function*/
Tokenizer tokenizer_new(char* src);
char peek(Tokenizer* t);
char advance(Tokenizer* t);

int is_sym_char(char c);
Token next_token(Tokenizer* t);
lval* parse_sexpr(Tokenizer* t);
lval* parse_qexpr(Tokenizer* t);
lval* parse_atom(Token tok);
lval* parse_expr_list(Tokenizer* t, TokenType end_type);
lval* lval_parse(char* input);


#endif
