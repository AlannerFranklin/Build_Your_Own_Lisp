#include "mpc.h"
#include "config.h"
#include "pool.h"

/* Linux/Mac 专用头文件 */
#include <editline/readline.h>
#include <editline/history.h>

int number_of_leaves(mpc_ast_t* t) {
  if (t->children_num == 0) { return 1; }
  int total = 0;
  for (int i = 0; i < t->children_num; i++) {
    total += number_of_leaves(t->children[i]);
  }
  return total;
}

int number_of_branches(mpc_ast_t* t) {
  if (t->children_num == 0) { return 0; }
  int total = 1;
  for (int i = 0; i < t->children_num; i++) {
    total += number_of_branches(t->children[i]);
  }
  return total;
}

int max_children(mpc_ast_t* t) {
  if (t->children_num == 0) { return 0; }
  int max = t->children_num;
  for (int i = 0; i < t->children_num; i++) {
    int child_max = max_children(t->children[i]);
    if (child_max > max) { max = child_max; }
  }
  return max;
}

/*
mpc_parser_t* Number;
mpc_parser_t* Symbol;
mpc_parser_t* String;
mpc_parser_t* Comment;
mpc_parser_t* Sexpr;
mpc_parser_t* Qexpr;
mpc_parser_t* Expr;
mpc_parser_t* Lispy;
*/

int main(int argc, char** argv) {

  /* Create Some Parsers */
  #if 0
  Number = mpc_new("number");
  Symbol = mpc_new("symbol");
  String = mpc_new("string");
  Comment = mpc_new("comment");
  Sexpr = mpc_new("sexpr");
  Qexpr = mpc_new("qexpr");
  Expr = mpc_new("expr");
  Lispy = mpc_new("lispy");

  mpca_lang(MPCA_LANG_DEFAULT,
    "                                                                             \
      number   : /-?[0-9]+(\\.[0-9]+)?/;                                          \
      symbol   : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;                               \
      string   : /\"(\\\\.|[^\"])*\"/ ;                                           \
      comment : /;[^\\r\\n]*/ ;                                                   \
      sexpr    : '(' <expr>* ')' ;                                                \
      qexpr    : '{' <expr>* '}' ;                                                \
      expr     : <number> | <symbol> | <string> | <sexpr> | <qexpr> | <comment> ; \
      lispy    : /^/ <expr>* /$/ ;                                                \
    ",
    Number, Symbol, String, Comment, Sexpr, Qexpr, Expr, Lispy);
  #endif

  lval_pool_init();

  lenv* e = lenv_new();
  lenv_add_builtins(e);

  /* Load Standard Library */
  lval* args = lval_add(lval_sexpr(), lval_str("chapter/prelude.lspy"));
  lval* x = builtin_load(e, args);
  if (x->type == LVAL_ERR) { lval_println(x); }
  lval_del(x);

  if (argc >= 2) {
    /* loop over each supplied filename (starting from 1) */
    for (int i = 1; i < argc; i++) {
      lval* args = lval_add(lval_sexpr(), lval_str(argv[i]));
      lval* x = builtin_load(e, args);
      if (x->type == LVAL_ERR) { lval_println(x); }
      lval_del(x);
    }
  } else {

    puts("Lispy Version 0.0.0.0.1");
    puts("Press Ctrl+c to Exit\n");

    while (1) {
      char *input = readline("lispy> ");
      if (!input) break;
      add_history(input);
      lval* x = lval_parse(input);
      if (x->type == LVAL_SEXPR) {
        /* 如果是 S-Expression (列表)，我们认为它包含多个顶层表达式 */
        /* 我们依次弹出并求值 */
        while (x->count > 0) {
          lval* result = lval_eval(e, lval_pop(x, 0));
          lval_println(result);
          lval_del(result);
        }
        lval_del(x);
      } else {
        lval_println(x);
        lval_del(x);
      }
      free(input);
      lval_pool_dump_log("memory.log");

      /* Attempt to Parse the user Input */
      #if 0
      mpc_result_t r;
      if (mpc_parse("<stdin>", input, Lispy, &r)){
        /* Step 1: Read (Chapter 9 Goal) */
        lval* x = lval_read(r.output);

        if (x->type != LVAL_ERR) {
          lval* result = lval_eval(e, x);
          lval_println(result);
          
          lval_del(result);
        } else {
          lval_println(x);
          lval_del(x);
        }
        
        /* Step 2: Evaluate (Chapter 10 Goal) */
        lval* result = lval_eval(e, x);
        lval_println(result);
        
        lval_del(result);
        mpc_ast_delete(r.output);
        } else {
          mpc_err_print(r.error);
          mpc_err_delete(r.error);
        }

        free(input);
        lval_pool_dump_log("memory.log");
      }
      #endif
    }
    lenv_del(e);
    /* Undefine and Delete our Parsers */
    //mpc_cleanup(8, Number, Symbol, String, Comment, Qexpr, Sexpr, Expr, Lispy);
    lval_pool_dump_log("memory.log");
    lval_pool_cleanup();
    return 0;
}
}