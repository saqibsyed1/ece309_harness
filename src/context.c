#define _POSIX_C_SOURCE 200809L /* needed so strdup() is available */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "context.h"

void context_init(Context *ctx) {
    ctx->count = 0;
    ctx->next_slot = 0;
    for (int i = 0; i < MAX_TURNS; i++) {
        ctx->turns[i].user_msg = NULL;
        ctx->turns[i].assistant_msg = NULL;
    }
}

void context_add_turn(Context *ctx, const char *user_msg, const char *assistant_msg) {
    int slot = ctx->next_slot;

    /* If this slot is already holding an old turn (buffer is full and
       wrapping around), free the old strings first so we don't leak them. */
    if (ctx->turns[slot].user_msg != NULL) {
        free(ctx->turns[slot].user_msg);
        free(ctx->turns[slot].assistant_msg);
    }

    ctx->turns[slot].user_msg = strdup(user_msg);
    ctx->turns[slot].assistant_msg = strdup(assistant_msg);

    ctx->next_slot = (slot + 1) % MAX_TURNS;   /* wrap around after 5 */
    if (ctx->count < MAX_TURNS) {
        ctx->count++;
    }
}

void context_free(Context *ctx) {
    for (int i = 0; i < MAX_TURNS; i++) {
        free(ctx->turns[i].user_msg);
        free(ctx->turns[i].assistant_msg);
    }
}
