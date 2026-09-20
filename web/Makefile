ROOT := ..
SITE := web
EMCC ?= emcc
ENGINE_SOURCES := $(wildcard $(ROOT)/toolkit/*.c) $(wildcard $(ROOT)/script/*.c) $(wildcard $(ROOT)/render/*.c) $(wildcard $(ROOT)/sim/*.c) $(wildcard $(ROOT)/entity/*.c) $(wildcard $(ROOT)/universe/*.c) $(wildcard $(ROOT)/gui/*.c)
COMMON_FLAGS := -DWASM_BUILD -DCOMMAND_LINE_EXPLICIT -O3 -Wall -Wextra -I$(ROOT)/toolkit -I$(ROOT)/script -I$(ROOT)/render -I$(ROOT)/sim -I$(ROOT)/entity -I$(ROOT)/universe
EXPORTS := _apesdk_start,_apesdk_cycle,_apesdk_draw,_apesdk_mouse,_apesdk_mouse_up,_apesdk_key,_apesdk_key_up,_apesdk_menu,_apesdk_population,_apesdk_selected_x,_apesdk_selected_y,_apesdk_stop

.PHONY: build serve clean check-toolchain
build: check-toolchain $(SITE)/apesdk.js
check-toolchain:
	@command -v $(EMCC) >/dev/null || { echo 'emcc is not installed/activated. Source emsdk_env.sh only configures an installed SDK.'; echo 'Install the compiler with: TMPDIR="/Volumes/500GB HDD/apesdk-emsdk-tmp" "/Volumes/500GB HDD/apesdk-emsdk/emsdk" install sdk-releases-f04ea239d533260dd1db760dd2d668d5f9a88d6b-64bit'; echo 'Then activate it with the same emsdk path and source emsdk_env.sh again.'; exit 1; }
$(SITE)/apesdk.js: Makefile src/apesdk_web.c $(ENGINE_SOURCES)
	$(EMCC) $(COMMON_FLAGS) src/apesdk_web.c $(ENGINE_SOURCES) -o $@ -sWASM=1 -sMODULARIZE=1 -sEXPORT_ES6=1 -sENVIRONMENT=web -sEXPORTED_FUNCTIONS='[$(EXPORTS)]' -sEXPORTED_RUNTIME_METHODS='["cwrap","HEAPU8"]' -sINITIAL_MEMORY=268435456 -sEMULATE_FUNCTION_POINTER_CASTS=1 -sNO_EXIT_RUNTIME=1
serve: build
	cd $(SITE) && python3 -m http.server 8080
clean:
	rm -f $(SITE)/apesdk.js $(SITE)/apesdk.wasm $(SITE)/apesdk.worker.js
