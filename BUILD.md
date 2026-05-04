## Building the ApeSDK

ApeSDK has been built with a variety of development environments including CodeWarrior, Visual Studio and Xcode. The current repository is packaged around the command-line Simulated Ape build.

## Command Line

The command-line build script builds the `simape` binary on macOS and Linux:

```sh
./build.sh --output simape
./simape --self-test
```

The script respects the standard `CC`, `CFLAGS`, `CPPFLAGS`, `LDFLAGS`, and `LIBS` environment variables. It can also install into a staging prefix:

```sh
./build.sh --prefix /tmp/apesdk
```

## Homebrew Release

The Homebrew formula expects the v0.708 source package at:

```text
https://github.com/barbalet/apesdk/releases/download/v0.708/apesdk-0.708.tar.gz
```

Create the release package and checksum with:

```sh
./packaging/create-release-tarball.sh 0.708
```

Upload `dist/apesdk-0.708.tar.gz` to the GitHub release for tag `v0.708`. The generated `dist/apesdk-0.708.tar.gz.sha256` file contains the checksum used by `Formula/apesdk.rb`.

## Contact

To contact Tom Barbalet directly email;

   `barbalet at gmail dot com`
