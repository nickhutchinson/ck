#!/bin/sh
# Run one regression command without changing its output or exit status.

suite=$(pwd)
command="$*"
printf 'RUN  [%s] %s\n' "$suite" "$command"
"$@"
status=$?

if [ "$status" -eq 0 ]; then
	result=PASS
else
	result=FAIL
fi
printf '%s [%s] status=%d %s\n' "$result" "$suite" "$status" "$command"

# CI supplies an absolute path so every suite appends to the same results file.
if [ -n "${CK_TEST_RESULTS:-}" ]; then
	printf '%s\t%d\t%s\n' "$suite" "$status" "$command" >> "$CK_TEST_RESULTS" ||
		printf 'Cannot append regression result to %s\n' "$CK_TEST_RESULTS" >&2
fi

exit "$status"
