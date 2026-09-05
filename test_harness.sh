#!/usr/bin/env bash
#
# test_harness.sh
# Tests the mini LLM harness for correct behavior, correct state
# management (5-turn context cap + eviction), and memory safety.
#
# Run from the src/ directory after building, e.g.:
#   gcc -std=c11 -Wall -Wextra -g -o harness main.c context.c model.c tool.c
#   gcc -std=c11 -Wall -Wextra -g -o test_context test_context_manual.c context.c
#   bash ../tests/test_harness.sh

PASS=0
FAIL=0

check() {
    # $1 = description, $2 = the text to search in, $3 = the text we expect to find
    if echo "$2" | grep -qF "$3"; then
        echo "  [PASS] $1"
        PASS=$((PASS+1))
    else
        echo "  [FAIL] $1   (expected to find: \"$3\")"
        FAIL=$((FAIL+1))
    fi
}

echo "== Test 1: normal message gets echoed =="
OUT=$(printf 'hello\n/exit\n' | ./harness)
check "echoes plain input" "$OUT" "You said: hello"

echo "== Test 2: calculator works =="
OUT=$(printf 'calc 3 + 4\ncalc 4 * 2 + 1\ncalc 10 / 0\n/exit\n' | ./harness)
check "basic addition" "$OUT" "= 7"
check "precedence (multiply before add)" "$OUT" "= 9"
check "divide by zero doesn't crash, gives error" "$OUT" "division by zero"

echo "== Test 3: context caps at 5 turns and evicts oldest =="
# We test the context module directly (not through the CLI), since the
# harness intentionally has no /history command to inspect state through.
OUT=$(./test_context)
check "context caps at exactly 5" "$OUT" "count = 5"
check "oldest turns (msg1) got evicted" "$OUT" "msg1 and msg2 should be GONE"
if echo "$OUT" | grep -q "user=msg1 "; then
    echo "  [FAIL] msg1 should NOT still be stored in any slot"
    FAIL=$((FAIL+1))
else
    echo "  [PASS] msg1 fully evicted, not found in current storage"
    PASS=$((PASS+1))
fi

echo "== Test 4: no memory leaks (Valgrind) =="
VG_LOG=$(mktemp)
printf 'hello\ncalc 3 + 4\nmsg3\nmsg4\nmsg5\nmsg6\nmsg7\n/exit\n' \
    | valgrind --leak-check=full --error-exitcode=99 --log-file="$VG_LOG" ./harness >/dev/null 2>&1
VG_STATUS=$?

if [ "$VG_STATUS" -eq 99 ]; then
    echo "  [FAIL] valgrind detected errors:"
    cat "$VG_LOG"
    FAIL=$((FAIL+1))
elif grep -q "All heap blocks were freed -- no leaks are possible" "$VG_LOG"; then
    echo "  [PASS] no memory leaks -- all heap blocks freed"
    PASS=$((PASS+1))
else
    echo "  [FAIL] possible leak detected, see log:"
    grep -A4 "LEAK SUMMARY" "$VG_LOG"
    FAIL=$((FAIL+1))
fi
rm -f "$VG_LOG"

echo ""
echo "===================================="
echo "Results: $PASS passed, $FAIL failed"
echo "===================================="
[ "$FAIL" -eq 0 ]
