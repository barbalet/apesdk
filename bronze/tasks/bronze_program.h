#ifndef BRONZE_PROGRAM_H
#define BRONZE_PROGRAM_H
#include "../dsl/brz_dsl.h"

enum { BRONZE_RECIPE_LIMIT = 64 };
typedef struct {
    unsigned minutes, minimum_skill;
    int at_workplace, tool;
} BronzeRecipePolicy;

/* Owned on the serialized Bronze host thread. The scenario, pending world and
   active world each hold a reference, so a paused world can finish its reset. */
typedef struct BronzeProgram {
    const ParsedConfig *config;
    size_t references;
    void (*destroy)(struct BronzeProgram *);
    BronzeRecipePolicy recipes[BRONZE_RECIPE_LIMIT];
} BronzeProgram;

static inline BronzeProgram *bronze_program_retain(BronzeProgram *program)
{
    if (program) program->references++;
    return program;
}

static inline void bronze_program_release(BronzeProgram *program)
{
    if (program && --program->references == 0) program->destroy(program);
}
#endif
