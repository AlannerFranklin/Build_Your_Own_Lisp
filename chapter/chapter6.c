#include <stdio.h>
#include <stdlib.h>

#include "mpc.h"

/* Linux/Mac 专用头文件 */
#include <editline/readline.h>
#include <editline/history.h>

int main(int argc, char** argv) {

  /* Create Some Parsers */
  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Expr = mpc_new("expr");
  mpc_parser_t* Lispy = mpc_new("lispy");

  mpca_lang(MPCA_LANG_DEFAULT,
    "                                                                         \
      number   : /-?[0-9]+\.[0-9]+/ ;                                         \
      operator : '+' | '-' | '*' | '/' | '%' | "add" | "sub" | "mul" | "div" ;\
      /* 注意：这里我们只支持两个操作数，且必须有括号，避免优先级问题 */
      expr : <number> | '(' <expr> <operator> <expr> ')' ;
      lispy : /^/ <expr> /$/ ;                                \
    ",
    Number, Operator, Expr, Lispy);

  mpca_lang(MPCA_LANG_DEFAULT,
  "                                            \
    adjective : \"wow\" | \"many\"             \
              | \"so\"  | \"such\";            \
    noun      : \"lisp\" | \"language\"        \
              | \"book\" | \"build\" | \"c\";  \
    phrase    : <adjective> <noun>;            \
    doge      : /^/ <phrase>* /$/;             \
  ",
  Adjective, Noun, Phrase, Doge);

  puts("Lispy Version 0.0.0.0.1");
  puts("Press Ctrl+c to Exit\n");

  char *input = readline("lispy> ");
  /* Attempt to Parse the user Input */
  mpc_result_t r;
  if (mpc_parse("<stdin>", input, Lispy, &r)){
    mpc_ast_print(r.output);
    mpc_ast_delete(r.output);
  } else {
    mpc_err_print(r.error);
    mpc_err_delete(r.error);
  }

  /* Undefine and Delete our Parsers */
  mpc_cleanup(4, Number, Operator, Expr, Lispy);
  return 0;
}