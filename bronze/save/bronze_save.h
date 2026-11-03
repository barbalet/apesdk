#ifndef BRONZE_SAVE_H
#define BRONZE_SAVE_H

#include "../../toolkit/toolkit.h"

enum { BRONZE_SAVE_HEADER = 80 };
typedef struct {
    const n_byte *data;
    n_uint length;
} BronzeSaveSection;

/* Borrowed section views are valid while the input buffer is alive. */
int bronze_save_read(BronzeSaveSection file, BronzeSaveSection *native,
                     BronzeSaveSection *sidecar);
int bronze_save_write(n_byte *file, n_uint capacity, BronzeSaveSection native,
                      BronzeSaveSection sidecar);

/* Publish an already encoded package without overwriting the old file on failure. */
int bronze_save_publish(const char *path, BronzeSaveSection package);

#endif
