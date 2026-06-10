# ApeSDK Release Procedure

This checklist describes the release flow for ApeSDK when shipping the macOS Simulated Ape application built from `toolchains/sim-mac`. It creates Apple Silicon and Intel DMG packages from the `sim-mac` Xcode project, plus a source archive for the exact tagged source used to build those artifacts.

Before starting, decide the new release number and use it as the `VERSION` input throughout this checklist. Set `VERSION` without a leading `v`; the Git tag adds the leading `v` separately. For example, the current Simulated Ape engine version `0.711` uses `VERSION=0.711` and tag `v0.711`.

## 1. Prepare the Version

Update the Simulated Ape engine version in `sim/sim.h`:

```text
SHORT_VERSION_NAME
VERSION_NUMBER
```

Update the Xcode marketing version in `toolchains/sim-mac/sim-mac.xcodeproj/project.pbxproj` for the `sim-mac` target:

```text
MARKETING_VERSION = <VERSION>;
```

Use the same `VERSION` value in artifact names without the leading `v`. The Xcode target and scheme remain `sim-mac`, but the built app bundle and executable are `Simulated Ape`.

## 2. Write the Release Synopsis

Create an approximately 200-word synopsis for this version before packaging the release. Summarize the user-facing Simulated Ape changes first, then call out major ApeSDK, file-format, source-compatibility, or platform changes that matter to downstream users. Use this synopsis as the GitHub release description.

## 3. Tag the Source

After the version number is decided and the final release commit is ready, tag the source code with the matching version number. The tag must point at the exact commit used to build the DMGs and source archive.

```bash
VERSION="<VERSION>"
git tag -a "v${VERSION}" -m "ApeSDK ${VERSION}"
git push origin "v${VERSION}"
```

If the release version changes, update `VERSION` and recreate the tag before publishing it.

## 4. Build Apple Silicon

From the repository root:

```bash
mkdir -p dist
VERSION="<VERSION>"
xcodebuild \
  -project toolchains/sim-mac/sim-mac.xcodeproj \
  -scheme sim-mac \
  -configuration Release \
  -destination "generic/platform=macOS" \
  -derivedDataPath .build/release-derived-data-arm64 \
  ARCHS=arm64 \
  ONLY_ACTIVE_ARCH=NO \
  CODE_SIGNING_ALLOWED=NO \
  build
```

The unsigned build output is:

```text
.build/release-derived-data-arm64/Build/Products/Release/Simulated Ape.app
```

Ad-hoc sign the generated app if Developer ID signing is not available:

```bash
codesign --force --deep --sign - ".build/release-derived-data-arm64/Build/Products/Release/Simulated Ape.app"
```

If you have Developer ID and notarization credentials, sign and notarize instead of ad-hoc signing:

```bash
VERSION="<VERSION>"
codesign --force --deep --options runtime --timestamp --sign "$DEVELOPER_ID_APPLICATION" ".build/release-derived-data-arm64/Build/Products/Release/Simulated Ape.app"
ditto -c -k --keepParent ".build/release-derived-data-arm64/Build/Products/Release/Simulated Ape.app" "dist/simulated-ape-mac-silicon-${VERSION}-notary.zip"
xcrun notarytool submit "dist/simulated-ape-mac-silicon-${VERSION}-notary.zip" --keychain-profile "$NOTARY_PROFILE" --wait
xcrun stapler staple ".build/release-derived-data-arm64/Build/Products/Release/Simulated Ape.app"
```

Package the DMG:

```bash
VERSION="<VERSION>"
hdiutil create \
  -volname "Simulated Ape ${VERSION} Apple Silicon" \
  -srcfolder ".build/release-derived-data-arm64/Build/Products/Release/Simulated Ape.app" \
  -format UDZO \
  -ov \
  "dist/simulated-ape-mac-silicon-${VERSION}.dmg"
```

Verify the architecture:

```bash
lipo -info ".build/release-derived-data-arm64/Build/Products/Release/Simulated Ape.app/Contents/MacOS/Simulated Ape"
```

## 5. Build Intel

```bash
VERSION="<VERSION>"
xcodebuild \
  -project toolchains/sim-mac/sim-mac.xcodeproj \
  -scheme sim-mac \
  -configuration Release \
  -destination "generic/platform=macOS" \
  -derivedDataPath .build/release-derived-data-x86_64 \
  ARCHS=x86_64 \
  ONLY_ACTIVE_ARCH=NO \
  CODE_SIGNING_ALLOWED=NO \
  build
```

The unsigned build output is:

```text
.build/release-derived-data-x86_64/Build/Products/Release/Simulated Ape.app
```

Ad-hoc sign the generated app if Developer ID signing is not available:

```bash
codesign --force --deep --sign - ".build/release-derived-data-x86_64/Build/Products/Release/Simulated Ape.app"
```

If you have Developer ID and notarization credentials, use the same signing, notary submission, and stapling flow described in the Apple Silicon section, with the x86_64 app path and an Intel-specific notary zip name.

Package the DMG:

```bash
VERSION="<VERSION>"
hdiutil create \
  -volname "Simulated Ape ${VERSION} Intel" \
  -srcfolder ".build/release-derived-data-x86_64/Build/Products/Release/Simulated Ape.app" \
  -format UDZO \
  -ov \
  "dist/simulated-ape-mac-intel-${VERSION}.dmg"
```

Verify the architecture:

```bash
lipo -info ".build/release-derived-data-x86_64/Build/Products/Release/Simulated Ape.app/Contents/MacOS/Simulated Ape"
```

## 6. Create the Source Package

Stage the source into a versioned folder so the archive has a stable top-level directory. Exclude VCS folders, build outputs, release artifacts, local Xcode user state, and Finder metadata.

```bash
VERSION="<VERSION>"
SRC_ROOT="apesdk-${VERSION}"
SRC_STAGE="$(mktemp -d)/${SRC_ROOT}"
rsync -a ./ "$SRC_STAGE"/ \
  --exclude .git \
  --exclude .build \
  --exclude dist \
  --exclude "*.xcuserstate" \
  --exclude "xcuserdata" \
  --exclude ".DS_Store"
ditto -c -k --keepParent "$SRC_STAGE" "dist/apesdk-src-${VERSION}.zip"
```

## 7. Verify Release Artifacts

```bash
VERSION="<VERSION>"
ls -lh \
  "dist/simulated-ape-mac-silicon-${VERSION}.dmg" \
  "dist/simulated-ape-mac-intel-${VERSION}.dmg" \
  "dist/apesdk-src-${VERSION}.zip"
shasum -a 256 \
  "dist/simulated-ape-mac-silicon-${VERSION}.dmg" \
  "dist/simulated-ape-mac-intel-${VERSION}.dmg" \
  "dist/apesdk-src-${VERSION}.zip"
```

Attach these files to the GitHub release:

```text
dist/simulated-ape-mac-silicon-<VERSION>.dmg
dist/simulated-ape-mac-intel-<VERSION>.dmg
dist/apesdk-src-<VERSION>.zip
```
