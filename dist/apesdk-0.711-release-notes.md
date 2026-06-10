# ApeSDK 0.711

ApeSDK 0.711 refreshes the Simulated Ape release line with the current engine
identity, macOS packaging, and documentation pointers aligned around the new
version. The user-facing Simulated Ape application remains the primary release
artifact: the `sim-mac` target was built in Release configuration for both Apple
Silicon and Intel Macs, ad-hoc signed, and packaged as separate DMG downloads.

This release is mainly a consistency and release-readiness pass rather than a
large behavioral change. The core C version constants now report `Simulated Ape
0.711` and `VERSION_NUMBER` 711, the macOS Xcode marketing version matches
0.711, and the deterministic unit test expectations were updated to protect the
new version boundary. The README, release checklist, book overview, first
chapter, release chapter, and Simulated Universe bridge handshake defaults were
also brought forward to the 0.711 identity so source readers, generated
fixtures, and release consumers see the same value.

Validation for the release included the full deterministic `./test.sh` suite,
Apple Silicon and Intel Release builds of `toolchains/sim-mac`, architecture
checks for both app binaries, source archive creation, and SHA-256 checksums for
the release artifacts.
