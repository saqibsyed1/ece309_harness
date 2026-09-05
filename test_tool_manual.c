#include <stdio.h>
#include "tool.h"

static void try_calc(const char *input) {
    const char *expr;
    if (!tool_is_calc_request(input, &expr)) {
        printf("\"%s\" -> not a calc request\n", input);
        return;
    }
    double result;
    char err[128];
    if (tool_calculate(expr, &result, err, sizeof(err))) {
        printf("\"%s\" -> %g\n", input, result);
    } else {
        printf("\"%s\" -> ERROR: %s\n", input, err);
    }
}

int main(void) {
    try_calc("calc 3 + 4");          /* expect 7 */
    try_calc("calc 4 * 2 + 1");      /* expect 9  (precedence: 4*2=8, +1=9) */
    try_calc("calc 10 / 0");         /* expect division by zero error */
    try_calc("calc 5 +");            /* expect malformed error */
    try_calc("hello");               /* expect "not a calc request" */
    try_calc("calc 3.5 * 2");        /* expect 7 (decimals) */
    return 0;
}
