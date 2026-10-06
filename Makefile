.PHONY: build test test-gcc test-clang test-sanitized coverage test-nightly clean

# One public entry point for all C test lanes.  The legacy module scripts remain
# compatibility wrappers, but test.sh is the only runner used by CI.
build:
	./build.sh

test:
	./test.sh

test-gcc:
	CC=gcc ./test.sh

test-clang:
	CC=clang ./test.sh

test-sanitized:
	CC=clang TEST_WARNINGS='-Wall -Wextra' SANITIZE=1 ./test.sh

coverage:
	COVERAGE=1 ./test.sh

# Deliberately separate from the PR lane: repeat deterministic scenarios and
# allow fuzz/property executables to be added without slowing every push.
test-nightly:
	TEST_SCENARIO_SEEDS='1 7 42 65535' ./test.sh

clean:
	find . -name '*.o' -o -name '*.gcda' -o -name '*.gcno' | xargs rm -f
