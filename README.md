# ECE 309 Project 1 — LLM Mini-Harness in C

A minimal terminal-based LLM agent harness built via vibe coding
(Specification-Driven Development + AI-assisted code generation).

The harness simulates the core plumbing between an LLM and the operating
system: a REPL core loop, bounded conversation context management, and
tool execution — all driven by a deterministic mock model, so it needs no
API key and no network access.

## Build

```sh
cd src
gcc -std=c11 -Wall -Wextra -g -o harness main.c context.c model.c tool.c
```

## Run

```sh
./harness
```

```
Mini LLM Harness. Type '/exit' or '/quit' to leave.
Try: hello   OR   calc 3 + 4

you> hello
assistant> You said: hello
you> calc 3 + 4
assistant> 3 + 4 = 7
you> calc 10 / 0
assistant> Error: division by zero
you> /exit
assistant> Goodbye.
```

## Test

```sh
cd src
gcc -std=c11 -Wall -Wextra -g -o test_context test_context_manual.c context.c
bash ../tests/test_harness.sh
```

The test script checks: normal-message echo behavior, calculator
correctness (including operator precedence and divide-by-zero), the
5-turn context window (capping and eviction of the oldest turn), and,
via Valgrind, that the program has zero memory leaks.

## Project layout

```
src/
  main.c                  core REPL loop (I/O, orchestration only)
  context.h / context.c   fixed-capacity (5-turn) conversation history
  model.h / model.c       mock LLM: canned echo reply + tool-call dispatch
  tool.h / tool.c         calculator tool (+ - * /, standard precedence)
  test_context_manual.c   standalone check of context eviction behavior
  test_tool_manual.c      standalone check of calculator correctness
tests/
  test_harness.sh         automated test suite (state mgmt + memory safety)
vibe_coding_log.md         SDD process log: spec, prompts, bugs found & fixed
```

## Specification (decided before any AI prompting)

| Decision | Choice |
|---|---|
| Non-math input | Echoed back as `"You said: <message>"` |
| Tool trigger | `calc <expression>`, e.g. `calc 3 + 4` |
| Supported ops | `+ - * /`, standard precedence, no parentheses |
| Divide by zero | Prints an error, program keeps running (never crashes) |
| Context size | Last 5 turns (user message + assistant reply) |
| Context overflow | Oldest turn automatically evicted/freed |
| Exit | `/exit` or `/quit`, all memory freed before exit |

See `vibe_coding_log.md` for the full process: the exact prompts used to
generate each module, and two real bugs the generated code had (an
operator-precedence bug in the calculator, and a false-positive in the
test script itself) that were caught by actually running and verifying
the output rather than trusting the code on inspection.
