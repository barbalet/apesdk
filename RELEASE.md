# ApeSDK Release Procedure

This procedure releases the two supported desktop variants from
`maccatalyst/maccatalyst.xcodeproj`:

- **Native macOS** — the `sim-mac` scheme. This is the Mac-specific AppKit product.
- **Mac Catalyst** — the `ApeSim-MacCatalyst` scheme. This is the iOS-derived product packaged to run on macOS through Mac Catalyst.

They are separate products and must remain separate ZIP files. Do not replace the native package with the Catalyst package, or combine their app bundles in one archive. `toolchains/sim-mac` remains a reference project; release builds come from `maccatalyst`.

Set `VERSION` without a leading `v`. For example, engine version `0.711` uses `VERSION=0.711` and Git tag `v0.711`.

## 1. Prepare the Version

Update the engine version in `sim/sim.h` (`SHORT_VERSION_NAME` and `VERSION_NUMBER`) and the `MARKETING_VERSION` values for both `sim-mac` and `ApeSim-MacCatalyst` in `maccatalyst/maccatalyst.xcodeproj/project.pbxproj`. The same value belongs in both desktop artifacts and their filenames.

Before packaging, run the C suite and the two hosted Swift Testing suites:

```bash
./test.sh

xcodebuild \
  -project maccatalyst/maccatalyst.xcodeproj \
  -scheme sim-mac \
  -configuration Debug \
  -destination 'platform=macOS' \
  -derivedDataPath .build/test-native-macos \
  CODE_SIGNING_ALLOWED=NO \
  test

xcodebuild \
  -project maccatalyst/maccatalyst.xcodeproj \
  -scheme ApeSim-MacCatalyst \
  -configuration Debug \
  -destination 'platform=macOS,variant=Mac Catalyst' \
  -derivedDataPath .build/test-maccatalyst \
  CODE_SIGNING_ALLOWED=NO \
  test
```

The shared `maccatalyst/Tests/SimMacParityTests.swift` source is intentionally compiled by both test bundles: `SimMacTests` for native macOS and `ApeSim-MacCatalystTests` for Catalyst. A release is not ready if either test command fails.

## 2. Build and Run-check Native macOS

From the repository root:

```bash
VERSION="<VERSION>"
mkdir -p dist

xcodebuild \
  -project maccatalyst/maccatalyst.xcodeproj \
  -scheme sim-mac \
  -configuration Release \
  -destination 'generic/platform=macOS' \
  -derivedDataPath .build/release-native-macos \
  CODE_SIGNING_ALLOWED=NO \
  build

NATIVE_APP='.build/release-native-macos/Build/Products/Release/Simulated Ape.app'
test -d "$NATIVE_APP"
codesign --force --deep --sign - "$NATIVE_APP"
open -W "$NATIVE_APP"
```

`open -W` verifies that macOS can launch the generated native product and that it returns after the application quits. Perform the normal visible smoke check before quitting: the `View`, `Terrain`, and `Control` windows appear; a cycle runs; and Quit closes the process.

For Developer ID distribution, replace the ad-hoc signing command with your Developer ID signing, notarization, and stapling flow before creating the ZIP.

```bash
ditto -c -k --keepParent "$NATIVE_APP" \
  "dist/simulated-ape-native-macos-${VERSION}.zip"
```

## 3. Build and Run-check Mac Catalyst

Build the dedicated Catalyst scheme separately. Its app bundle is a different product even though its displayed name is also `Simulated Ape`.

```bash
xcodebuild \
  -project maccatalyst/maccatalyst.xcodeproj \
  -scheme ApeSim-MacCatalyst \
  -configuration Release \
  -destination 'generic/platform=macOS,variant=Mac Catalyst' \
  -derivedDataPath .build/release-maccatalyst \
  CODE_SIGNING_ALLOWED=NO \
  build

CATALYST_APP='.build/release-maccatalyst/Build/Products/Release-maccatalyst/Simulated Ape.app'
test -d "$CATALYST_APP"
codesign --force --deep --sign - "$CATALYST_APP"
open -W "$CATALYST_APP"
```

The Catalyst run-check must launch the Catalyst app, verify its `View`, `Terrain`, and `Control` windows and a running simulation, then quit cleanly. It must not substitute the native product. Use Developer ID signing and notarization rather than ad-hoc signing for external distribution.

Package it independently:

```bash
ditto -c -k --keepParent "$CATALYST_APP" \
  "dist/simulated-ape-maccatalyst-${VERSION}.zip"
```

## 4. Create the Source Package

```bash
SRC_ROOT="apesdk-${VERSION}"
SRC_STAGE="$(mktemp -d)/${SRC_ROOT}"
rsync -a ./ "$SRC_STAGE"/ \
  --exclude .git \
  --exclude .build \
  --exclude dist \
  --exclude '*.xcuserstate' \
  --exclude xcuserdata \
  --exclude .DS_Store
ditto -c -k --keepParent "$SRC_STAGE" "dist/apesdk-src-${VERSION}.zip"
```

## 5. Verify and Publish

Verify that each archive contains exactly its intended app bundle and has a distinct checksum:

```bash
unzip -l "dist/simulated-ape-native-macos-${VERSION}.zip"
unzip -l "dist/simulated-ape-maccatalyst-${VERSION}.zip"

shasum -a 256 \
  "dist/simulated-ape-native-macos-${VERSION}.zip" \
  "dist/simulated-ape-maccatalyst-${VERSION}.zip" \
  "dist/apesdk-src-${VERSION}.zip"
```

Attach these three files to the GitHub release:

```text
dist/simulated-ape-native-macos-<VERSION>.zip
dist/simulated-ape-maccatalyst-<VERSION>.zip
dist/apesdk-src-<VERSION>.zip
```

After the final release commit and successful checks, create the matching tag:

```bash
git tag -a "v${VERSION}" -m "ApeSDK ${VERSION}"
git push origin "v${VERSION}"
```
