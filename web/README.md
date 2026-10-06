# ApeSDK WebAssembly build

This directory packages the shared ApeSDK C simulation for the browser without copying engine sources. `src/apesdk_web.c` is the browser adapter; all simulation sources are compiled from the repository root.

Install and activate the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html), then run `cd web && make build`. Deploy `web/web/` as a static site. The server must serve `.wasm` as `application/wasm`.

## Fresh-machine recipe

On a machine with sufficient local disk space, clone or copy this repository,
then run the checked-in installer. It installs the SDK outside the repository
by default, verifies `emcc`, and creates the deployable graphical site:

```sh
cd /path/to/apesdk
bash web/install-emsdk.sh
```

The default SDK location is `$HOME/emsdk`; pass a different local location as
the first argument if desired. The SDK directory is a tool dependency, not a
Git artifact, so do not copy it with the repository.

On this Mac, the SDK checkout is on the external drive. `source emsdk_env.sh` is
only the final activation step; it does not download the compiler. Run this
once, allowing it to finish:

```sh
TMPDIR="/Volumes/500GB HDD/apesdk-emsdk-tmp" \
  "/Volumes/500GB HDD/apesdk-emsdk/emsdk" install \
  sdk-releases-f04ea239d533260dd1db760dd2d668d5f9a88d6b-64bit
"/Volumes/500GB HDD/apesdk-emsdk/emsdk" activate \
  sdk-releases-f04ea239d533260dd1db760dd2d668d5f9a88d6b-64bit
source "/Volumes/500GB HDD/apesdk-emsdk/emsdk_env.sh"
```

The build defines `WASM_BUILD`. Use that symbol for browser-specific shared-C changes; do not fork engine sources into this package.

The generated module exports the shared GUI lifecycle, drawing, mouse, keyboard,
and menu routes. The included site has interactive View, Terrain, and Control
canvases rendered by the same `gui/shared.c` and `gui/draw.c` path used by the
desktop wrappers; it is not a statistics-only viewer.
