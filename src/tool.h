#ifndef TOOL_H
#define TOOL_H

#include <stdbool.h>

/* Checks if input looks like "calc <something>".
   If yes, returns true and sets *expr to point at the part after "calc ".
   If no, returns false. */
bool tool_is_calc_request(const char *input, const char **expr);

/* Evaluates a simple math expression (+ - * / with standard precedence).
   On success: returns true, stores the answer in *result.
   On failure (bad input or divide by zero): returns false, and
   writes a short error message into err_msg. */
bool tool_calculate(const char *expr, double *result, char *err_msg, int err_msg_size);

#endif
