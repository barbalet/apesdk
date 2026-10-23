# JSON regression fixtures

Run from the repository root:

```sh
bash toolkit/test/test_object_json.sh
bash toolkit/test/test_object_file.sh
```

The first runner tests the public C interfaces and parser boundaries with address
and undefined-behavior sanitizers. The second builds apesdk-json, checks 36 valid
fixtures byte for byte, and checks ten intentionally malformed fixtures for exit
status 1 with no serialized output. Both runners are included in test.sh.

| Fixture | Coverage |
| --- | --- |
| json_numbers.json | Signed integers, fractions, precision, exponent notation |
| json_number_boundaries.json | Integer limits, large/small exponents, negative zero |
| json_nulls.json | Explicit null properties, null-only arrays, following values |
| json_null_positions.json | First/middle/last array entries and nested optional data |
| json_mixed.json | Mixed primitive/container types and nulls |
| json_empty_containers.json | Empty and nested object/array ownership paths |
| json_root_null.json | Scalar null with no trailing newline |
| json_root_decimal.json | Scalar negative decimal/exponent with trailing newline |
| json_root_negative.json | Scalar negative integer |

Existing simulation, geographic, random, and vector examples are retained. The
four vector conversion fixtures also have a separate existing vector test runner.
The valid file list in test_object_file.sh is explicit; do not assume every file
below this directory is valid JSON.

`invalid/` contains deliberately malformed inputs: leading zeros, incomplete
fractions/exponents, null spelling/suffix errors, split numeric tokens, forbidden
whitespace, truncated containers, trailing values, and embedded NUL bytes. These files are rejection
tests, not stale examples. Their temporary copies and any command output are
confined to a disposable directory during tests.

Generated in-memory tests cover long numeric tokens, exact-length buffers without
terminators, truncated input, all replacement type combinations, large flat
collection cleanup, and constructor/serialization/reparse checks. They also verify
that integer extraction does not coerce decimal or null values and that failed
non-finite decimal assignments preserve existing data.


## Additional validation

```sh
bash toolkit/test/test_object_json_fuzz.sh
bash toolkit/test/test_object_json_alloc.sh
SANITIZE=1 bash test.sh
```

The mutation runner uses a fixed seed (0x41504553), each fixture prefix, and 10,000
mutations per fixture under ASan/UBSan. It checks preprocessing/parser acceptance
parity and serialization/reparse stability. It is reproducible mutation testing,
not a coverage-guided libFuzzer campaign. The target in fuzz_object_json.c can also
be linked to libFuzzer on a toolchain that provides its runtime. Failures save the
input to FUZZ_FAILURE_PATH (default /tmp/apesdk-json-fuzz-failure.bin).

The allocation runner fails each allocation in selected parser, public setter,
and scalar/array/object serialization paths, counting live allocations to catch
leaks. It also checks that failed setters preserve the previous value and that
failed serialization returns no partial document. Its test allocator does not
change the production allocator.

The full SDK sanitizer command now halts on UBSan errors by default. The additional
validation found outstanding reports in unchanged simulation code:

- entity/social.c: negative feature indices access the features array.
- entity/being.c: a signed `1 << 31` shift is undefined.
- entity/body.c: a shift count becomes negative.
- toolkit/file.c: simulation-file reads/writes use potentially misaligned n_byte4 pointers.

The strict run stops at entity/social.c:163. These findings remain open; a normal
deterministic pass must not be interpreted as a clean full-SDK sanitizer run.
