#include <stdio.h>
#include <string.h>
#include "context.h"
#include "model.h"

#define INPUT_SIZE 512
#define RESPONSE_SIZE 512

/* Removes the trailing newline that fgets() leaves on the input. */
static void strip_newline(char *s) {
    int len = strlen(s);
    if (len > 0 && s[len - 1] == '\n') {
        s[len - 1] = '\0';
    }
}

int main(void) {
    Context ctx;
    context_init(&ctx);

    char input[INPUT_SIZE];
    char response[RESPONSE_SIZE];

    printf("Mini LLM Harness. Type '/exit' or '/quit' to leave.\n");
    printf("Try: hello   OR   calc 3 + 4\n\n");

    while (1) {
        printf("you> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin)) {
            break; /* end of input (e.g. Ctrl+D) */
        }
        strip_newline(input);

        if (strcmp(input, "/exit") == 0 || strcmp(input, "/quit") == 0) {
            printf("assistant> Goodbye.\n");
            break;
        }

        if (strlen(input) == 0) {
            continue; /* ignore blank lines */
        }

        mock_model_generate(input, response, sizeof(response));
        printf("assistant> %s\n", response);

        context_add_turn(&ctx, input, response);
    }

    context_free(&ctx);
    return 0;
}
