#include "config.h"
#include "error.h"

lval* builtin_len(lenv* e, lval* a) {
  LASSERT_NUM("len", a, 1);
  LASSERT_TYPE("len", a, 0, LVAL_QEXPR);
  lval* x = lval_take(a, 0);
  long count = x->count;
  lval_del(x);
  return lval_num(count);
}

lval* builtin_cons(lenv* e, lval* a) {
  LASSERT_NUM("cons", a, 2);
  LASSERT_TYPE("cons", a, 1, LVAL_QEXPR);
  lval* v = lval_pop(a, 0);
  lval* q = lval_pop(a, 0);
  lval_del(a);
  lval_offer(q, v);
  return q;
}

lval* builtin_init(lenv* e, lval* a) {
  LASSERT_NUM("init", a, 1);
  LASSERT_TYPE("init", a, 0, LVAL_QEXPR);
  LASSERT_NOT_EMPTY("init", a, 0);
  lval* q = lval_take(a, 0);
  lval_del(lval_pop(q, q->count-1));
  return q;
}

lval* builtin_head(lenv* e, lval* a) {
  LASSERT_NUM("head", a, 1);
  LASSERT(a, a->cell[0]->type == LVAL_QEXPR || a->cell[0]->type == LVAL_STR,
    "Function 'head' passed incorrect type for argument 0. Got %s, Expected %s or %s.",
    ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR), ltype_name(LVAL_STR));

  if (a->cell[0]->type == LVAL_QEXPR) {
      LASSERT_NOT_EMPTY("head", a, 0);
      lval* v = lval_take(a, 0);
      while (v->count > 1) { lval_del(lval_pop(v, 1)); }
      return v;
  }
  
  if (a->cell[0]->type == LVAL_STR) {
      lval* v = lval_take(a, 0);
      LASSERT(a, strlen(v->str) > 0, "Function 'head' passed empty string!");
      char* s = malloc(2);
      s[0] = v->str[0];
      s[1] = '\0';
      lval_del(v);
      return lval_str(s);
  }

  return NULL; // Should be unreachable
}

lval* builtin_tail(lenv* e, lval* a) {
  LASSERT_NUM("tail", a, 1);
  LASSERT(a, a->cell[0]->type == LVAL_QEXPR || a->cell[0]->type == LVAL_STR,
    "Function 'tail' passed incorrect type for argument 0. Got %s, Expected %s or %s.",
    ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR), ltype_name(LVAL_STR));

  if (a->cell[0]->type == LVAL_QEXPR) {
      LASSERT_NOT_EMPTY("tail", a, 0);
      lval* v = lval_take(a, 0);
      lval_del(lval_pop(v, 0));
      return v;
  }
  
  if (a->cell[0]->type == LVAL_STR) {
      lval* v = lval_take(a, 0);
      LASSERT(a, strlen(v->str) > 0, "Function 'tail' passed empty string!");
      lval* x = lval_str(v->str + 1);
      lval_del(v);
      return x;
  }

  return NULL; // Should be unreachable
}

lval* builtin_list(lenv* e, lval* a) {
  a->type = LVAL_QEXPR;
  return a;
}

lval* builtin_eval(lenv* e, lval* a) {
  LASSERT_NUM("eval", a, 1);
  LASSERT_TYPE("eval", a, 0, LVAL_QEXPR);
  lval* x = lval_take(a, 0);
  x->type = LVAL_SEXPR;
  return lval_eval(e, x);
}

lval* builtin_op(lenv* e, lval* a, char* op) {
  
  /* Ensure all arguments are numbers */
  for (int i = 0; i < a->count; i++) {
    if (a->cell[i]->type != LVAL_NUM && a->cell[i]->type != LVAL_DEC) {
      LASSERT(a, 0, "Function '%s' passed incorrect type for argument %i. Got %s, Expected %s.",
        op, i, ltype_name(a->cell[i]->type), ltype_name(LVAL_NUM));
    }
  }
  
  /* Pop the first element */
  if (a->count == 0) {
    lval_del(a);
    return lval_err("Function '%s' passed too few arguments!", op);
  }
  lval* x = lval_pop(a, 0);

  /* If no arguments and sub then perform unary negation */
  if ((strcmp(op, "-") == 0 || strcmp(op, "sub") == 0) && a->count == 0) {
     if (x->type == LVAL_NUM) { x->num = -x->num; }
     if (x->type == LVAL_DEC) { x->dec = -x->dec; }
  }

  /* While there are still elements remaining */
  while (a->count > 0) {

    /* Pop the next element */
    lval* y = lval_pop(a, 0);

    /* Perform operation */
    if (x->type == LVAL_DEC || y->type == LVAL_DEC) {
        /* Cast to double if one is double */
        double x_val = (x->type == LVAL_NUM) ? (double)x->num : x->dec;
        double y_val = (y->type == LVAL_NUM) ? (double)y->num : y->dec;
        
        /* Upgrade x to decimal */
        x->type = LVAL_DEC;

        if (strcmp(op, "+") == 0 || strcmp(op, "add") == 0) { x->dec = x_val + y_val; }
        if (strcmp(op, "-") == 0 || strcmp(op, "sub") == 0) { x->dec = x_val - y_val; }
        if (strcmp(op, "*") == 0 || strcmp(op, "mul") == 0) { x->dec = x_val * y_val; }
        if (strcmp(op, "/") == 0 || strcmp(op, "div") == 0) {
          if (y_val == 0) {
            lval_del(x); lval_del(y);
            x = lval_err("Division By Zero!"); break;
          }
          x->dec = x_val / y_val;
        }
        if (strcmp(op, "%") == 0 || strcmp(op, "mod") == 0) {
             lval_del(x); lval_del(y);
             x = lval_err("Modulo not supported for decimals!"); break;
        }
    } else {
        /* Standard Integer Arithmetic */
        if (strcmp(op, "+") == 0 || strcmp(op, "add") == 0) { x->num += y->num; }
        if (strcmp(op, "-") == 0 || strcmp(op, "sub") == 0) { x->num -= y->num; }
        if (strcmp(op, "*") == 0 || strcmp(op, "mul") == 0) { x->num *= y->num; }
        if (strcmp(op, "/") == 0 || strcmp(op, "div") == 0) {
          if (y->num == 0) {
            lval_del(x); lval_del(y);
            x = lval_err("Division By Zero!"); break;
          }
          x->num /= y->num;
        }
        if (strcmp(op, "%") == 0 || strcmp(op, "mod") == 0) {
           if (y->num == 0) {
            lval_del(x); lval_del(y);
            x = lval_err("Division By Zero!"); break;
          }
          x->num %= y->num;
        }
    }
    
    lval_del(y);
  }

  lval_del(a);
  return x;
}

lval* builtin_join(lenv* e, lval* a) {
  for (int i = 0; i < a->count; i++) {
    LASSERT(a, a->cell[i]->type == LVAL_QEXPR || a->cell[i]->type == LVAL_STR,
      "Function 'join' passed incorrect type for argument %i. Got %s, Expected %s or %s.",
      i, ltype_name(a->cell[i]->type), ltype_name(LVAL_QEXPR), ltype_name(LVAL_STR));
  }

  lval* x = lval_pop(a, 0);

  if (x->type == LVAL_QEXPR) {
      while (a->count) {
        lval* y = lval_pop(a, 0);
        LASSERT(a, y->type == LVAL_QEXPR, "Function 'join' passed mixed types!");
        x = lval_join(x, y);
      }
  }
  
  if (x->type == LVAL_STR) {
      while (a->count) {
        lval* y = lval_pop(a, 0);
        LASSERT(a, y->type == LVAL_STR, "Function 'join' passed mixed types!");
        
        char* s = malloc(strlen(x->str) + strlen(y->str) + 1);
        strcpy(s, x->str);
        strcat(s, y->str);
        
        free(x->str); x->str = s;
        lval_del(y);
      }
  }

  lval_del(a);
  return x;
}

lval* builtin_var(lenv* e, lval* a, char* func) {
    LASSERT_TYPE(func, a, 0, LVAL_QEXPR);

    lval* syms = a->cell[0];
    for (int i = 0;i < syms->count; i++) {
        LASSERT(a, syms->cell[i]->type == LVAL_SYM,
            "Function '%s' cannot define non-symbol. "
            "Got %s, Expected %s.", func,
            ltype_name(syms->cell[i]->type),
            ltype_name(LVAL_SYM));
    }

    LASSERT(a, (syms->count == a->count-1),
        "Function '%s' passed too many arguments for symbols. "
        "Got %i, Expected %i.", func, syms->count, a->count-1);

    for (int i = 0; i < syms->count; i++) {
        /* If 'def' define in globally. If 'put' define in locally */
        if (strcmp(func, "def") == 0) {
            lenv_def(e, syms->cell[i], a->cell[i+1]);
        }
        
        if (strcmp(func, "=") == 0) {
            lenv_put(e, syms->cell[i], a->cell[i+1]);
        } 
    }
    
    lval_del(a);
    return lval_sexpr();
}

lval* builtin_def(lenv* e, lval* a) {
  return builtin_var(e, a, "def");
}

lval* builtin_put(lenv* e, lval* a) {
  return builtin_var(e, a, "=");
}

lval* builtin_add(lenv* e, lval* a) {
  return builtin_op(e, a, "+");
}

lval* builtin_sub(lenv* e, lval* a) {
  return builtin_op(e, a, "-");
}

lval* builtin_mul(lenv* e, lval* a) {
  return builtin_op(e, a, "*");
}

lval* builtin_div(lenv* e, lval* a) {
  return builtin_op(e, a, "/");
}

lval* builtin_exit(lenv* e, lval* a) {
  lval_del(a);
  exit(0);
}

lval* builtin_printenv(lenv* e, lval* a) {
  lval_del(a);
  for (int i = 0; i < e->count; i++) {
    printf("%-10s : ", e->syms[i]);
    lval_println(e->vals[i]);
  }
  return lval_sexpr();
}

lval* builtin_lambda(lenv* e, lval* a) {
    /* Check Two arguments, each of which are Q-Expressions */
    LASSERT_NUM("lambda", a, 2);
    LASSERT_TYPE("lambda", a, 0, LVAL_QEXPR);
    LASSERT_TYPE("lambda", a, 1, LVAL_QEXPR);

    /* Check first Q-Expression contains only Symbols */
    for (int i = 0;i < a->cell[0]->count; i++) {
        LASSERT(a, (a->cell[0]->cell[i]->type == LVAL_SYM),
        "Cannot define non-symbol. Got %s, Expected %s.",
        ltype_name(a->cell[0]->cell[i]->type), ltype_name(LVAL_SYM));
    }

    /* Pop first two arguments and pass them to lval_lambda */
    lval* formals = lval_pop(a, 0);
    lval* body = lval_pop(a, 0);
    lval_del(a);

    return lval_lambda(formals, body);
}

lval* builtin_fun(lenv* e, lval* a) {
    LASSERT_NUM("fun", a, 2);
    LASSERT_TYPE("fun", a, 0, LVAL_QEXPR);
    LASSERT_TYPE("fun", a, 1, LVAL_QEXPR);

    /* Check first argument is a list of symbols */
    lval* syms = a->cell[0];
    LASSERT_NOT_EMPTY("fun", a, 0);

    for (int i = 0; i < syms->count; i++) {
        LASSERT(a, syms->cell[i]->type == LVAL_SYM,
            "Function 'fun' cannot define non-symbol. Got %s, Expected %s.",
            ltype_name(syms->cell[i]->type), ltype_name(LVAL_SYM));
    }

    /* Pop arguments */
    lval* args = lval_pop(a, 0);
    lval* body = lval_pop(a, 0);
    lval_del(a);

    /* Get function name (first symbol) */
    lval* name = lval_pop(args, 0);

    /* The rest are formal arguments */
    lval* formals = args;

    /* Create lambda function */
    lval* fun = lval_lambda(formals, body);

    /* Define in environment */
    lenv_def(e, name, fun);
    
    lval_del(name);
    return lval_sexpr();
}

lval* builtin_gt(lenv* e, lval* a) {
  return builtin_ord(e, a, ">");
}

lval* builtin_lt(lenv* e, lval* a) {
  return builtin_ord(e, a, "<");
}

lval* builtin_ge(lenv* e, lval* a) {
  return builtin_ord(e, a, ">=");
}

lval* builtin_le(lenv* e, lval* a) {
  return builtin_ord(e, a, "<=");
}

lval* builtin_ord(lenv* e, lval* a, char* op) {
  LASSERT_NUM("ord", a, 2);
  LASSERT_TYPE("ord", a, 0, LVAL_NUM);
  LASSERT_TYPE("ord", a, 1, LVAL_NUM);

  int r;
  if (strcmp(op, ">") == 0) {
    r = a->cell[0]->num > a->cell[1]->num;
  }
  if (strcmp(op, "<") == 0) {
    r = a->cell[0]->num < a->cell[1]->num;
  }
  if (strcmp(op, ">=") == 0) {
    r = a->cell[0]->num >= a->cell[1]->num;
  }
  if (strcmp(op, "<=") == 0) {
    r = a->cell[0]->num <= a->cell[1]->num;
  }
  lval_del(a);
  return lval_num(r);
}

lval* builtin_cmp(lenv* e, lval* a, char *op) {
  LASSERT_NUM(op, a, 2);
  int r;
  if (strcmp(op, "==") == 0) {
    r = lval_eq(a->cell[0], a->cell[1]);
  }
  if (strcmp(op, "!=") == 0) {
    r = !lval_eq(a->cell[0], a->cell[1]);
  }
  lval_del(a);
  return lval_num(r);
}

lval* builtin_eq(lenv* e, lval* a) {
  return builtin_cmp(e, a, "==");
}

lval* builtin_ne(lenv* e, lval* a) {
  return builtin_cmp(e, a, "!=");
}

lval* builtin_if(lenv* e, lval* a) {
  LASSERT_NUM("if", a, 3);
  LASSERT_TYPE("if", a, 0, LVAL_NUM);
  LASSERT_TYPE("if", a, 1, LVAL_QEXPR);
  LASSERT_TYPE("if", a, 2, LVAL_QEXPR);

  /* Mark Both Expressions as evaluable */
  lval* x;
  a->cell[1]->type = LVAL_SEXPR;
  a->cell[2]->type = LVAL_SEXPR;

  if (a->cell[0]->num) {
    /* If condition is true evaluate first expression */
    x = lval_eval(e, lval_pop(a, 1));
  } else {
    /* Otherwise evaluate second expression */
    x = lval_eval(e, lval_pop(a, 2));
  }

  lval_del(a);
  return x;
}

int lval_is_true(lval* v) {
  if (v->type == LVAL_NUM) {
    return v->num != 0;
  }
  if (v->type == LVAL_QEXPR || v->type == LVAL_SEXPR) {
    return v->count != 0;
  }
  if (v->type == LVAL_ERR) { return 0; }
  return 1;
}

lval* builtin_or(lenv* e, lval* a) {
  LASSERT_NUM("or", a, 2);
  int r = lval_is_true(a->cell[0]) || lval_is_true(a->cell[1]);
  lval_del(a);
  return lval_num(r);
}

lval* builtin_and(lenv* e, lval* a) {
  LASSERT_NUM("and", a, 2);
  int r = lval_is_true(a->cell[0]) && lval_is_true(a->cell[1]);
  lval_del(a);
  return lval_num(r);
}

lval* builtin_not(lenv* e, lval* a) {
  LASSERT_NUM("not", a, 1);
  int r = !lval_is_true(a->cell[0]);
  lval_del(a);
  return lval_num(r);
}

lval* builtin_true(lenv* e, lval* a) {
  LASSERT_NUM("true", a, 0);
  return lval_num(1);
}

lval* builtin_false(lenv* e, lval* a) {
  LASSERT_NUM("false", a, 0);
  return lval_num(0);
}

lval* builtin_load(lenv* e, lval * a) {
  LASSERT_NUM("load", a, 1);
  LASSERT_TYPE("load", a, 0, LVAL_STR);

  /* Parse File given by string name */
  //mpc_result_t r;
  /* 1. 打开文件 */
  char* filename = a->cell[0]->str;
  FILE* f = fopen(filename, "r");
  if (f == NULL) {
    lval* err = lval_err("Could not open file %s", filename);
    lval_del(a);
    return err;
  }
  /* 2. 读取整个文件到字符串缓冲区 */
  fseek(f, 0, SEEK_END);
  long length = ftell(f);
  fseek(f, 0, SEEK_SET);
  char* buffer = malloc(length + 1);
  if (!buffer) {
      fclose(f);
      lval_del(a);
      return lval_err("Memory allocation failed for file %s", filename);
  }

  fread(buffer, 1, length, f);

  buffer[length] = '\0';
  fclose(f);

  /* 3. 使用 lval_parse 解析内容 */
  lval* expr = lval_parse(buffer);
  free(buffer); // 解析完就可以释放原始字符串了
  lval_del(a);  // 释放参数 a

  if (expr->type == LVAL_ERR) {
    return expr;
  }

  /* 5. 依次求值 (expr 是一个包含所有表达式的 S-Expr) */
  while (expr->count) {
    lval* x = lval_eval(e, lval_pop(expr, 0));
    if (x->type == LVAL_ERR) { lval_println(x); }
    lval_del(x);
  }
  lval_del(expr);
  return lval_sym("ok");

  #if 0
  if (mpc_parse_contents(a->cell[0]->str, Lispy, &r)) {

    /* Read contents */
    lval* expr = lval_read(r.output);
    mpc_ast_delete(r.output);

    /* Evaluate each Expression */
    while (expr->count) {
      lval* x = lval_eval(e, lval_pop(expr, 0));
      /* If Evaluation leads to error print it */
      if (x->type == LVAL_ERR) { lval_println(x); }
      lval_del(x);
    }

    /* Delete expressions and arguments */
    lval_del(expr);
    lval_del(a);

    /* Return ok */
    return lval_sym("ok");
  } else {
    /* Get Parse Error as String */
    char* err_msg = mpc_err_string(r.error);
    mpc_err_delete(r.error);

    /* Create new error message using it */
    lval* err = lval_err("Could not load Library %s", err_msg);
    free(err_msg);
    lval_del(a);

    /* Cleanup and return error */
    return err;
  }
  #endif
}

#include "config.h"
#include "error.h"

lval* builtin_print(lenv* e, lval* a) {
  
  /* Print each argument followed by a space */
  for (int i = 0; i < a->count; i++) {
    lval_print(a->cell[i]); putchar(' ');
  }
  
  /* Print a newline and delete arguments */
  putchar('\n');
  lval_del(a);
  
  return lval_sexpr();
}

lval* builtin_show(lenv* e, lval* a) {
  
  /* Print each argument followed by a space */
  for (int i = 0; i < a->count; i++) {
    lval_print_str(a->cell[i]); putchar(' ');
  }
  
  /* Print a newline and delete arguments */
  putchar('\n');
  lval_del(a);
  
  return lval_sexpr();
}

lval* builtin_error(lenv* e, lval* a) {
  LASSERT_NUM("error", a, 1);
  LASSERT_TYPE("error", a, 0, LVAL_STR);
  
  /* Construct Error from first argument */
  lval* err = lval_err(a->cell[0]->str);
  
  /* Delete arguments and return */
  lval_del(a);
  return err;
}

lval* builtin_read(lenv* e, lval* a) {
  LASSERT_NUM("read", a, 1);
  LASSERT_TYPE("read", a, 0, LVAL_STR);
  
  /* Parse String as if it were a file */

  lval* x = lval_parse(a->cell[0]->str);
  if (x->type != LVAL_ERR) { 
    x->type = LVAL_QEXPR; // Return as Q-Expression
  }
  lval_del(a);
  return x;

  
  #if 0
  mpc_result_t r;
  if (mpc_parse("<read>", a->cell[0]->str, Lispy, &r)) {
    
    /* Read contents */
    lval* x = lval_read(r.output);
    x->type = LVAL_QEXPR; // Return as Q-Expression
    
    mpc_ast_delete(r.output);
    lval_del(a);
    return x;
  } else {
    /* Get Parse Error as String */
    char* err_msg = mpc_err_string(r.error);
    mpc_err_delete(r.error);
    
    /* Create new error message using it */
    lval* err = lval_err("Could not read String %s", err_msg);
    free(err_msg);
    lval_del(a);
    
    return err;
  }
  #endif
}