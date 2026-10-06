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
    /* Filled once by brz_cfg_compile(); execution never needs to resolve text. */
    int action_code;
    int arg0_resource_id;
    int arg1_resource_id;
    int arg0_item_id;
    int arg1_item_id;
    uint16_t terrain_query;
    int recipe_index;
} OpDef;

typedef enum {
    BRZ_ACTION_INVALID = -1,
    BRZ_ACTION_GATHER,
    BRZ_ACTION_CRAFT,
    BRZ_ACTION_TRADE,
    BRZ_ACTION_REST,
    BRZ_ACTION_MOVE_TO,
    BRZ_ACTION_ROAM,
    BRZ_ACTION_WANDER,
    BRZ_ACTION_DEPOSIT,
    BRZ_ACTION_EAT
} BrzActionCode;

typedef enum {
    ST_OP = 0,
    ST_CHANCE,
    ST_WHEN
} StmtKind;

typedef enum { BRZ_COND_INVALID=-1, BRZ_COND_TRUE, BRZ_COND_FALSE, BRZ_COND_HUNGER, BRZ_COND_FATIGUE, BRZ_COND_PROB } BrzConditionKind;
typedef enum { BRZ_CMP_TRUTHY, BRZ_CMP_GT, BRZ_CMP_LT, BRZ_CMP_GE, BRZ_CMP_LE, BRZ_CMP_EQ, BRZ_CMP_NE } BrzCompareCode;
typedef struct { BrzConditionKind kind[2]; BrzCompareCode comparison[2]; double value[2]; int join_or; int terms; } CompiledCondition;

typedef struct StmtDef StmtDef;

struct StmtDef {
    StmtKind kind;
    int line;
    union {
        OpDef op;
        struct { double chance_pct; BrzVec body; } chance;    /* percent 0..100 */
        struct { char* when_expr; CompiledCondition condition; BrzVec body; } when_stmt;
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
    CompiledCondition condition;
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
    int kind_id;
    uint16_t terrain_query;
} ResourceDef;

typedef struct {
    char* name;
    char* output;
    double output_amount;
    BrzVec inputs;       /* RecipeInputDef */
    int line;
    int output_resource_id;
    int output_item_id;
} RecipeDef;

typedef struct {
    char* kind;
    double amount;
    int resource_id;
    int item_id;
} RecipeInputDef;

typedef struct {
    char* name;
    int min_args;
    int max_args;
    int line;
    int code;
} ActionDef;

typedef struct { char* id; int line; } MapRefDef;
typedef struct { char* name; char* kind; char* map_ref; double capacity; int line; } PlaceDef;
typedef struct { char* name; char* workplace; double min_age; int line; } RoleDef;
typedef struct { char* name; double priority; int line; } NeedDef;
typedef struct {
    char* name; char* provider_role; char* recipient_role; char* place;
    char* input; char* output; double price; int line;
} ServiceDef;
typedef struct { char* name; char* affects; int line; } DisruptionDef;

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
    int language_version;
    char* scenario_profile;

    /* kinds { resources { ... } items { ... } } */
    KindTable resource_kinds;
    KindTable item_kinds;

    /* resource params or other numeric params */
    BrzVec params; /* ParamDef */

    BrzVec resources; /* ResourceDef */
    BrzVec recipes;   /* RecipeDef */
    BrzVec actions;   /* ActionDef */
    BrzVec map_refs;  /* MapRefDef */
    BrzVec places;    /* PlaceDef */
    BrzVec roles;     /* RoleDef */
    BrzVec needs;     /* NeedDef */
    BrzVec services;  /* ServiceDef */
    BrzVec disruptions; /* DisruptionDef */
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
bool brz_cfg_compile(ParsedConfig* cfg, FILE* errors);
int brz_action_code(const char* name);
uint16_t brz_terrain_query(const char* name);
int brz_condition_compile(const char* expression, CompiledCondition* condition, int line, FILE* errors);

#endif /* BRZ_DSL_H */
