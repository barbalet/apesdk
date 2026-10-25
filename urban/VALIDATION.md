# PRA validation

Validated locally on the owner-supported Apple M4 Pro Mac, 2026-10-18.

- `make -C pra all test demo`: strict C11, warnings as errors, static library and
  command-line tool; 2,000 deterministic seeds, 366 original no-hallway layouts,
  up to nine rooms; door opening/closing and capsule headroom with the game’s 0.3 m radius,
  simultaneous open-door clearance, shelter, mesh
  normals/finiteness, JSON size/truncation, invalid inputs, explicit ten-room capacity and fixed-PRNG rejection, 64-house bounds and
  distant-coordinate rebasing pass. Both exterior and open-door cutaway OBJ
  examples and a JSON scene are generated.
- `make -C pra sanitize`: those C assertions pass under AddressSanitizer and
  UndefinedBehaviorSanitizer.
- `pra/tests/validate-original.sh /Users/barbalet/apesdk
  /Users/barbalet/urbanscribble/urban`: 10,000 seeds match independently compiled
  original Urban generators linked against the full current ApeSDK toolkit.
  The comparison also passed with both production and reference code instrumented
  by address/undefined-behavior sanitizers. This validates original room records,
  genetics, seeds, tree profiles and fence segments, not new 3D adapter geometry.
- `EMCC=/Users/barbalet/apesdk-emsdk-local/upstream/emscripten/emcc
  pra/build-web.sh` and `node pra/tests/wasm.cjs`: actual compiled WebAssembly
  matches native C for fourteen scenes, exact original room data, seeds and door
  state. Metric doorway centers agree within 1e-7 metres; small floating-point
  differences are allowed. All four mesh flags produce finite triangle buffers.
- Actual embedded-browser preview: exterior roof, driveway, front fence gap,
  radial-profile trees, windows and walls render. Roof removal and door opening
  reveal the original three-room no-hallway layout. Retained local inspection
  image: `../artifacts/pra-cutaway.png`. This is a visual component check, not the
  original game's platform or duration qualification.

`src/` adaptations and original verbatim `vendor/urban/` files are distinguished
in `README.md` and `PROVENANCE.json`. The main engine’s separate C migration and release acceptance are documented
in ../C-MIGRATION.md and ../RELEASE-GATES.md; these component results do not
qualify that application.
