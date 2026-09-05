#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tool.h"

#define MAX_TOKENS 64

bool tool_is_calc_request(const char *input, const char **expr) {
    /* Does the input start with "calc "? */
    if (strncmp(input, "calc ", 5) == 0) {
        *expr = input + 5;   /* point right after "calc " */
        return true;
    }
    return false;
}

/* Breaks "3 + 4 * 2" into tokens: [3] [+] [4] [*] [2]
   Numbers go in 'nums', operators go in 'ops'.
   Returns how many numbers were found, or -1 on bad syntax. */
static int tokenize(const char *expr, double *nums, char *ops) {
    int num_count = 0;
    int op_count = 0;
    const char *p = expr;

    while (*p != '\0') {
        if (isspace((unsigned char)*p)) {
            p++;
            continue;
        }
        if (isdigit((unsigned char)*p) || *p == '.') {
            char *end;
            nums[num_count++] = strtod(p, &end);
            if (end == p) return -1;   /* nothing valid was read */
            p = end;
        } else if (*p == '+' || *p == '-' || *p == '*' || *p == '/') {
            ops[op_count++] = *p;
            p++;
        } else {
            return -1;   /* unexpected character */
        }
        if (num_count >= MAX_TOKENS || op_count >= MAX_TOKENS) return -1;
    }

    if (num_count == 0 || num_count != op_count + 1) {
        return -1;   /* e.g. "3 + " or "+ 4" is malformed */
    }
    return num_count;
}

bool tool_calculate(const char *expr, double *result, char *err_msg, int err_msg_size) {
    double nums[MAX_TOKENS];
    char ops[MAX_TOKENS];

    int n = tokenize(expr, nums, ops);
    if (n <= 0) {
        snprintf(err_msg, err_msg_size, "could not understand expression \"%s\"", expr);
        return false;
    }

    /* Pass 1: handle * and / first (standard math precedence) */
    for (int i = 0; i < n - 1; i++) {
        if (ops[i] == '*' || ops[i] == '/') {
            if (ops[i] == '/' && nums[i + 1] == 0.0) {
                snprintf(err_msg, err_msg_size, "division by zero");
                return false;
            }
            nums[i] = (ops[i] == '*') ? nums[i] * nums[i + 1] : nums[i] / nums[i + 1];

            /* We just "consumed" nums[i+1] and ops[i] -- remove them by
               shifting everything after them down by one slot. The nums
               array has n entries; the ops array has n-1 entries. */
            for (int j = i + 1; j < n - 1; j++) {
                nums[j] = nums[j + 1];
            }
            for (int j = i; j < n - 2; j++) {
                ops[j] = ops[j + 1];
            }
            n--;
            i--; /* re-check this position in case of another * or / */
        }
    }

    /* Pass 2: handle remaining + and - left to right */
    double total = nums[0];
    for (int i = 0; i < n - 1; i++) {
        total = (ops[i] == '+') ? total + nums[i + 1] : total - nums[i + 1];
    }

    *result = total;
    return true;
}
