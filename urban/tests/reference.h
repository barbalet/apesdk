/* Fixed-width records keep SDK-private C types inside the reference translation units. */
#ifndef PRA_TEST_REFERENCE_H
#define PRA_TEST_REFERENCE_H
#include <stdint.h>
typedef struct
{
    int64_t rooms, genetics[64], points[16][32][2];
} reference_house;
typedef struct
{
    int64_t points[32], radius, center[2];
} reference_tree;
void reference_create_house(uint16_t seed[2], const int64_t center[2], reference_house *out);
void reference_create_tree(uint16_t seed[2], const int64_t center[2], reference_tree *out);
void reference_create_fence(uint16_t seed[2], uint8_t direction, const int64_t center[2],
                            int64_t out[8][2][2]);
uint16_t reference_random(uint16_t seed[2]);
#endif
