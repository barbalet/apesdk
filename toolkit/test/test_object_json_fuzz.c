/* Reproducible mutation driver, also usable without a libFuzzer runtime. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);
static uint32_t state = 0x41504553;
static uint32_t random_value(void)
{
    state ^= state << 13; state ^= state >> 17; state ^= state << 5;
    return state;
}

int main(int argc, char **argv)
{
    unsigned long cases = 0;
    int file;
    for (file = 1; file < argc; file++)
    {
        uint8_t seed[8192], data[8192];
        FILE *input = fopen(argv[file], "rb");
        size_t size, i;
        if (!input) return 1;
        size = fread(seed, 1, sizeof(seed), input);
        fclose(input);
        for (i = 0; i <= size; i++)
        {
            LLVMFuzzerTestOneInput(seed, i);
            cases++;
        }
        for (i = 0; i < 10000; i++)
        {
            size_t length = size, changes = 1 + random_value() % 8, j;
            memcpy(data, seed, length);
            for (j = 0; j < changes; j++)
            {
                size_t position = random_value() % (length + 1);
                switch (random_value() % 4)
                {
                case 0:
                    if (position < length) data[position] ^= 1u << (random_value() % 8);
                    break;
                case 1:
                    if (length < sizeof(data))
                    {
                        memmove(data + position + 1, data + position, length - position);
                        data[position] = random_value(); length++;
                    }
                    break;
                case 2:
                    if (position < length)
                    { memmove(data + position, data + position + 1, length - position - 1); length--; }
                    break;
                default: length = position; break;
                }
            }
            LLVMFuzzerTestOneInput(data, length);
            cases++;
        }
    }
    printf("Mutation checks passed: %lu cases, fixed seed 0x41504553\n", cases);
    return argc > 1 ? 0 : 1;
}
