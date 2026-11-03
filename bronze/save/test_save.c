#include "bronze_save.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    const n_byte native_bytes[] = {1, 2, 3};
    const n_byte bronze_bytes[] = {4, 5};
    n_byte file[BRONZE_SAVE_HEADER + sizeof(native_bytes) + sizeof(bronze_bytes)];
    BronzeSaveSection native = {native_bytes, sizeof(native_bytes)};
    BronzeSaveSection sidecar = {bronze_bytes, sizeof(bronze_bytes)};
    assert(bronze_save_write(file, sizeof(file), native, sidecar));

    BronzeSaveSection decoded_native, decoded_sidecar;
    assert(bronze_save_read((BronzeSaveSection){file, sizeof(file)}, &decoded_native, &decoded_sidecar));
    assert(decoded_native.length == native.length && decoded_sidecar.length == sidecar.length);
    assert(!memcmp(decoded_native.data, native.data, native.length));
    assert(!memcmp(decoded_sidecar.data, sidecar.data, sidecar.length));

    for (n_uint length = 0; length < sizeof(file); ++length)
        assert(!bronze_save_read((BronzeSaveSection){file, length}, &decoded_native, &decoded_sidecar));

    /* Every byte participates in format or integrity validation. */
    for (n_uint index = 0; index < sizeof(file); ++index) {
        file[index] ^= 1;
        assert(!bronze_save_read((BronzeSaveSection){file, sizeof(file)}, &decoded_native, &decoded_sidecar));
        file[index] ^= 1;
    }
    assert(!bronze_save_write(file, sizeof(file) - 1, native, sidecar));
    char path[] = "/tmp/bronze-save-test.XXXXXX";
    int descriptor = mkstemp(path);
    assert(descriptor >= 0 && close(descriptor) == 0);
    BronzeSaveSection package = {file, sizeof(file)};
    assert(bronze_save_publish(path, package));
    file[sizeof(file) - 1] ^= 1;
    assert(!bronze_save_publish(path, package));
    file[sizeof(file) - 1] ^= 1;

    FILE *saved = fopen(path, "rb");
    assert(saved);
    n_byte restored[sizeof(file)];
    assert(fread(restored, 1, sizeof(restored), saved) == sizeof(restored));
    assert(fgetc(saved) == EOF && fclose(saved) == 0);
    assert(memcmp(file, restored, sizeof(file)) == 0);
    assert(unlink(path) == 0);
    return 0;
}
