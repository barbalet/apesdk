# Chapter 13 - Building And Running The Python Library

The Python directory is a small command-line host for ApeSDK. It is useful when
an experiment, notebook, automation tool, or another Python program needs to
drive the normal simulation console without maintaining a second copy of the C
engine.

The important design rule is simple: ApeSDK has one canonical C source tree.
The Python build uses the source at the repository root; it does not copy
`toolkit`, `sim`, `entity`, `universe`, or their headers into `python/`.

## What The Python Build Produces

[`python/mylib.sh`](../python/mylib.sh) builds the shared library:

```text
python/apesim.so
```

On macOS this is a Mach-O dynamic library. On Linux it is an ELF shared
library. In both cases, Python loads it through `ctypes` and talks to a small C
adapter. The produced library is a standalone runtime artifact: once built, the
Python launcher uses `apesim.so` rather than compiling C on every run.

The build itself deliberately uses the top-level source directories:

```text
toolkit/   script/   render/graph.c   sim/   entity/   universe/   longterm.c
```

This avoids the most dangerous form of duplication: a reduced simulation that
quietly falls behind the main ApeSDK engine.

## Building The Library

From the repository root:

```sh
cd python
./mylib.sh
```

The script determines the repository root from its own location, compiles the
canonical C sources, and writes `apesim.so` beside the Python launcher. It uses
`-dynamiclib` on macOS and `-shared` on Linux, together with `-fPIC`, `-lz`,
`-lm`, and `-lpthread`.

The standard native command-line build remains separate:

```sh
./build.sh
```

That command builds the regular `simape` executable. The Python build creates a
library instead, but both use the same simulation implementation.

## The `PYTHON_BUILD` Define

The Python build command includes:

```sh
-DPYTHON_BUILD
```

For the C preprocessor, that is equivalent to placing this line before the
source is compiled:

```c
#define PYTHON_BUILD
```

It does not create a second source variant. It selects a small integration layer
inside the canonical source, guarded with:

```c
#ifdef PYTHON_BUILD
/* Python-specific adapter */
#endif
```

When the ordinary top-level executable is built, `PYTHON_BUILD` is absent and
the normal `main()` entry point in [`longterm.c`](../longterm.c) is compiled.
When `mylib.sh` builds the shared library, the guarded code exports these four
functions instead:

```c
void  python_init(void);
n_int python_quit_check(void);
void  python_check_string(n_string incoming);
void  python_close(void);
```

`python_init()` configures the console callbacks, prints the same ApeSDK
version banner as the normal command-line executable, and creates a simulation.
`python_check_string()` supplies one console line. `python_quit_check()` tells
the launcher when `quit` has been processed, and `python_close()` releases the
simulation.

The define also makes the console handoff safe for an interactive Python host.
The native console can use two workers: one may be busy running `run forever`
while the other receives `stop`. In the Python build, the adapter waits until a
submitted input line has been collected and waits for a worker slot when both
are briefly busy. This prevents an eagerly typed next command from overwriting
the previous command, while preserving the ability to stop a long-running
simulation.

## Running The Python Console

After building:

```sh
cd python
python3 apesim.py
```

The startup banner identifies the same ApeSDK version and build date as the
native command-line program. Commands are the normal ApeSDK console commands:

```text
help
sim
run forever
stop
quit
```

For example, `run forever` starts the simulation, `stop` interrupts it, and
`sim` reports the current world state. The Python launcher converts each typed
line into a writable byte buffer and passes it to the library with `ctypes`.

## Embedding In Another Python Program

The existing [`python/apesim.py`](../python/apesim.py) is intentionally small.
It demonstrates the full API contract: initialize, submit UTF-8 command text,
observe the quit state, and close. A program embedding the library should keep
the same order and should not call `python_close()` until it is finished with
the simulation.

The shared library is ignored by Git because it is a platform-specific build
artifact. The source of truth is the top-level C tree plus the small Python
adapter guarded by `PYTHON_BUILD`.

## Troubleshooting

If `python3 apesim.py` cannot load `apesim.so`, rebuild it first with
`./mylib.sh` from the `python` directory. If the build fails after a C change,
fix the top-level source rather than adding a copy below `python/`. That keeps
the native executable, tests, and Python library on the same simulation code.

When testing console behavior, use an interactive terminal. Long-running
commands intentionally rely on a second console worker so a later `stop` can
interrupt them.
