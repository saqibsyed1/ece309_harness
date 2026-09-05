# Vibe Coding Log — ECE 309 Project 1: LLM Mini-Harness in C

This log documents how I used an AI assistant to build the harness, following
Specification-Driven Development: I decided the rules first, then prompted
the AI piece by piece, then verified each piece before moving to the next.

---

## 1. The Specification (decided before any prompting)

Before writing or generating any code, I made these decisions:

| Decision | Choice |
|---|---|
| Non-math input | Echo it back as `"You said: <message>"` |
| Tool trigger | User types `calc <expression>`, e.g. `calc 3 + 4` |
| Supported operations | `+ - * /` only, standard precedence (no parentheses) |
| Divide by zero | Print an error message, program keeps running (never crash) |
| Context size | Last 5 turns (1 turn = 1 user message + 1 assistant reply) |
| Context overflow | Automatically evict/delete the oldest turn when a 6th arrives |
| History command | None — kept the CLI simple, no `/history` |
| Exit | `/exit` or `/quit`, with all memory freed before the program ends |

This table is the actual spec I fed to the AI, module by module.

---

## 2. Prompts Used and What Came Back

### Prompt 1 — Context Management
> "Write a C header and source file (`context.h`, `context.c`) that stores
> the last 5 conversation turns (a turn = one user message + one assistant
> reply) in a fixed-size array acting as a ring buffer. When a 6th turn
> comes in, automatically overwrite/free the oldest one. Store the strings
> as heap-allocated copies. Provide `context_init()`, `context_add_turn()`,
> and `context_free()`. No memory leaks under any sequence of calls."

**Result:** `context.h` / `context.c` — a ring buffer using `strdup` to copy
strings, freeing a slot's old contents before overwriting it on wraparound.

**Verification:** Wrote a small standalone test (`test_context_manual.c`)
that adds 7 turns and prints what's left. Output confirmed `msg1` and `msg2`
were gone and `msg6`/`msg7` had taken their place, with the buffer correctly
capped at 5. Ran it under Valgrind: **0 leaks, 0 errors** ("15 allocs, 15
frees").

### Prompt 2 — Tool Execution
> "Write `tool.h` and `tool.c` in C. Add `tool_is_calc_request()` that checks
> if a string starts with `calc ` and returns the expression part. Add
> `tool_calculate()` that evaluates a simple math expression with `+ - * /`
> (standard precedence, no parentheses needed). If there's a divide by zero,
> return an error instead of crashing. Numbers can be multi-digit and have
> decimals."

**Result:** A tokenizer that splits the expression into numbers/operators,
then a two-pass evaluator (multiply/divide first, then add/subtract).

**Verification — bug found:** Wrote `test_tool_manual.c` and ran it.
`calc 4 * 2 + 1` returned **8** instead of the correct **9**. Traced the bug
to the code that "removes" a number/operator pair after combining them with
`*`/`/`: it shifted the operator array using the wrong loop bounds, which
overwrote a live operator with uninitialized/garbage memory from past the
end of the array. **Fix:** corrected the two shift loops to use the actual
array lengths (`nums` has `n` entries, `ops` has `n-1`). Re-ran the test —
`4 * 2 + 1` now correctly returns `9`, and all other cases (basic ops,
divide-by-zero, malformed input, decimals) still passed.

### Prompt 3 — Mock Model + Main Loop
> "Write `model.h`/`model.c` with `mock_model_generate(input, response_buffer)`
> that checks for a calc request and routes to the tool, otherwise echoes
> `\"You said: <input>\"`. Then write `main.c`: a loop that prompts `you>`,
> reads a line, calls the model, prints `assistant>`, stores the exchange in
> context, and exits cleanly on `/exit`/`/quit`, freeing memory first."

**Result:** `model.c` dispatches to the tool or falls back to an echo;
`main.c` is a plain read-generate-print-store loop.

**Verification:** Compiled all four files together with `-Wall -Wextra` —
zero warnings. Ran the compiled program with a piped test conversation
(greeting, two calculator calls including a divide-by-zero, then `/exit`)
and confirmed every response matched the spec. Ran the same session under
Valgrind: **0 leaks, 0 errors**, even after feeding 7 messages (forcing at
least one context eviction mid-run).

### Prompt 4 — AI-Generated Testing
> "Write a Bash script that tests my harness by piping input via stdin and
> checking output. Verify: (1) a normal message is echoed correctly, (2) the
> calculator works including divide-by-zero, (3) context caps at 5 turns and
> evicts the oldest, (4) run under Valgrind and fail if anything leaks."

**Result:** `test_harness.sh`. Since the harness intentionally has no
`/history` command (a deliberate design choice, not an oversight), the
script tests context behavior by running the standalone `test_context`
binary directly rather than trying to inspect state through the CLI.

**Verification — second bug found (in the test script itself):** First run
gave 7 passed / 1 failed. The failing check asserted the substring `"msg1"`
should not appear anywhere in the test output — but the test program's own
explanatory print statement (`"msg1 and msg2 should be GONE"`) contains the
literal text `"msg1"`, so the check was tripping over its own diagnostic
message, not real leftover data. **Fix:** narrowed the check to search for
the more specific pattern `"user=msg1 "` (the exact format used when a slot
actually holds that value), which only matches if msg1 were genuinely still
stored. Re-ran: **8/8 passed.**

---

## 3. Bugs Caught During Verification (summary)

| # | Where | What went wrong | How it was caught | Fix |
|---|---|---|---|---|
| 1 | `tool.c`, `tool_calculate()` | Array-shift loop used wrong bounds when removing a consumed number/operator pair after a `*`/`/`, corrupting a later operator with garbage memory. Silently gave `4 * 2 + 1 = 8` instead of `9`. | Manual test script with known expected answers — the wrong number in the output was the tell. | Rewrote the two shift loops with correct array-length bounds; re-verified all test cases. |
| 2 | `test_harness.sh`, Test 3 | Leak-of-abstraction: the test checked for the raw substring `"msg1"`, which also matched the test program's own diagnostic sentence, producing a false failure. | Ran the script and read the actual failure output rather than assuming the script itself was correct. | Changed the check to match the specific stored-value format (`"user=msg1 "`) instead of a bare substring. |

Neither bug was found by reading the code and deciding it "looked right" —
both were only caught by actually compiling and running the code and
checking the real output against expected values. That's the core practice
this project is meant to teach: generated code (and generated tests) are
drafts, not final answers, until verified.

---

## 4. Final Verification Results

- All source files compile with `gcc -std=c11 -Wall -Wextra -g` — **zero
  warnings**.
- `test_harness.sh`: **8/8 checks passing** (echo behavior, calculator
  correctness including precedence and divide-by-zero, context capping and
  eviction, Valgrind leak check).
- Valgrind, run separately against both the standalone context test and a
  full 7-message harness session: **0 errors, 0 bytes leaked** in both
  cases ("All heap blocks were freed -- no leaks are possible").

## 5. Files Produced

```
src/
  context.h / context.c   -- 5-turn ring buffer for conversation history
  tool.h / tool.c         -- calculator tool (+ - * /, standard precedence)
  model.h / model.c       -- mock model: routes to tool or echoes input
  main.c                  -- the core loop (read, generate, print, store)
  test_context_manual.c   -- standalone verification of context behavior
  test_tool_manual.c      -- standalone verification of calculator behavior
tests/
  test_harness.sh         -- automated test suite (state + memory safety)
```
