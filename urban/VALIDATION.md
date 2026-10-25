# PRA validation

Validated locally on the owner-supported Apple M4 Pro Mac, 2026-10-25.

- `make -C urban all test demo`: strict C11, warnings as errors, static library and
  command-line tool; 2,000 deterministic seeds, 366 original no-hallway layouts,
  up to nine rooms; door opening/closing and capsule headroom with the game’s 0.3 m radius,
  simultaneous open-door clearance, shelter, mesh
  normals/finiteness, JSON size/truncation, invalid inputs, explicit ten-room capacity and fixed-PRNG rejection, 64-house bounds and
  distant-coordinate rebasing pass. Both exterior and open-door cutaway OBJ
  examples and a JSON scene are generated.
- `make -C urban sanitize`: those C assertions pass under AddressSanitizer and
  UndefinedBehaviorSanitizer.
- `urban/tests/validate-original.sh /Users/barbalet/apesdk
  /Users/barbalet/urbanscribble/urban`: 10,000 seeds match independently compiled
  original Urban generators linked against the full current ApeSDK toolkit.
  The comparison also passed with both production and reference code instrumented
  by address/undefined-behavior sanitizers. This validates original room records,
  genetics, seeds, tree profiles and fence segments, not new 3D adapter geometry.
- `EMCC=/Users/barbalet/apesdk-emsdk-local/upstream/emscripten/emcc
  urban/build-web.sh` and `node urban/tests/wasm.cjs`: actual compiled WebAssembly
  matches native C for fourteen scenes, exact original room data, seeds and door
  state. Metric doorway centers agree within 1e-7 metres; small floating-point
  differences are allowed. All four mesh flags produce finite triangle buffers.
- Actual embedded-browser preview: exterior roof, driveway, front fence gap,
  radial-profile trees, windows and walls render. Roof removal and door opening
  reveal the original three-room no-hallway layout. Retained local inspection
  image: `../artifacts/pra-cutaway.png`. This is a visual component check, not the
  original game's platform or duration qualification.

The single production generator is in `src/`. The optional original-C comparison
reads the external checkout; no verbatim fixture copies are stored here.
Toolkit functions are compiled directly with `APESDK_URBAN_ONLY=1`, while normal
SDK builds keep their full existing behavior. See `README.md` and `PROVENANCE.json`.
