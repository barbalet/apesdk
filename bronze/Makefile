CC ?= cc
.PHONY: test clean

test:
	bash ./test.sh

# Linked by the ApeSDK test runner/source closure, not this lightweight target.
land-harness: test

clean:
	rm -f bronze_apesdk_adapter.o bronze_apesdk_adapter_test
