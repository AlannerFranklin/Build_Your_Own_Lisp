#include "config.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>

Tokenizer tokenizer_new(char* src) {
    Tokenizer t;
    t.src = src;
    t.pos = 0;
    t.len = strlen(src);
    return t;
}

/* 查看当前字符 */
char peek(Tokenizer* t) {
    if (t->pos >= t->len) return '\0';
    return t->src[t->pos];
}

/* 消耗当前字符并前进 */
char advance(Tokenizer* t) {
    if (t->pos >= t->len) return '\0';
    return t->src[t->pos++];
}

int is_sym_char(char c) {
    if (c == '\0') return 0;
    return isalnum(c) || strchr("!$%&*/:<=>?@^_~+-\\", c);
}

Token next_token(Tokenizer* t) {
    Token tok;
    tok.error = NULL;
    while (isspace(peek(t))) advance(t);
    if (peek(t) == ';') {
        while (peek(t) != '\n' && peek(t) != '\0') advance(t);
        return next_token(t);
    }
    tok.start = &t->src[t->pos];
    if (t->pos >= t->len) {
        tok.type = TOK_EOF;//改为结束符是为什么?
        tok.length = 0;
        return tok;
    }
    char c = advance(t);
    switch(c) {
        case '(' : tok.type = TOK_LPAREN; tok.length = 1; return tok;
        case ')' : tok.type = TOK_RPAREN; tok.length = 1; return tok;
        case '{' : tok.type = TOK_LBRACE; tok.length = 1; return tok;
        case '}' : tok.type = TOK_RBRACE; tok.length = 1; return tok;
    }

    if (c == '"') {
        tok.type = TOK_STR;
        while (peek(t) != '"' && peek(t) != '\0') {
            /* 处理转义字符 \" (简单处理，暂不支持复杂转义) */
            if (peek(t) == '\\') advance(t);
            advance(t);
        }
        if (peek(t) == '"') {
            advance(t);
            tok.length = &t->src[t->pos] - tok.start;
            return tok;
        } else {
            tok.type = TOK_ERR;
            tok.error = "Unterminated string literal";
            return tok;
        }
    }

    if (isdigit(c)) {
        tok.type = TOK_NUM;
        while (isdigit(peek(t)) || peek(t) == '.') advance(t);
        tok.length = &t->src[t->pos] - tok.start;
        return tok;
    } 

    if (is_sym_char(c)) {
        tok.type = TOK_SYM;
        while (is_sym_char(peek(t))) advance(t);
        tok.length = &t->src[t->pos] - tok.start;
        return tok;
    }

    tok.type = TOK_ERR;
    tok.error = "Unexpected character";
    tok.length = 1;
    return tok;
}

lval* parse_sexpr(Tokenizer* t) {
    return parse_expr_list(t, TOK_RPAREN);
}

lval* parse_qexpr(Tokenizer* t) {
    return parse_expr_list(t, TOK_RBRACE);
}

lval* parse_atom(Token tok) {
    char* s = malloc(tok.length + 1);
    strncpy(s, tok.start, tok.length);
    s[tok.length] = '\0';
    
    if (tok.type == TOK_NUM) {
        long x = strtol(s, NULL, 10);
        free(s);
        return lval_num(x);
    }

    if (tok.type == TOK_SYM) {
        lval* v = lval_sym(s);
        free(s);
        return v;
    }

    if (tok.type == TOK_STR) {
        char* unescaped = lval_str_unescape(s);
        lval* v = lval_str(unescaped);
        free(unescaped);
        free(s);
        return v;
    }

    free(s);
    return lval_err("Unexpected token type in parse_atom");
 }

lval* parse_expr_list(Tokenizer* t, TokenType end_type) {
    lval* res;
    if (end_type == TOK_RPAREN) {
        res = lval_sexpr();
    } else {
        res = lval_qexpr();
    }

    Token tok = next_token(t);
    while (tok.type != end_type && tok.type != TOK_EOF) {
        if (tok.type == TOK_ERR) {
            lval_del(res);
            return lval_err(tok.error);
        }

        lval* ele = NULL;

        if (tok.type == TOK_LPAREN) {
            ele = parse_sexpr(t);
        } else if (tok.type == TOK_LBRACE) {
            ele = parse_qexpr(t);
        } else {
            ele = parse_atom(tok);
        }

        if (ele->type == LVAL_ERR) {
            lval_del(res);
            return ele;
        }

        lval_add(res, ele);
        tok = next_token(t);
    }

    if (tok.type != end_type) {
        lval_del(res);
        return lval_err("Missing closing parenthesis/brace");
    }
    return res;
}

lval* lval_parse(char* input) {
    Tokenizer t = tokenizer_new(input);
    lval* res = lval_sexpr();
    Token tok = next_token(&t);

    while (tok.type != TOK_EOF) {
        if (tok.type == TOK_ERR) {
            lval_del(res);
            return lval_err(tok.error);
        }

        lval* ele = NULL;
        if (tok.type == TOK_LPAREN) {
            ele = parse_expr_list(&t, TOK_RPAREN);
        } else if (tok.type == TOK_LBRACE) {
            ele = parse_expr_list(&t, TOK_RBRACE);
        } else {
            ele = parse_atom(tok);
        }

        if (ele->type == LVAL_ERR) {
            lval_del(res);
            return ele;
        }

        lval_add(res, ele);
        tok = next_token(&t);
    }
    return res;
}
