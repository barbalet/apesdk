CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra
DSL = dsl/brz_parser.c dsl/brz_dsl.c dsl/brz_kinds.c dsl/brz_util.c dsl/brz_vec.c
RUNTIME = runtime/brz_agent.c runtime/brz_land.c runtime/brz_port.c \
	runtime/brz_settlement.c runtime/brz_sim.c runtime/brz_world.c

# Compile-only boundary check. Linking belongs to the consuming ApeSDK
# toolchain, which supplies entity, land, and universe objects.
adapter-check:
	$(CC) $(CFLAGS) -I../.. -c adapter/bronze_apesdk_adapter.c -o /tmp/bronze_apesdk_adapter.o

bronze-sim: $(DSL) $(RUNTIME) runtime/bronze_main.c
	$(CC) $(CFLAGS) -Idsl -Iruntime $(DSL) $(RUNTIME) runtime/bronze_main.c -lm -o bronze-sim

test: bronze-sim adapter-check
	./bronze-sim content/first_slice.bronze >/tmp/apesdk-bronze-first-slice.log
	./bronze-sim content/bronze_age_full.bronze >/tmp/apesdk-bronze-full.log
	bash ./test.sh
	bash ./hybrid/test.sh
	bash ./actors/test.sh
	@echo "Bronze package scenarios and ApeSDK harness passed"

contract-test:
	python3 ./contract/check.py

land-harness: test

clean:
	rm -f bronze-sim bronze_apesdk_adapter.o bronze_apesdk_adapter_test

.PHONY: adapter-check bronze-sim test contract-test land-harness clean
