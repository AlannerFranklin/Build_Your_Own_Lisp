#include "pool.h"
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Global free list head */
static lval* free_list = NULL;

/* Statistics (optional, for debugging) */
long pool_count = 0;      // Number of free objects in pool
long total_allocs = 0;    // Total system mallocs performed

void lval_pool_init(void) {
    free_list = NULL;
    pool_count = 0;
    total_allocs = 0;
}

void lval_pool_print_stats(void) {
    printf("\n=== Memory Pool Statistics ===\n");
    printf("Total System Mallocs: %ld (This is the total number of unique lval blocks created)\n", total_allocs);
    printf("Current Free Objects: %ld (Objects returned to pool and ready for reuse)\n", pool_count);
    printf("==============================\n");
}

lval* lval_alloc(void) {
    if (free_list == NULL) {
        // Pool is empty, request from OS
        total_allocs++;
        return malloc(sizeof(lval));
    } else {
        // Reuse from pool
        lval* v = free_list;
        
        // Move head to next (reuse body as next pointer)
        free_list = v->body; 
        
        pool_count--;
        return v;
    }
}

void lval_release(lval* v) {
    if (v == NULL) return;

    // Insert v at head of free list
    // Reuse body field to point to old head
    v->body = free_list;
    free_list = v;
    
    pool_count++;
}

void lval_pool_cleanup(void) {
    lval* curr = free_list;
    while (curr) {
        lval* next = curr->body;
        free(curr); // Actually free to OS
        curr = next;
    }
    free_list = NULL;
    pool_count = 0;
}

void lval_pool_dump_log(const char* filename) {
    FILE* f = fopen(filename, "a"); // Append mode
    if (!f) return;
    
    time_t now = time(NULL);
    char* timestamp = ctime(&now);
    timestamp[strlen(timestamp)-1] = '\0'; // Remove newline
    
    fprintf(f, "[%s] Total Allocs: %ld | Free Objects: %ld | Active Objects: %ld\n", 
        timestamp, total_allocs, pool_count, total_allocs - pool_count);
        
    fclose(f);
}