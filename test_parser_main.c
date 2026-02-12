#include "config.h"
#include <stdio.h>

/* Declare external parser entry point from parser.c */
lval* lval_parse(char* input);

/* Helper to print lval for testing - simplified version of lval_print */
void lval_print_test(lval* v);

void lval_print_test(lval* v) {
    if (!v) return;
    switch (v->type) {
        case LVAL_NUM: printf("%ld", v->num); break;
        case LVAL_ERR: printf("Error: %s", v->err); break;
        case LVAL_SYM: printf("%s", v->sym); break;
        case LVAL_STR: printf("\"%s\"", v->str); break;
        case LVAL_SEXPR:
            printf("(");
            for (int i = 0; i < v->count; i++) {
                lval_print_test(v->cell[i]);
                if (i != v->count - 1) putchar(' ');
            }
            printf(")");
            break;
        case LVAL_QEXPR:
            printf("{");
            for (int i = 0; i < v->count; i++) {
                lval_print_test(v->cell[i]);
                if (i != v->count - 1) putchar(' ');
            }
            printf("}");
            break;
        default:
            printf("<Unknown Type %d>", v->type);
    }
}

void lval_println_test(lval* v) {
    lval_print_test(v);
    putchar('\n');
}

#include "pool.h"

int main(int argc, char** argv) {
    lval_pool_init(); // Initialize stats
    printf("=== Testing Handwritten Parser ===\n");
    
    char* tests[] = {
        /* Basic Atoms */
        "123",
        "+",
        "\"hello string\"",
        "some-symbol",
        
        /* S-Expressions */
        "(+ 1 2)",
        "(list 1 2 3)",
        "(head (tail (list 1 2 3)))",
        
        /* Q-Expressions */
        "{head (list 1 2 3)}",
        "{1 2 3}",
        
        /* Definitions & Functions */
        "def {x} 100",
        "print \"hello world\"",
        "((lambda {x} {+ x 1}) 10)",
        
        /* Comments */
        "; this is a comment\n(print \"after comment\")",
        
        /* Complex/Nested */
        "(fun {x y} {+ x y})",
        "(if (== x 10) {print \"ten\"} {print \"not ten\"})",
        
        /* Error Cases */
        "(+ 1 2",   /* Missing closing paren */
        "\"unterminated", /* Unterminated string */
        ")", /* Unexpected closing paren at start - might be parsed as atom or error? */
        
        NULL
    };

    for (int i = 0; tests[i] != NULL; i++) {
        printf("\nInput:  %s\n", tests[i]);
        printf("Result: ");
        lval* result = lval_parse(tests[i]);
        lval_println_test(result);
        lval_del(result);
        
        /* Check stats after each test to see reuse in action */
        // lval_pool_print_stats(); 
    }

    lval_pool_print_stats();
    lval_pool_cleanup();

    return 0;
}
