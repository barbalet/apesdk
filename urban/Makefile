CC ?= cc
AR ?= ar
CFLAGS ?= -O2 -g
BUILD ?= build
WARN = -std=c11 -Wall -Wextra -Werror -Wpedantic
CPPFLAGS += -Iinclude -Isrc -I../toolkit
SOURCES = src/pra.c src/mesh.c src/house.c src/tree.c src/fence.c
OBJECTS = $(SOURCES:src/%.c=$(BUILD)/%.o) $(addprefix $(BUILD)/sdk-,math.o vect.o memory.o)
.PHONY: all test sanitize clean demo
all: $(BUILD)/libpra.a $(BUILD)/pra
$(BUILD):
	mkdir -p $@
$(BUILD)/%.o: src/%.c include/pra.h src/pra_internal.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARN) -c $< -o $@
$(BUILD)/sdk-%.o: ../toolkit/%.c ../toolkit/toolkit.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -DAPESDK_URBAN_ONLY=1 $(WARN) -c $< -o $@
$(BUILD)/libpra.a: $(OBJECTS)
	rm -f $@
	$(AR) rcs $@ $^
$(BUILD)/pra: src/main.c $(BUILD)/libpra.a
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARN) $< $(BUILD)/libpra.a -lm -o $@
$(BUILD)/test: tests/test.c $(BUILD)/libpra.a
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARN) $< $(BUILD)/libpra.a -lm -o $@
test: $(BUILD)/test
	./$(BUILD)/test
sanitize:
	$(MAKE) test BUILD=build/sanitize CFLAGS='-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer'
demo: all
	./$(BUILD)/pra --houses 4 --obj $(BUILD)/exterior.obj
	./$(BUILD)/pra --houses 4 --open --cutaway --obj $(BUILD)/interior.obj
	./$(BUILD)/pra --houses 4 > $(BUILD)/scene.json
clean:
	rm -rf build

$(BUILD)/mesh.o: include/pra_plants.h
