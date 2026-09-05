#include <stdio.h>
#include "model.h"
#include "tool.h"

void mock_model_generate(const char *input, char *response_buffer, int buffer_size) {
    const char *expr;

    if (tool_is_calc_request(input, &expr)) {
        /* This mimics an LLM deciding to call a tool. The harness (not
           the "model") actually runs the calculation. */
        double result;
        char err[128];
        if (tool_calculate(expr, &result, err, sizeof(err))) {
            snprintf(response_buffer, buffer_size, "%s = %g", expr, result);
        } else {
            snprintf(response_buffer, buffer_size, "Error: %s", err);
        }
        return;
    }

    /* No tool needed -- just echo it back, per the spec. */
    snprintf(response_buffer, buffer_size, "You said: %s", input);
}
