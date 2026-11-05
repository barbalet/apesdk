# Agent inside a township

The old PNG captures were removed because they no longer represent the
current renderer. The optional capture workflow remains available.

For each bundled scenario, the generated selected simulation agent was placed
at the first building's recorded entrance. The map uses a 32-cell span;
Terrain uses the normal agent-follow camera with no selected-site override,
at bearings 0.7 and 3.84 radians. This is a third-person follow view.
The original agent location is restored after capture.

Reproduce with an existing output directory:

```sh
BRONZE_TOWNSHIP_EVIDENCE=/tmp/bronze-township-agent bash bronze/actors/test.sh
```

The optional evidence export adds `*-agent-map.ppm`, `*-agent-terrain.ppm`
and `*-agent-terrain-reverse.ppm`. These upright frames come directly from the simulation renderer.

All actor, session, work, extended-scenario and township checks passed.
Entrances: Example Large (479,143), First Slice and Vertical Slice (302,151),
Bronze Age Full (329,55), each at building 129 in township 1.

Buildings and mines now render as separate geometry above the ground, and
water uses a partially transparent dark blue surface tint. Use a fresh capture
or the live application to assess the current appearance.
