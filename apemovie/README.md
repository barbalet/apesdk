# apemovie

`apemovie` is a standalone movie generator for the ApeSDK Terrain screen. It
uses the existing `shared_draw(NUM_TERRAIN, width, height, ...)` UI-rendering
path to produce frames, then streams BGRA raw video into `ffmpeg` for H.264 MP4
encoding. The utility is intentionally isolated under `apemovie/`; the existing
simulation, rendering, and UI source files are compiled as inputs but are not
modified.

## Build

```sh
./apemovie/build.sh
```

The build creates:

```text
apemovie/apemovie
```

The utility is C. It expects `ffmpeg` at `/opt/homebrew/bin/ffmpeg`,
`/usr/local/bin/ffmpeg`, `/usr/bin/ffmpeg`, or on `PATH`. You can also pass an
explicit encoder path with `--ffmpeg PATH`.

## Default Run

```sh
./apemovie/apemovie --output terrain-20-days.mp4
```

Defaults:

- `--days 20`
- `--width 2048`
- `--height 1536`
- `--fps 30`
- `--minutes-per-frame 5`
- `--seed 0x12738291`
- `--bitrate 12000000`

The simulation advances one minute per ApeSDK `sim_cycle()`. With the default
five simulated minutes per movie frame, a 20-day run records 5,760 frames and
produces about 3.2 minutes of video at 30 fps.

## Longer Or Denser Runs

Minute-by-minute output for 20 simulated days:

```sh
./apemovie/apemovie \
  --output terrain-20-days-minute.mp4 \
  --days 20 \
  --minutes-per-frame 1
```

A longer time-lapse, such as 120 simulated days:

```sh
./apemovie/apemovie \
  --output terrain-120-days.mp4 \
  --days 120 \
  --minutes-per-frame 15
```

Higher resolution output:

```sh
./apemovie/apemovie \
  --output terrain-4096x3072.mp4 \
  --width 4096 \
  --height 3072 \
  --days 20
```

H.264 output requires even dimensions.

## Saved Simulations And Scripts

Load a saved simulation before recording:

```sh
./apemovie/apemovie --load saved.sim --output loaded-terrain.mp4
```

Load an ApeScript file before recording:

```sh
./apemovie/apemovie --script experiment.txt --output scripted-terrain.mp4
```

Birth/death debug output from the simulation is suppressed by default so long
movie runs do not flood the terminal. Add `--events` to keep those messages.

## Options

```text
--output PATH
--days N
--width N
--height N
--fps N
--minutes-per-frame N
--frames N
--seed N
--load PATH
--script PATH
--bitrate N
--ffmpeg PATH
--events
--help
```
