#include "brz_util.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

bool brz_streq(const char* a, const char* b)
{
    if(a==b) return true;
    if(!a || !b) return false;
    return strcmp(a,b)==0;
}

char* brz_strdup(const char* s)
{
    if(!s) return NULL;
    size_t n = strlen(s);
    char* out = (char*)malloc(n+1);
    if(!out) return NULL;
    memcpy(out, s, n+1);
    return out;
}

char* brz_read_entire_file(const char* path, size_t* out_size)
{
    if(out_size) *out_size = 0;
    FILE* f = fopen(path, "rb");
    if(!f) return NULL;
    if(fseek(f, 0, SEEK_END)!=0) { fclose(f); return NULL; }
    long sz = ftell(f);
    if(sz < 0) { fclose(f); return NULL; }
    rewind(f);

    char* buf = (char*)malloc((size_t)sz + 1);
    if(!buf) { fclose(f); return NULL; }

    size_t n = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    buf[n] = 0;
    if(out_size) *out_size = n;
    return buf;
}

/* xorshift32 */
void brz_rng_seed(BrzRng* r, uint32_t seed)
{
    if(!r) return;
    r->state = seed ? seed : 0xA341316C;
}

uint32_t brz_rng_u32(BrzRng* r)
{
    uint32_t x = r->state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    r->state = x;
    return x;
}

int brz_rng_range(BrzRng* r, int lo, int hi)
{
    if(hi < lo) { int t=lo; lo=hi; hi=t; }
    uint32_t span = (uint32_t)(hi - lo + 1);
    uint32_t v = brz_rng_u32(r);
    return lo + (int)(v % span);
}
