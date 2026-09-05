#ifndef CONTEXT_H
#define CONTEXT_H

#define MAX_TURNS 5   /* per spec: remember last 5 exchanges */

typedef struct {
    char *user_msg;       /* heap-allocated copy of what the user typed */
    char *assistant_msg;  /* heap-allocated copy of the model's reply   */
} Turn;

typedef struct {
    Turn turns[MAX_TURNS];
    int count;       /* how many slots are currently filled (0 to 5) */
    int next_slot;   /* where the *next* turn will be written        */
} Context;

void context_init(Context *ctx);
void context_add_turn(Context *ctx, const char *user_msg, const char *assistant_msg);
void context_free(Context *ctx);

#endif
