#include "config.h"
#include "error.h"

lval* lval_file(char* mode) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_FILE;

    /* 分配共享结构体 */
    v->file_rc = malloc(sizeof(lval_file_t));
    v->file_rc->file = NULL;
    v->file_rc->ref_count = 1; /* 初始化引用计数为 1 */
    v->file_rc->mode = malloc(strlen(mode) + 1);
    strcpy(v->file_rc->mode, mode);
    return v;
}

lval* builtin_fopen(lenv* e, lval* a) {
    LASSERT_NUM("fopen", a, 2);
    LASSERT_TYPE("fopen", a, 0, LVAL_STR);
    LASSERT_TYPE("fopen", a, 1, LVAL_STR);

    char* filename = a->cell[0]->str;
    char* mode = a->cell[1]->str;

    /* Create the file lval */
    lval* f = lval_file(mode);
    f->file_rc->file = fopen(filename, mode);

    /* If fopen failed */
    if (!f->file_rc->file) {
        lval_del(f);
        lval_del(a);
        return lval_err("Failed to open file '%s' with mode '%s'.", filename, mode);
    }
    lval_del(a);
    return f;
}

lval* builtin_fclose(lenv* e, lval* a) {
    LASSERT_NUM("fclose", a, 1);
    LASSERT_TYPE("fclose", a, 0, LVAL_FILE);

    lval* f = a->cell[0];
    if (f->file_rc->file) {
        fclose(f->file_rc->file);
        f->file_rc->file = NULL;//为什么不是直接del？
    }
    lval_del(a);
    return lval_sexpr();
}

lval* builtin_fread(lenv* e, lval* a) {
    LASSERT_NUM("fread", a, 2);
    LASSERT_TYPE("fread", a, 0, LVAL_FILE);
    LASSERT_TYPE("fread", a, 1, LVAL_NUM);

    lval* f = a->cell[0];
    long size = a->cell[1]->num;

    /* Check if file is open */
    if (!f->file_rc->file) {
        lval_del(a);
        return lval_err("Cannot read from a closed file!");
    }

    /* Allocate buffer */
    char* buffer = malloc(size + 1);
    
    /* Read from file */
    size_t read_size = fread(buffer, 1, size, f->file_rc->file);
    buffer[read_size] = '\0';

    lval_del(a);
    
    /* Return as string */
    lval* result = lval_str(buffer);
    free(buffer);
    return result;
}




lval* builtin_fwrite(lenv* e, lval* a) {
    LASSERT_NUM("fwrite", a, 2);
    LASSERT_TYPE("fwrite", a, 0, LVAL_FILE);
    LASSERT_TYPE("fwrite", a, 1, LVAL_STR);

    lval* f = a->cell[0];
    char* str = a->cell[1]->str;

    if(!f->file_rc->file) {
        lval_del(a);
        return lval_err("Cannot write to a closed file!");
    }

    fwrite(str, 1, strlen(str), f->file_rc->file);//这里为什么不用像fread那样分配缓冲区？
    lval_del(a);
    return lval_sexpr();
}

lval* builtin_fseek(lenv* e, lval* a) {
    LASSERT_NUM("fseek", a, 2);
    LASSERT_TYPE("fseek", a, 0, LVAL_FILE);
    LASSERT_TYPE("fseek", a, 1, LVAL_NUM);

    lval* f = a->cell[0];
    long offset = a->cell[1]->num;

    if(!f->file_rc->file) {
        lval_del(a);
        return lval_err("Cannot seek in a closed file!");
    }

    fseek(f->file_rc->file, offset, SEEK_SET);
    lval_del(a);
    return lval_sexpr();
}

lval* builtin_ftell(lenv* e, lval* a) {
    LASSERT_NUM("ftell", a, 1);
    LASSERT_TYPE("ftell", a, 0, LVAL_FILE);

    lval* f = a->cell[0];
    if(!f->file_rc->file) {
        lval_del(a);
        return lval_err("Cannot tell position in a closed file!");
    }

    long pos = ftell(f->file_rc->file);
    lval_del(a);
    return lval_num(pos);
}

lval* builtin_rewind(lenv* e, lval* a) {
    LASSERT_NUM("rewind", a, 1);
    LASSERT_TYPE("rewind", a, 0, LVAL_FILE);

    lval* f = a->cell[0];

    if (!f->file_rc->file) {
        lval_del(a);
        return lval_err("Cannot rewind a closed file!");
    }

    rewind(f->file_rc->file);

    lval_del(a);
    return lval_sexpr();
}