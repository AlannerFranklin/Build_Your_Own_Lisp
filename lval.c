#include "config.h"
#include "pool.h"

/* Linux/Mac 专用头文件 */
#include <editline/readline.h>
#include <editline/history.h>

/* 在 lval.c 中添加 */

char* lval_str_escape(char* s) {
  char* buffer = malloc(strlen(s) * 2 + 1); // 最坏情况每个字符都转义
  char* p = buffer;
  
  while (*s) {
    switch (*s) {
      case '\a': *p++ = '\\'; *p++ = 'a'; break;
      case '\b': *p++ = '\\'; *p++ = 'b'; break;
      case '\f': *p++ = '\\'; *p++ = 'f'; break;
      case '\n': *p++ = '\\'; *p++ = 'n'; break;
      case '\r': *p++ = '\\'; *p++ = 'r'; break;
      case '\t': *p++ = '\\'; *p++ = 't'; break;
      case '\v': *p++ = '\\'; *p++ = 'v'; break;
      case '\\': *p++ = '\\'; *p++ = '\\'; break;
      case '\"': *p++ = '\\'; *p++ = '\"'; break;
      default: *p++ = *s; break;
    }
    s++;
  }
  *p = '\0';
  
  buffer = realloc(buffer, strlen(buffer) + 1); // 缩减内存
  return buffer;
}

/* 在 lval.c 中添加 */

char* lval_str_unescape(char* s) {
  char* buffer = malloc(strlen(s) + 1);
  char* p = buffer;
  
  while (*s) {
    if (*s == '\\') {
      s++;
      switch (*s) {
        case 'a': *p++ = '\a'; break;
        case 'b': *p++ = '\b'; break;
        case 'f': *p++ = '\f'; break;
        case 'n': *p++ = '\n'; break;
        case 'r': *p++ = '\r'; break;
        case 't': *p++ = '\t'; break;
        case 'v': *p++ = '\v'; break;
        case '\\': *p++ = '\\'; break;
        case '\"': *p++ = '\"'; break;
        default: *p++ = '\\'; *p++ = *s; break; // 未知转义，保留原样
      }
    } else {
      *p++ = *s;
    }
    s++;
  }
  *p = '\0';
  
  buffer = realloc(buffer, strlen(buffer) + 1);
  return buffer;
}

char* ltype_name(int t) {
  switch(t) {
    case LVAL_FUN: return "Function";
    case LVAL_NUM: return "Number";
    case LVAL_ERR: return "Error";
    case LVAL_SYM: return "Symbol";
    case LVAL_SEXPR: return "S-Expression";
    case LVAL_QEXPR: return "Q-Expression";
    case LVAL_STR: return "String";
    case LVAL_FILE: return "File";
    default: return "Unknown";
  }
}


/* Create a new number type lval */
lval* lval_num(long x) {
  lval* v = lval_alloc();
  v->type = LVAL_NUM;
  v->num = x;
  return v;
}

/* Create a new decimal type lval */
lval* lval_dec(double x) {
  lval* v = lval_alloc();
  v->type = LVAL_DEC;
  v->dec = x;
  return v;
}


/* Create a new error type lval */
lval* lval_err(char* fmt, ...) {
 lval* v = lval_alloc();
 v->type = LVAL_ERR;

 /* Create a va list and initialize it */
 va_list va;
 va_start(va, fmt);

 /* Allocate 512 bytes of space */
 v->err = malloc(512);
 /* printf the error string with a maximum of 511 characters */
  vsnprintf(v->err, 511, fmt, va);

  /* Reallocate to number of bytes actually used */
  v->err = realloc(v->err, strlen(v->err)+1);

  /* Cleanup our va list */
  va_end(va);

  return v;
}

lval* lval_sym(char* s) {
  lval* v = lval_alloc();
  v->type = LVAL_SYM;
  v->sym = malloc(strlen(s) + 1);
  strcpy(v->sym, s);
  return v;
}

lval* lval_sexpr(void) {
  lval* v = lval_alloc();
  v->type = LVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

lval* lval_add(lval* v, lval* x) {
  v->count++;
  v->cell = realloc(v->cell, sizeof(lval*) * v->count);
  v->cell[v->count-1] = x;
  return v;
}

lval* lval_offer(lval* v, lval* x) {
  v->count++;
  v->cell = realloc(v->cell, sizeof(lval*) * v->count);
  memmove(&v->cell[1], &v->cell[0], sizeof(lval*) * (v->count-1));
  v->cell[0] = x;
  return v;
}

lval* lval_copy(lval* v) {
  lval* x = lval_alloc();
  x->type = v->type;
  switch (v->type) {
    /* Copy Functions and Numbers Directly */
    case LVAL_FUN: 
      if (v->builtin) {
        x->builtin = v->builtin;
      } else {
        x->builtin = NULL;
        x->env = lenv_copy(v->env);
        x->formals = lval_copy(v->formals);
        x->body = lval_copy(v->body);
      }
      break;
    case LVAL_NUM: x->num = v->num; break;

    /* Copy Strings using malloc and strcpy */
    case LVAL_ERR:
      x->err = malloc(strlen(v->err) + 1);
      strcpy(x->err, v->err); break;
    case LVAL_STR: x->str = malloc(strlen(v->str) + 1); strcpy(x->str, v->str); break;
    case LVAL_SYM: x->sym = malloc(strlen(v->sym) + 1); strcpy(x->sym, v->sym); break;
    
    /* Copy Lists by copying each sub-expression */
    case LVAL_QEXPR:
    case LVAL_SEXPR:
      x->count = v->count;
      x->cell = malloc(sizeof(lval*) * x->count);
      for (int i = 0; i < x->count; i++) {
        x->cell[i] = lval_copy(v->cell[i]);
      }
      break;
      
    case LVAL_FILE:
        //x->file = malloc(sizeof(FILE));
        x->file_rc = v->file_rc;
        x->file_rc->ref_count++;
        break;
  }
  
  return x;
}

void lval_del(lval* v) {
  if (!v) return;
  lval_vec stack = {0};
  vec_push(&stack, v);

  for (int i = 0;i < stack.count;i++) {

    lval* curr = stack.items[i];
    switch (curr->type) {
      case LVAL_SEXPR:
      case LVAL_QEXPR:
        for (int j = 0;j < curr->count;j++) {
          vec_push(&stack, curr->cell[j]);
        }
        break;
      case LVAL_FUN:
        if (!curr->builtin) {
          vec_push(&stack, curr->formals);
          vec_push(&stack, curr->body);

          lenv* e = curr->env;
          if (e) {
            for (int j = 0;j < e->count;j++) {
              vec_push(&stack, e->vals[j]);
            }
            free(e->syms);
            free(e->vals);
            free(e);
            curr->env = NULL;
          }
        }
        break;
    }
  }
  for (int i = stack.count - 1;i >= 0;i--) {
    lval* curr = stack.items[i];

    switch (curr->type) {
      case LVAL_SYM : free(curr->sym); break;
      case LVAL_ERR : free(curr->err); break;
      case LVAL_STR : free(curr->str); break;
      case LVAL_SEXPR:
      case LVAL_QEXPR:
        free(curr->cell); break;
      case LVAL_FILE:
        curr->file_rc->ref_count--;
        if (curr->file_rc->ref_count == 0) {
          fclose(curr->file_rc->file);
          free(curr->file_rc->mode);
          free(curr->file_rc);
        }
        break;
    }
    lval_release(curr);
  }
  vec_free(&stack);
}

lval* lval_fun(lbuiltin func) {
  lval* v = lval_alloc();
  v->type = LVAL_FUN;
  v->builtin = func;
  v->sym = NULL; // 初始化为空，后续由环境注入名字
  return v;
}

lval* lval_qexpr(void) {
  lval* v = lval_alloc();
  v->type = LVAL_QEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

lval* lval_read_num(mpc_ast_t* t) {
  errno = 0;
  if (strstr(t->contents, ".")) {
    double x = strtod(t->contents, NULL);
    return errno != ERANGE ? lval_dec(x) : lval_err("invalid number");
  }
  long x = strtol(t->contents, NULL, 10);
  return errno != ERANGE ? lval_num(x) : lval_err("invalid number");
}

lval* lval_read(mpc_ast_t* t) {
  /* If Symbol or Number return conversion to that type */
  if (strstr(t->tag, "number")) { return lval_read_num(t); }
  if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }

  /* If root (>) or sexpr then create empty list */
  lval* x = NULL;
  if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
  if (strstr(t->tag, "sexpr")) { x = lval_sexpr(); }
  if (strstr(t->tag, "qexpr")) { x = lval_qexpr(); }
  if (strstr(t->tag, "string")) { return lval_read_str(t); }
  
  for (int i = 0; i < t->children_num; i++) {
    if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
    if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
    if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
    if (strcmp(t->children[i]->contents, "}") == 0) { continue; }
    if (strcmp(t->children[i]->tag,  "regex") == 0) { continue; }
    if (strstr(t->children[i]->tag, "comment")) { continue; }
    x = lval_add(x, lval_read(t->children[i]));
  }

  return x;
}


// v: 要打印的列表 (S-Expression)
// open: 开头的字符 (比如 '(' )
// close: 结尾的字符 (比如 ')' )
void lval_expr_print(lval* v, char open, char close) {
  
  putchar(open); // 1. 先打印开头的括号
  
  for(int i = 0; i < v->count; i++) { // 2. 遍历列表里的每一个子元素
    
    /* Print Value contained within */
    lval_print(v->cell[i]); // 3. 递归调用打印函数，打印子元素
                            // (比如列表里有个数字 5，就去打印 5)

    /* Don't print trailing space if last element */
    if(i != (v->count - 1)) { // 4. 如果不是最后一个元素，就在后面加个空格
      putchar(' ');           // 这样输出就是 (1 2 3) 而不是 (1 2 3 )
    }
  }
  
  putchar(close); // 5. 最后打印结尾的括号
}

lval* lval_eval(lenv* e, lval* v) {

  while(1) {
    if (v->type == LVAL_SYM) {
      /* ... 查找符号 ... */
      /* 如果找到值，释放原来的 v，返回新值 */
      /* 这里不需要循环，因为符号求值结果就是结果 */
      lval* x = lenv_get(e, v);
      lval_del(v);
      return x;
    }
    if (v->type == LVAL_SEXPR) {
      /* Evaluate Children (Recursive, not tail call) */
      /* 这里必须递归，因为参数本身可能是复杂的表达式 */
      for (int i = 0;i < v->count;i++) {
        v->cell[i] = lval_eval(e, v->cell[i]);
      }

      /* Error Checking */
      for (int i = 0;i < v->count;i++) {
        if (v->cell[i]->type == LVAL_ERR) {
          return lval_take(v, i);
        }
      }
      
      /* Empty Expression */
      if (v->count == 0) { return v; }
      /* Single Expression */
      if (v->count == 1 && v->cell[0]->type != LVAL_FUN) { return lval_take(v, 0); }

      lval* f = lval_pop(v, 0);
      if (f->type != LVAL_FUN) {
        lval_del(f);
        lval_del(v);
        return lval_err("S-Expression starts with incorrect type. Got %s, Expected %s.",
          ltype_name(f->type), ltype_name(LVAL_FUN));
      }

      /* 如果是内置函数，直接调用 */
      if (f->builtin) {
        
        /* TCO Patch for IF: Handle 'if' specifically to avoid recursion */
        if (f->builtin == builtin_if) {
          if (v->count != 3) {
            lval_del(f); lval_del(v);
            return lval_err("Function 'if' passed incorrect number of arguments.");
          }
          if (v->cell[0]->type != LVAL_NUM) {
            lval_del(f); lval_del(v);
            return lval_err("Function 'if' passed incorrect type for condition.");
          }
          if (v->cell[1]->type != LVAL_QEXPR || v->cell[2]->type != LVAL_QEXPR) {
            lval_del(f); lval_del(v);
            return lval_err("Function 'if' passed incorrect type for branches.");
          }

          lval* cond = lval_pop(v, 0);
          lval* then_branch = lval_pop(v, 0);
          lval* else_branch = lval_pop(v, 0);

          lval* chosen = NULL;
          if (cond->num) {
            chosen = then_branch;
            lval_del(else_branch);
          } else {
            chosen = else_branch;
            lval_del(then_branch);
          }

          lval_del(cond);
          lval_del(f);
          lval_del(v);

          chosen->type = LVAL_SEXPR;
          v = chosen;
          continue;
        }

        lval* result = f->builtin(e, v);
        lval_del(f);
        return result;
      }

      /* 如果是自定义函数 */
      /* 参数绑定到env中 */

      int given = v->count;
      int total = f->formals->count;
      while (v->count) {
        if (f->formals->count == 0) {
          lval_del(f);
          lval_del(v);
          return lval_err("Function passed too many arguments. Got %i, Expected %i.", given, total);
        }

        lval* sym = lval_pop(f->formals, 0);

        if(strcmp(sym->sym, "&") == 0) {
          if (f->formals->count != 1) {
            lval_del(v);
            lval_del(f);
            return lval_err("Function format invalid. Symbol '&' not followed by single symbol.");
          }

          lval* nsym = lval_pop(f->formals, 0);
          lenv_put(f->env, nsym, builtin_list(e, v));
          builtin_list(e, v);
          lval_del(nsym);
          lval_del(nsym);
          break;
        }
        lval* val = lval_pop(v, 0);
        lenv_put(f->env, sym, val);
        lval_del(sym);
        lval_del(val);
      }
      lval_del(v);

      /* 如果形参列表空了，说明参数都齐了，可以执行函数体了！ */
      if (f->formals->count > 0 && strcmp(f->formals->cell[0]->sym, "&") == 0) {
        if (f->formals->count != 2) {
          lval_del(f);
          return lval_err("Function format invalid. Symbol '&' not followed by single symbol.");
        }
        lval_del(lval_pop(f->formals, 0));
        lval* sym = lval_pop(f->formals, 0);
        lval* val = lval_qexpr();
        lenv_put(f->env, sym, val);
        lval_del(sym);
        lval_del(val);
      }

      if (f->formals->count == 0) {
          
          /* TCO: Path Compression for Environment to prevent stack overflow in lenv_get */
          if (e->par) {
            f->env->par = e->par;
          } else {
            f->env->par = e;
          }
          
          lval* body = lval_copy(f->body);
          body->type = LVAL_SEXPR;
          
          lenv* next_e = f->env;
          
          f->env = NULL; 
          lval_del(f);
          
          v = body;
          e = next_e;
          continue; 
          
      } else {
        return f;
      }
    }
    return v;
  }
}

void lval_print(lval* v) {
  switch (v->type) {
    case LVAL_NUM : printf("%li", v->num); break;
    case LVAL_DEC : printf("%g", v->dec); break;
    case LVAL_ERR: printf("Error: %s", v->err); break;
    case LVAL_SYM: printf("%s", v->sym); break;
    case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
    case LVAL_QEXPR: lval_expr_print(v, '{', '}'); break;
    case LVAL_FUN:
        if (v->builtin) {
            printf("<builtin>");
        } else {
            printf("(\\ "); lval_print(v->formals);
            putchar(' '); lval_print(v->body); putchar(')');
        }  
      break;
    case LVAL_STR: lval_print_str(v);break;
    case LVAL_FILE: printf("<file %p>", v->file_rc->file); break;
    break;
  }
}

void lval_print_str(lval* v) {
  /* Make a Copy of the string */
  char* escaped = malloc(strlen(v->str)+1);
  strcpy(escaped, v->str);
  /* Pass it through the escape function */
  //escaped = mpcf_escape(escaped);
  char* new_escaped = lval_str_escape(escaped);
  free(escaped);
  /* Print it between " characters */
  printf("\"%s\"", new_escaped);
  /* free the copied string */
  free(new_escaped);
}

void lval_println(lval* v) {
  lval_print(v);
  printf("\n"); 
}

lval* lval_pop(lval* v, int i) {
  /* Find the item at "i" */
  lval* x = v->cell[i];

  /* Shift memory after the item at "i" over the top */
  memmove(&v->cell[i], &v->cell[i+1],
    sizeof(lval*) * (v->count-i-1));

  /* Decrease the count of items in the list */
  v->count--;

  /* Reallocate the memory used */
  v->cell = realloc(v->cell, sizeof(lval*) * v->count);
  return x;
}

lval* lval_take(lval* v, int i) {
  lval* x = lval_pop(v, i);
  lval_del(v);
  return x;
}

lval* lval_join(lval* x, lval* y) {
  while(y->count) {
    x = lval_add(x, lval_pop(y, 0));
  }
  lval_del(y);
  return x;
}

#if 0
lval* lval_eval_sexpr(lenv* e, lval* v) {
  
  /* Evaluate Children */
  for (int i = 0; i < v->count; i++) {
    v->cell[i] = lval_eval(e, v->cell[i]);
  }

  /* Error Checking */
  for (int i = 0; i < v->count; i++) {
    if (v->cell[i]->type == LVAL_ERR) { return lval_take(v, i); }
  }

  /* Empty Expression */
  if (v->count == 0) { return v; }

  /* Single Expression */
  if (v->count == 1 && v->cell[0]->type != LVAL_FUN) { return lval_take(v, 0); }

  /* Ensure First Element is Function */
  lval* f = lval_pop(v, 0);
  if (f->type != LVAL_FUN) {
    lval* err = lval_err(
    "S-Expression starts with incorrect type. "
    "Got %s, Expected %s.",
    ltype_name(f->type), ltype_name(LVAL_FUN));
    lval_del(f); lval_del(v);
    return err;
  }

  /* Call builtin with operator */
  lval* result = lval_call(e, f, v);
  lval_del(f);
  return result;
}
#endif

lval* lval_lambda(lval* formals, lval* body) {
    lval* v = lval_alloc();
    v->type = LVAL_FUN;

    /* Set Builtin to Null */
    v->builtin = NULL;

    /* Build new environment */
    v->env = lenv_new();

    /* Set Formals and Body */
    v->formals = formals;
    v->body = body;
    return v;
}

#if 0
lval* lval_call(lenv* e, lval* f, lval* a) {
    /* If Builtin then simply call that */
    if (f->builtin) { return f->builtin(e, a); }

    /* Record Argument Counts */
    int given = a->count;
    int total = f->formals->count;

    /* While arguments still remain to be processed */
    while (a->count) {

        /* If we've ran out of formal arguments to bind */
        if (f->formals->count == 0) {
            lval_del(a);
            return lval_err("Function passed too many arguments. "
                            "Got %i, Expected %i.", given, total);
        }

        /* Pop the first symbol from the formals */
        lval* sym = lval_pop(f->formals, 0);

        /* Special Case to deal with '&' */
        if (strcmp(sym->sym, "&") == 0) {

            // 新增检查：如果没有实参了，报错！
            if (a->count == 0) {
                lval_del(a);
                return lval_err("Function passed too few arguments. "
                                "Expected at least one argument for '&'.");
            }
            /* Ensure '&' is not passed invalidly */
            if (f->formals->count != 1) {
                lval_del(a);
                return lval_err("Function format invalid. "
                                "Symbol '&' not followed by single symbol.");
            }

            /* Next formal should be bound to remaining arguments */
            lval* nsym = lval_pop(f->formals, 0);
            lenv_put(f->env, nsym, builtin_list(e, a));
            lval_del(sym);
            lval_del(nsym);
            break;
        }

        /* Pop the next argument from the list */
        lval* val = lval_pop(a, 0);

        /* Bind a copy into the function's environment */
        lenv_put(f->env, sym, lval_copy(val));

        /* Delete symbol and value */
        lval_del(sym);
        lval_del(val);
    }

    /* Argument list is now bound so can be cleaned up */
    lval_del(a);

    /* If '&' remains in formal list bind to empty list */
    if (f->formals->count > 0 &&
        strcmp(f->formals->cell[0]->sym, "&") == 0) {

        /* Check to ensure that & is not passed invalidly */
        if (f->formals->count != 2) {
            return lval_err("Function format invalid. "
                            "Symbol '&' not followed by single symbol.");
        }

        /* Pop and delete '&' symbol */
        lval_del(lval_pop(f->formals, 0));

        /* Pop next symbol and create empty list */
        lval* sym = lval_pop(f->formals, 0);
        lval* val = lval_qexpr();

        /* Bind to environment and delete */
        lenv_put(f->env, sym, val);
        lval_del(sym);
        lval_del(val);
    }

    /* If all formals have been bound evaluate */
    if (f->formals->count == 0) {

        /* Set environment parent to evaluation environment */
        f->env->par = e;

        /* Evaluate and return */
        return builtin_eval(f->env,
                            lval_add(lval_sexpr(), lval_copy(f->body)));
    } else {
        /* Otherwise return partially evaluated function */
        return lval_copy(f);
    }
}
#endif


int lval_eq(lval* x, lval* y) {
  /* Different Types are always unequal */
  if (x->type != y->type) { return 0;}

  /* Compare Based upon type */
  switch (x->type) {
    /* Compare Number Value */
    case LVAL_NUM: return x->num == y->num;

    /* Compare String Values */
    case LVAL_ERR : return (strcmp(x->err, y->err) == 0);
    case LVAL_SYM : return (strcmp(x->sym, y->sym) == 0);
    
    /* If builtin compare, otherwise compare formals and body */
    case LVAL_FUN :
      if (x->builtin || y->builtin) { 
        return x->builtin == y->builtin; 
      } else {
        return lval_eq(x->formals, y->formals) 
          && lval_eq(x->body, y->body); 
      }

    /* If list compare every individual element */
    case LVAL_SEXPR:
    case LVAL_QEXPR:
      if (x->count != y->count) { return 0;}
      for (int i = 0;i < x->count; i++) {
        /* If any element not equal then whole list not equal */
        if (!lval_eq(x->cell[i], y->cell[i])) { return 0; }
      }
      /* Otherwise lists must be equal */
      return 1;
    case LVAL_STR: return (strcmp(x->str, y->str) == 0);
    break;
  }
  return 0;
}

lval* lval_str(char* s) {
    lval* v = lval_alloc();
    v->type = LVAL_STR;
    v->str = malloc(strlen(s) + 1);
    strcpy(v->str, s);
    return v;
}

lval* lval_read_str(mpc_ast_t* t) {
  /* Cut off the final quote character */
  t->contents[strlen(t->contents)-1] = '\0';
  /* Copy the string missing out the first quote character */
  char* unescaped = malloc(strlen(t->contents+1)+1);
  strcpy(unescaped, t->contents+1);
  /* Pass through the unescape function */
  //unescaped = mpcf_unescape(unescaped);
  char* new_unescaped = lval_str_unescape(unescaped);
  free(unescaped);
  /* Construct a new lval using the string */
  lval* str = lval_str(new_unescaped);
  /* Free the string and return */
  free(new_unescaped);
  return str;
}