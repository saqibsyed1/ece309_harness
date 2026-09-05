#include <stdio.h>
#include "context.h"

int main(void) {
    Context ctx;
    context_init(&ctx);

    /* Add 7 turns -- more than the 5-slot capacity */
    context_add_turn(&ctx, "msg1", "reply1");
    context_add_turn(&ctx, "msg2", "reply2");
    context_add_turn(&ctx, "msg3", "reply3");
    context_add_turn(&ctx, "msg4", "reply4");
    context_add_turn(&ctx, "msg5", "reply5");
    context_add_turn(&ctx, "msg6", "reply6");
    context_add_turn(&ctx, "msg7", "reply7");

    printf("After adding 7 turns, count = %d (should be 5, capped)\n", ctx.count);
    printf("Current stored turns:\n");
    for (int i = 0; i < MAX_TURNS; i++) {
        printf("  slot %d: user=%s assistant=%s\n",
               i, ctx.turns[i].user_msg, ctx.turns[i].assistant_msg);
    }
    printf("(msg1 and msg2 should be GONE - overwritten by msg6/msg7)\n");

    context_free(&ctx);
    printf("Freed all memory. Done.\n");
    return 0;
}
