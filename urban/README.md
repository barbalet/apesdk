# PRA — Procedural Residential Architecture

Shared ApeSDK C11 component, extracted from the house, tree and fence generators used by
ApeSDK's `toolchains/urban-mac` Xcode project. No graphics library, SDK runtime,
or global state is required by the core library. The included browser host renders
the same C code compiled to WebAssembly.

```sh
make -C urban all test demo
make -C urban sanitize
urban/tests/validate-original.sh /path/to/apesdk /path/to/urbanscribble/urban
EMCC=/path/to/emcc urban/build-web.sh
node urban/tests/wasm.cjs
python3 -m http.server 8080 --directory urban/build/web
```

Open `http://localhost:8080` for the orbit/cutaway/walking preview. WASD moves,
arrows or dragging turns, E opens a nearby door. Movement uses C collision queries.
The demo supplies roof and door controls, deterministic seeds and JSON export.
`urban/build/pra --seed 372114642 --houses 4 --obj example.obj` exports a colored
triangle OBJ in metres, Y up. Without `--obj`, the CLI writes JSON to stdout.

The public API is in `include/pra.h`. Create a `pra_scene`, query rooms, change doors,
query capsule collision and shelter, and build an exterior or cutaway triangle
mesh. Free scenes with `pra_destroy` and meshes with `pra_mesh_free`. Pass a zeroed
`pra_mesh` to `pra_build_mesh`; free a previous successful mesh before reuse. A failed
build leaves the output untouched. Interleaved vertices contain position XYZ,
normal XYZ, and linear RGB. Rebase using a double precision origin before converting
to float. Scene queries use absolute metric positions.

## Original C validation

`vendor/urban/` retains verbatim originals. `PROVENANCE.json` records source
revisions and SHA-256 values. The Xcode project expects a sibling `urban` directory;
the available source checkout was `urbanscribble/urban`. The reference test compiles
those external original C files independently against the **full original current
ApeSDK toolkit**, and compares 10,000 seeds with the extracted code. It checks every
room coordinate, genetics, final PRNG state, tree profile/radius and fence segment.
It does not compare the extraction against itself. Source checkouts remain untouched.
The host error sink aborts if the original toolkit encounters an unexpected error.

Original house layouts are retained, including houses **without hallways**. No
hallway is inserted. `pra_room_points` and JSON room records are the original
transformed integer values, centered at zero; multiply by 0.07 metres and add the
house center to place them. Absent window/door records remain zero sentinels.
The renderer uses the captured pre-transform plan and the same sine-table transform,
avoiding small integer rounding cracks between adjoining walls. The metric adapter
uses 0.12 m walls about those boundaries; the original six-unit inset records are
retained for validation but are not treated as physical wall thickness (that would
leave the narrow original hallway impassable at this scale). Metric geometry may
differ from quantized source records by less than one source unit per coordinate.

Compatibility types use 64-bit integers on native and WASM. Room capacity is 16,
rather than the original 8, which overflows for some valid seeds. The reference shim
uses that capacity too; it does not change the original algorithm. Seed pair
`(130, 10)` is an explicit ten-room regression. The SDK also has fixed random
states: `(20600, 18)` returns 18 forever, so the original room-width rejection
loop never terminates. Production generation permits at most 512 rejections
per width and returns NULL if it cannot generate a bounded layout. The raw
original reference is tested at that fixed state without entering its endless
house loop. Other completing reference layouts retain exact integer parity. Shared walls are
split and deduplicated. Original side doors sometimes meet a perpendicular wall
exactly at a junction; the 3D adapter moves the portal to the nearest clear point on
the same wall. **Raw original room records remain unchanged.** Doors, collision and
mesh use that same corrected portal. Doors are 0.7 m wide and 2.1 m tall; capsules
must fit. Windows have solid glass for collision. Open panels swing into an adjacent room (preferring rooms over the central
hall); hinge selection keeps neighboring doorways clear when all doors open.
Panels remain collidable. Floor slabs and roofs block vertical capsule overlap,
including while the mesh is displayed as a cutaway. Tests use the game’s 0.3 m capsule radius.

## Yard and interior additions

Floors, wall apertures, frames, glass, door panels and removable flat roofs lift the
original plan into 3D. Each house gets a fenced plot, a road strip, a driveway aligned
with its actual entry, and three branching broadleaf trees plus one bush. Original
radial profiles and radii remain in the raw source records; the 3D plant
presentation uses shared rounded crowns and tapered, forked branches.
Multi-house spacing and seed progression are adapter policy, rather than the
original 4×4 neighborhood output. Each house records its exact source seed for
independent original-C comparison. Yard placement, driveway and 3D geometry are additions: the original road code does
not provide a house driveway primitive. Original `tree_init` and `fence_init` remain
available internally and are preserved, rather than silently modifying their rules.
The public yard calls `tree_create` explicitly for four trees.

The main engine now consumes this generator for its residential environment; see
the wasm-z Urban adapter. The optional `pra_seed_trees_by_position` adapter reseeds
yard tree shapes from absolute coordinates; standalone defaults retain original
seed progression and reference validation.

This component has no stair, furniture, terrain, persistence or networking system.
It is independent of the completed main-engine C migration. It does not replace
or qualify the main engine’s Metal/WebGPU clients. The demo's small WebGL2 host is
independent of the main game's Metal/WebGPU rendering contract.
