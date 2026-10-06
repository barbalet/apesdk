CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra
RUNTIME = runtime/brz_agent.c runtime/brz_dsl.c runtime/brz_kinds.c \
	runtime/brz_land.c runtime/brz_parser.c runtime/brz_port.c \
	runtime/brz_settlement.c runtime/brz_sim.c runtime/brz_util.c \
	runtime/brz_vec.c runtime/brz_world.c

# Compile-only boundary check. Linking belongs to the consuming ApeSDK
# toolchain, which supplies entity, land, and universe objects.
adapter-check:
	$(CC) $(CFLAGS) -I../.. -c adapter/bronze_apesdk_adapter.c -o /tmp/bronze_apesdk_adapter.o

bronze-sim: $(RUNTIME) runtime/bronze_main.c
	$(CC) $(CFLAGS) -Iruntime $(RUNTIME) runtime/bronze_main.c -lm -o bronze-sim

test: bronze-sim adapter-check
	./bronze-sim content/first_slice.bronze >/tmp/apesdk-bronze-first-slice.log
	./bronze-sim content/bronze_age_full.bronze >/tmp/apesdk-bronze-full.log
	bash ./test.sh
	@echo "Bronze package scenarios and ApeSDK harness passed"

land-harness: test

clean:
	rm -f bronze-sim bronze_apesdk_adapter.o bronze_apesdk_adapter_test

.PHONY: adapter-check bronze-sim test land-harness clean
