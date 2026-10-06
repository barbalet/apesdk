#ifndef BRZ_DSL_H
#define BRZ_DSL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "brz_vec.h"
#include "brz_kinds.h"

/* BRONZESIM DSL structures.
   Designed to parse very large .bronze files without fixed MAX limits. */

typedef struct {
    char* op;       /* e.g. "move_to", "gather", "craft", "rest", "roam", "trade" */
    char* a0;       /* first word arg */
    char* a1;       /* second word arg */
    char* a2;       /* third word arg */
    double n0;      /* first numeric arg */
    bool has_n0;
    int line;
} OpDef;

typedef enum {
    ST_OP = 0,
    ST_CHANCE,
    ST_WHEN
} StmtKind;

typedef struct StmtDef StmtDef;

struct StmtDef {
    StmtKind kind;
    int line;
    union {
        OpDef op;
        struct { double chance_pct; BrzVec body; } chance;    /* percent 0..100 */
        struct { char* when_expr; BrzVec body; } when_stmt;   /* expr string */
    } as;
};

typedef struct {
    char* name;
    BrzVec stmts; /* StmtDef */
} TaskDef;

typedef struct {
    char* name;
    char* when_expr; /* string expression (simple boolean expr) */
    char* do_task;   /* task name */
    int weight;
    int line;
} RuleDef;

typedef struct {
    char* name;
    BrzVec tasks; /* TaskDef */
    BrzVec rules; /* RuleDef */
} VocationDef;

typedef struct {
    char* key;
    double value;    /* numeric value when has_svalue==false */
    bool  has_svalue;
    char* svalue;    /* string value when has_svalue==true */
} ParamDef;

/* Scenario policy is deliberately separate from the runtime.  Names are
   resolved during validation; the runtime uses the resulting kind ids. */
typedef struct {
    char* name;
    char* habitat;       /* terrain query name, e.g. coast or forest */
    double capacity;
    double renew;
    double nutrition;
    double market_target;
    int line;
} ResourceDef;

typedef struct {
    char* name;
    char* output;
    double output_amount;
    BrzVec inputs;       /* RecipeInputDef */
    int line;
} RecipeDef;

typedef struct {
    char* kind;
    double amount;
} RecipeInputDef;

typedef struct {
    char* name;
    int min_args;
    int max_args;
    int line;
} ActionDef;

typedef struct {
    char* food_fallback;
    double deposit_threshold;
    double rest_recovery;
} SettlementPolicyDef;

typedef struct {
    /* common knobs */
    uint32_t seed;
    int years;
    int agent_count;
    int settlement_count;

    /* kinds { resources { ... } items { ... } } */
    KindTable resource_kinds;
    KindTable item_kinds;

    /* resource params or other numeric params */
    BrzVec params; /* ParamDef */

    BrzVec resources; /* ResourceDef */
    BrzVec recipes;   /* RecipeDef */
    BrzVec actions;   /* ActionDef */
    SettlementPolicyDef settlement_policy;

    /* vocations { vocation X { ... } } */
    BrzVec vocations; /* VocationDef */
} ParsedConfig;

/* lifecycle */
void brz_cfg_init(ParsedConfig* cfg);
void brz_cfg_free(ParsedConfig* cfg);

/* helpers */
TaskDef* brz_voc_find_task(VocationDef* voc, const char* name);
const ResourceDef* brz_resource_find(const ParsedConfig* cfg, const char* name);
const RecipeDef* brz_recipe_find(const ParsedConfig* cfg, const char* name);
const ActionDef* brz_action_find(const ParsedConfig* cfg, const char* name);
bool brz_cfg_validate(const ParsedConfig* cfg, FILE* errors);

#endif /* BRZ_DSL_H */
