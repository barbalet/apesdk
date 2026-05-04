#!/bin/sh

set -eu

VERSION=${1:-}
if [ -z "$VERSION" ]
then
    VERSION=$(cat VERSION)
fi

ROOT="apesdk-$VERSION"
DIST_DIR="dist"
STAGE="$DIST_DIR/$ROOT"
ARCHIVE="$DIST_DIR/$ROOT.tar.gz"

rm -rf "$STAGE" "$ARCHIVE" "$ARCHIVE.sha256"
mkdir -p "$STAGE"

for file in README.md BUILD.md LICENSE VERSION build.sh longterm.c
do
    cp "$file" "$STAGE/$file"
done

for dir in toolkit script render sim entity universe
do
    find "$dir" -type f \( -name '*.c' -o -name '*.h' \) | sort | while IFS= read -r file
    do
        mkdir -p "$STAGE/$(dirname "$file")"
        cp "$file" "$STAGE/$file"
    done
done

chmod 755 "$STAGE/build.sh"

COPYFILE_DISABLE=1 tar -czf "$ARCHIVE" -C "$DIST_DIR" "$ROOT"

if command -v sha256sum >/dev/null 2>&1
then
    sha256sum "$ARCHIVE" > "$ARCHIVE.sha256"
else
    shasum -a 256 "$ARCHIVE" > "$ARCHIVE.sha256"
fi

cat "$ARCHIVE.sha256"
rm -rf "$STAGE"
