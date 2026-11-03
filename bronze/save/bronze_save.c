#include "bronze_save.h"

#include <CommonCrypto/CommonDigest.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static const n_byte signature[8] = {'B', 'R', 'Z', 'S', 'A', 'V', 'E', 1};

static uint32_t read_length(const n_byte *bytes)
{
    return (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8) |
           ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
}

static void write_length(n_byte *bytes, uint32_t length)
{
    for (unsigned i = 0; i < 4; ++i)
        bytes[i] = (n_byte)(length >> (8 * i));
}

static int valid_lengths(n_uint native, n_uint sidecar, n_uint capacity)
{
    return native && sidecar && native <= UINT32_MAX && sidecar <= UINT32_MAX &&
           capacity >= BRONZE_SAVE_HEADER && capacity - BRONZE_SAVE_HEADER <= UINT32_MAX &&
           native <= capacity - BRONZE_SAVE_HEADER &&
           sidecar == capacity - BRONZE_SAVE_HEADER - native;
}

int bronze_save_read(BronzeSaveSection file, BronzeSaveSection *native,
                     BronzeSaveSection *sidecar)
{
    if (!file.data || !native || !sidecar || file.length < BRONZE_SAVE_HEADER ||
        memcmp(file.data, signature, sizeof(signature)))
        return 0;
    n_uint native_length = read_length(file.data + 8);
    n_uint sidecar_length = read_length(file.data + 12);
    if (!valid_lengths(native_length, sidecar_length, file.length))
        return 0;
    const n_byte *payload = file.data + BRONZE_SAVE_HEADER;
    n_byte digest[CC_SHA256_DIGEST_LENGTH];
    CC_SHA256(payload, (CC_LONG)native_length, digest);
    if (memcmp(digest, file.data + 16, sizeof(digest)))
        return 0;
    CC_SHA256(payload, (CC_LONG)(native_length + sidecar_length), digest);
    if (memcmp(digest, file.data + 48, sizeof(digest)))
        return 0;
    *native = (BronzeSaveSection){payload, native_length};
    *sidecar = (BronzeSaveSection){payload + native_length, sidecar_length};
    return 1;
}

int bronze_save_write(n_byte *file, n_uint capacity, BronzeSaveSection native,
                      BronzeSaveSection sidecar)
{
    if (!file || !native.data || !sidecar.data ||
        !valid_lengths(native.length, sidecar.length, capacity))
        return 0;
    memcpy(file, signature, sizeof(signature));
    write_length(file + 8, (uint32_t)native.length);
    write_length(file + 12, (uint32_t)sidecar.length);
    CC_SHA256(native.data, (CC_LONG)native.length, file + 16);
    memcpy(file + BRONZE_SAVE_HEADER, native.data, native.length);
    memcpy(file + BRONZE_SAVE_HEADER + native.length, sidecar.data, sidecar.length);
    CC_SHA256(file + BRONZE_SAVE_HEADER, (CC_LONG)(native.length + sidecar.length), file + 48);
    return 1;
}

int bronze_save_publish(const char *path, BronzeSaveSection package)
{
    BronzeSaveSection native, sidecar;
    if (!path || !*path || !bronze_save_read(package, &native, &sidecar))
        return 0;
    static const char suffix[] = ".tmp.XXXXXX";
    n_uint length = strlen(path);
    if (length > SIZE_MAX - sizeof(suffix))
        return 0;
    char *temporary = malloc(length + sizeof(suffix));
    if (!temporary)
        return 0;
    memcpy(temporary, path, length);
    memcpy(temporary + length, suffix, sizeof(suffix));

    int published = 0;
    int descriptor = mkstemp(temporary);
    if (descriptor >= 0) {
        FILE *stream = fdopen(descriptor, "wb");
        if (stream) {
            int complete = fwrite(package.data, 1, package.length, stream) == package.length &&
                           fflush(stream) == 0 && fsync(descriptor) == 0;
            if (fclose(stream) == 0 && complete)
                published = rename(temporary, path) == 0;
        } else {
            close(descriptor);
        }
        if (!published)
            unlink(temporary);
    }
    free(temporary);
    return published;
}
