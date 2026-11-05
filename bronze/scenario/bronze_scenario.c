#include "bronze_scenario.h"
#include "../dsl/brz_parser.h"
#include "../tasks/bronze_program.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include "../dsl/brz_util.h"
#include "bronze_declaration_defaults.h"

struct BronzeScenario {
    BronzeProgram* active;
    BronzeLandPolicy land_policy;
    BronzeProfessionPolicy professions;
    char name[128];
    char diagnostics[16384];
};
static int error(FILE* report, int line, const char* format, ...)
{
    va_list args; va_start(args, format);
    fprintf(report,"ValidationError:%d: ",line);
    vfprintf(report,format,args); fputc('\n',report); va_end(args); return 0;
}
static int bounded(double value, double max) { return isfinite(value) && value>=0 && value<=max; }
static int conditions(const CompiledCondition* c, int line, FILE* report)
{
    int i;
    for(i=0;i<c->terms;i++) if(!bounded(c->value[i],c->kind[i]==BRZ_COND_RESOURCE?1e9:1)) return error(report,line,"need/probability values must be in 0..1");
    return 1;
}
static int statements(const ParsedConfig* cfg, const BrzVec* list, FILE* report)
{
    size_t i;
    for(i=0;i<list->len;i++) {
        const StmtDef* s=brz_vec_cat(list,i);
        if(s->kind==ST_OP) {
            const OpDef* op=&s->as.op;
            const ActionDef* action=brz_action_find(cfg,op->op);
            int count=(op->a0?1:0)+(op->a1?1:0)+(op->a2?1:0)+(op->has_n0?1:0);
            if(!action || count<action->min_args || count>action->max_args)
                return error(report,s->line,"action '%s' has unsupported argument count/units",op->op);
            int storage_scope = (op->action_code==BRZ_ACTION_FETCH || op->action_code==BRZ_ACTION_DEPOSIT) &&
                                op->a1 && !strcmp(op->a1,"household");
            int exchange = op->action_code==BRZ_ACTION_TRADE || op->action_code==BRZ_ACTION_SERVE;
            if(exchange && (!strcmp(op->a0,op->a1) || (op->has_n0 && op->n0<=0)))
                return error(report,s->line,"exchange requires distinct goods and a positive quantity");
            if(op->a2 || (!exchange && op->a1 && !storage_scope))
                return error(report,s->line,"action '%s' requires numeric quantities, not extra word arguments",op->op);
            if(s->as.op.has_n0 && !bounded(s->as.op.n0,1000000000)) return error(report,s->line,"quantity must be in 0..1000000000 economic units");
        } else if(s->kind==ST_CHANCE) {
            if(!bounded(s->as.chance.chance_pct,100)) return error(report,s->line,"chance must be in 0..100 percent");
            if(!statements(cfg,&s->as.chance.body,report)) return 0;
        } else if(!conditions(&s->as.when_stmt.condition,s->line,report) || !statements(cfg,&s->as.when_stmt.body,report)) return 0;
    }
    return 1;
}
static int known_parameter(const ParsedConfig* c, const char* key)
{
    static const char* names[]={"world_seed","world_years","world_sim_map_w","world_sim_map_h",
        "world_population","agents_count","settlements_count","sim_days","sim_cache_max","sim_snapshot_every","sim_map_every","sim_report_every"};
    size_t i;
    for(i=0;i<sizeof(names)/sizeof(names[0]);i++) if(!strcmp(key,names[i])) return 1;
    for(i=0;i<kind_table_count(&c->resource_kinds);i++) {
        char renewal[256];
        snprintf(renewal,sizeof(renewal),"%s_renew",kind_table_name(&c->resource_kinds,i));
        if(!strcmp(key,renewal)) return 1;
    }
    return 0;
}
/* The roster index is the compiled vocation index, including disabled jobs. */
static int occupation_parameter(const ParamDef* param, BronzeProfessionPolicy* policy, FILE* report)
{
    const char* key = param->key + strlen("world_occupation_");
    struct { const char* key; unsigned* value; unsigned min, max; } settings[] = {
        {"min_age_days", &policy->min_age_days, 0, 1000000},
        {"site_capacity", &policy->site_capacity, 1, 256},
        {"retry_minutes", &policy->retry_minutes, 1, 1440},
        {"skill_start", &policy->skill_start, 0, 1000},
        {"skill_max", &policy->skill_max, 1, 1000},
        {"mentor_skill", &policy->mentor_skill, 0, 1000},
        {"skill_gain", &policy->skill_gain, 0, 1000},
    };
    for (size_t i = 0; i < sizeof(settings)/sizeof(settings[0]); i++) {
        if (strcmp(key, settings[i].key)) continue;
        if (param->has_svalue || !bounded(param->value, settings[i].max) ||
            param->value < settings[i].min || floor(param->value) != param->value)
            return error(report, 0, "invalid occupation setting '%s'", key);
        *settings[i].value = (unsigned)param->value;
        return 1;
    }
    for (unsigned i = 0; i < policy->count; i++) {
        BronzeVocation* vocation = &policy->vocations[i];
        size_t length = strlen(vocation->name);
        if (strncmp(key, vocation->name, length)) continue;
        const char* suffix = key + length;
        if (!strcmp(suffix, "_weight")) {
            if (param->has_svalue || !bounded(param->value, 10000) || floor(param->value) != param->value)
                return error(report, 0, "occupation weight must be a whole number in 0..10000 for '%s'", vocation->name);
            vocation->weight = (unsigned)param->value;
            return 1;
        }
        if (!strcmp(suffix, "_workplace")) {
            static const char* names[] = {"", "field", "forest", "mine_copper", "mine_tin", "service", "coast"};
            if (param->has_svalue) {
                for (unsigned j = 1; j < sizeof(names)/sizeof(names[0]); j++) {
                    if (strcmp(param->svalue, names[j])) continue;
                    vocation->workplace = (BronzeJobPlace)j;
                    return 1;
                }
            }
            return error(report, 0, "invalid workplace for occupation '%s'", vocation->name);
        }
    }
    return error(report, 0, "unknown occupation setting '%s'", key);
}
static int recipe_parameter(const ParsedConfig *config, const ParamDef *param,
                            BronzeRecipePolicy *policies, FILE *report)
{
    const char *key = param->key + strlen("world_recipe_");
    for (size_t i = 0; i < config->recipes.len; i++) {
        const RecipeDef *recipe = brz_vec_cat(&config->recipes, i);
        size_t length = strlen(recipe->name);
        if (strncmp(key, recipe->name, length)) continue;
        const char *suffix = key + length;
        BronzeRecipePolicy *policy = &policies[i];
        if (!strcmp(suffix, "_minutes") && !param->has_svalue &&
            bounded(param->value, 1440) && param->value >= 1 && floor(param->value) == param->value) {
            policy->minutes = (unsigned)param->value;
            return 1;
        }
        if (!strcmp(suffix, "_minimum_skill") && !param->has_svalue &&
            bounded(param->value, 1000) && floor(param->value) == param->value) {
            policy->minimum_skill = (unsigned)param->value;
            return 1;
        }
        if (!strcmp(suffix, "_workplace") && param->has_svalue &&
            (!strcmp(param->svalue, "service") || !strcmp(param->svalue, "workplace"))) {
            policy->at_workplace = !strcmp(param->svalue, "workplace");
            return 1;
        }
        if (!strcmp(suffix, "_tool") && param->has_svalue) {
            int kind = kind_table_find(&config->item_kinds, param->svalue);
            if (kind >= 0) {
                policy->tool = kind;
                return 1;
            }
        }
    }
    return error(report, 0, "invalid recipe setting '%s'", key);
}

static int admission(const ParsedConfig* c, FILE* report, BronzeLandPolicy* policy,
                     BronzeProfessionPolicy* professions, BronzeRecipePolicy *recipes)
{
    size_t i,j;
    if(!c->vocations.len || c->vocations.len>BRONZE_VOCATION_LIMIT)
        return error(report,0,"scenario requires 1..%d vocations",BRONZE_VOCATION_LIMIT);
    professions->count = (unsigned)c->vocations.len;
    for (i=0;i<c->vocations.len;i++) {
        const VocationDef* v=brz_vec_cat(&c->vocations,i);
        if(strlen(v->name)>=BRONZE_VOCATION_NAME_LIMIT)
            return error(report,0,"vocation name exceeds %d characters",BRONZE_VOCATION_NAME_LIMIT-1);
        for(j=0;j<i;j++) if(!strcmp(v->name,professions->vocations[j].name))
            return error(report,0,"duplicate vocation '%s'",v->name);
        strcpy(professions->vocations[i].name,v->name);
    }
    if(c->scenario_profile || c->map_refs.len || c->places.len || c->roles.len || c->needs.len || c->services.len || c->disruptions.len)
        return error(report,0,"service-profile declarations are not supported by the Bronze graphical host");
    if(c->agent_count>100000 || c->settlement_count>10000) return error(report,0,"population/place count exceeds host limits");
    if(kind_table_count(&c->resource_kinds)>BRONZE_RESOURCE_LIMIT)
        return error(report,0,"scenario exceeds the %d resource kind limit",BRONZE_RESOURCE_LIMIT);
    for(i=0;i<c->resources.len;i++) {
        const ResourceDef* r=brz_vec_cat(&c->resources,i);
        if(!bounded(r->capacity,1e9) || !bounded(r->renew,1) || !bounded(r->nutrition,1) || !bounded(r->market_target,1e9))
            return error(report,r->line,"resource capacity/market target use 0..1e9 units; renew/nutrition use 0..1");
    }
    if(kind_table_count(&c->item_kinds)>BRONZE_ITEM_LIMIT || c->recipes.len>BRONZE_RECIPE_LIMIT)
        return error(report,0,"scenario exceeds the graphical recipe or item limit");
    for(i=0;i<c->recipes.len;i++) {
        const RecipeDef* r=brz_vec_cat(&c->recipes,i);
        recipes[i]=(BronzeRecipePolicy){.minutes=4,.tool=-1};
        if(!r->inputs.len || r->inputs.len>BRONZE_RESOURCE_LIMIT+BRONZE_ITEM_LIMIT ||
           (r->output_resource_id>=0)==(r->output_item_id>=0))
            return error(report,r->line,"recipe requires inputs and an unambiguous output kind");
        for(j=0;j<i;j++) {
            const RecipeDef *prior=brz_vec_cat(&c->recipes,j);
            if(!strcmp(r->name,prior->name)) return error(report,r->line,"duplicate recipe '%s'",r->name);
        }
        if(!bounded(r->output_amount,1e9) || r->output_amount==0) return error(report,r->line,"recipe output must be positive and at most 1e9 units");
        for(j=0;j<r->inputs.len;j++) {
            const RecipeInputDef* in=brz_vec_cat(&r->inputs,j);
            if((in->resource_id>=0)==(in->item_id>=0))
                return error(report,r->line,"recipe input requires an unambiguous kind");
            if(!bounded(in->amount,1e9) || in->amount==0) return error(report,r->line,"recipe input must be positive and at most 1e9 units");
        }
    }
    if(!bounded(c->settlement_policy.deposit_threshold,1e9) || !bounded(c->settlement_policy.rest_recovery,1))
        return error(report,0,"settlement policy is outside stock/normalized-recovery bounds");
    struct { const char* name; unsigned* value; unsigned min, max; } landscape[] = {
        {"world_field_slope", &policy->field_slope, 0, 255},
        {"world_field_coast", &policy->field_coast, 1, 254},
        {"world_field_moisture", &policy->field_moisture, 0, 255},
        {"world_woodland_slope", &policy->woodland_slope, 0, 255},
        {"world_woodland_moisture", &policy->woodland_moisture, 0, 255},
        {"world_ore_relief", &policy->ore_relief, 1, 255},
        {"world_tin_height", &policy->tin_height, 151, 255},
        {"world_town_capacity", &policy->town_capacity, 1, 256},
        {"world_town_house_capacity", &policy->town_house_capacity, 4, 16},
        {"world_town_work_yards", &policy->town_work_yards, 1, 4},
        {"world_carry_capacity", &policy->carry_capacity, 1, 1024},
        {"world_storage_capacity", &policy->storage_capacity, 1, 1000000},
        {"world_household_capacity", &policy->household_capacity, 0, 1000000},
        {"world_ration_daily", &policy->ration_daily, 0, 1024},
        {"world_town_initial_grain", &policy->town_initial_grain, 0, 1000000},
        {"world_patch_initial_per_cell", &policy->patch_initial_per_cell, 0, 8},
    };
    for(i=0;i<c->params.len;i++) {
        const ParamDef* p=brz_vec_cat(&c->params,i);
        if (!strcmp(p->key,"world_food_spoilage")) {
            if (p->has_svalue || !bounded(p->value,1))
                return error(report,0,"food spoilage must be a fraction in 0..1");
            policy->food_spoilage=p->value;
            continue;
        }
        if(!strncmp(p->key,"world_recipe_",strlen("world_recipe_"))) {
            if(!recipe_parameter(c,p,recipes,report)) return 0;
            continue;
        }
        if(!strncmp(p->key,"world_occupation_",strlen("world_occupation_"))) {
            if(!occupation_parameter(p,professions,report)) return 0;
            continue;
        }
        for(j=0;j<sizeof(landscape)/sizeof(landscape[0]);j++) {
            if(strcmp(p->key,landscape[j].name)) continue;
            if(p->has_svalue || !bounded(p->value,landscape[j].max) ||
               p->value<landscape[j].min || floor(p->value)!=p->value)
                return error(report,0,"parameter '%s' must be a whole number in %u..%u",
                             p->key,landscape[j].min,landscape[j].max);
            *landscape[j].value=(unsigned)p->value;
            break;
        }
        if(j<sizeof(landscape)/sizeof(landscape[0])) continue;
        /* Descriptive world annotations from the original extended format.
         * The native host supplies its own landscape and population. */
        if (p->has_svalue &&
            ((!strcmp(p->key,"world_terrain") && !strcmp(p->svalue,"coast")) ||
             (!strcmp(p->key,"world_climate") && !strcmp(p->svalue,"temperate")) ||
             (!strcmp(p->key,"world_river") && !strcmp(p->svalue,"yes")) ||
             (!strcmp(p->key,"world_politics") && !strcmp(p->svalue,"tribal")))) continue;
        if(p->has_svalue || !known_parameter(c,p->key)) return error(report,0,"unsupported or nonnumeric parameter '%s'",p->key);
        if(!p->has_svalue && (strstr(p->key,"map_w") || strstr(p->key,"map_h")) &&
           (p->value<1 || p->value>4096 || floor(p->value)!=p->value))
            return error(report,0,"map dimensions must be whole cells in 1..4096");
        if(!p->has_svalue && (strstr(p->key,"_days") || strstr(p->key,"_every")) &&
           (p->value>1000000 || floor(p->value)!=p->value))
            return error(report,0,"durations/report intervals must be whole days in 0..1000000");
        if(!p->has_svalue && !bounded(p->value,4294967295.0)) return error(report,0,"parameter '%s' is outside supported scalar bounds",p->key);
    }
    for(i=0;i<c->recipes.len;i++)
        if(recipes[i].minimum_skill>professions->skill_max)
            return error(report,0,"recipe minimum skill exceeds occupation skill scale");
    if(policy->town_initial_grain>policy->storage_capacity)
        return error(report,0,"initial grain exceeds township storage capacity");
    if(professions->skill_start>professions->skill_max || professions->skill_gain>professions->skill_max ||
       professions->mentor_skill>professions->skill_max)
        return error(report,0,"occupation starting skill and gain must fit the skill scale");
    for(i=0;i<c->vocations.len;i++) {
        const VocationDef* v=brz_vec_cat(&c->vocations,i);
        if (v->tasks.len > 256 || v->rules.len > 256)
            return error(report,0,"vocation %s exceeds the 256 task/rule limit",v->name);
        if(professions->vocations[i].weight && !professions->vocations[i].workplace)
            return error(report,0,"enabled occupation '%s' requires a workplace",v->name);
        for(j=0;j<v->rules.len;j++) { const RuleDef* r=brz_vec_cat(&v->rules,j); if(r->weight<1 || !conditions(&r->condition,r->line,report)) return error(report,r->line,"invalid rule weight or condition"); }
        for(j=0;j<v->tasks.len;j++) if(!statements(c,&((const TaskDef*)brz_vec_cat(&v->tasks,j))->stmts,report)) return 0;
    }
    return 1;
}
static void destroy_program(BronzeProgram* program)
{
    ParsedConfig* config = (ParsedConfig*)program->config;
    brz_cfg_free(config);
    free(config);
    free(program);
}

BronzeScenario* bronze_scenario_create(void) { return calloc(1,sizeof(BronzeScenario)); }
void bronze_scenario_destroy(BronzeScenario* s)
{
    if (!s) return;
    bronze_program_release(s->active);
    free(s);
}
int bronze_scenario_load(BronzeScenario* s, const char* path, const char* name)
{
    ParsedConfig* next; FILE* report; FILE* input; long size; int accepted=0;
    if(!s) return 0;
    snprintf(s->diagnostics,sizeof(s->diagnostics),"%s:0: unable to load scenario",name?name:"scenario");
    if(!path || !name) return 0;
    input=fopen(path,"rb");
    if(!input) { snprintf(s->diagnostics,sizeof(s->diagnostics),"%s:0: scenario file is missing or unreadable",name); return 0; }
    if(fseek(input,0,SEEK_END)!=0) { fclose(input); return 0; }
    size=ftell(input); fclose(input);
    if(size<0 || size>2*1024*1024) { snprintf(s->diagnostics,sizeof(s->diagnostics),"%s:0: file exceeds the 2 MiB authoring limit",name); return 0; }
    next=calloc(1,sizeof(*next)); report=tmpfile();
    if(!next || !report) { free(next); if(report) fclose(report); return 0; }
    brz_cfg_init(next);
    BronzeLandPolicy policy=bronze_land_defaults();
    BronzeProfessionPolicy professions={
        .site_capacity=32, .retry_minutes=60, .skill_max=100, .skill_gain=1
    };
    BronzeRecipePolicy recipes[BRONZE_RECIPE_LIMIT]={0};
    accepted=brz_parse_file_report(path,next,report);
    int declarations = accepted && !next->resources.len && !next->recipes.len && !next->actions.len;
    if (declarations) {
        accepted = declaration_defaults(next) && brz_cfg_compile(next, report);
        policy.patch_initial_per_cell = 8;
        professions.site_capacity = 256;
        for (size_t i = 0; i < next->vocations.len && i < BRONZE_VOCATION_LIMIT; ++i) {
            professions.vocations[i].weight = 1;
            professions.vocations[i].workplace = BRONZE_JOB_FIELD;
        }
    }
    accepted = accepted && admission(next,report,&policy,&professions,recipes);
    BronzeProgram* program = accepted ? malloc(sizeof(*program)) : NULL;
    if (accepted && !program) accepted = error(report, 0, "unable to retain task program");
    if(accepted) {
        *program = (BronzeProgram){.config=next,.references=1,.destroy=destroy_program};
        memcpy(program->recipes,recipes,sizeof(recipes));
        bronze_program_release(s->active);
        s->land_policy=policy;
        s->professions=professions;
        s->active=program; snprintf(s->name,sizeof(s->name),"%s",name);
        snprintf(s->diagnostics,sizeof(s->diagnostics),"%s: validated %zu vocations. Task execution uses the native work gate. See the selected actor for progress or blocked operations.",name,next->vocations.len);
    } else {
        size_t offset=(size_t)snprintf(s->diagnostics,sizeof(s->diagnostics),"%s:0: load rejected; previous scenario retained.\n",name);
        rewind(report);
        if(offset>=sizeof(s->diagnostics)) offset=sizeof(s->diagnostics)-1;
        size_t count=fread(s->diagnostics+offset,1,sizeof(s->diagnostics)-offset-1,report);
        s->diagnostics[offset+count]=0;
        s->diagnostics[sizeof(s->diagnostics)-1]=0;
        brz_cfg_free(next); free(next);
    }
    fclose(report); return accepted;
}
const char* bronze_scenario_name(const BronzeScenario* s) { return s?s->name:""; }
const char* bronze_scenario_diagnostics(const BronzeScenario* s) { return s?s->diagnostics:"Scenario allocation failed"; }
uint32_t bronze_scenario_seed(const BronzeScenario* s) { return s && s->active?s->active->config->seed:0; }
size_t bronze_scenario_vocations(const BronzeScenario* s) { return s && s->active?s->active->config->vocations.len:0; }

BronzeLandPolicy bronze_scenario_land_policy(const BronzeScenario* s)
{ return s && s->active ? s->land_policy : bronze_land_defaults(); }

const BronzeProfessionPolicy* bronze_scenario_professions(const BronzeScenario* s)
{ return s && s->active ? &s->professions : NULL; }

BronzeProgram* bronze_scenario_program(const BronzeScenario* s)
{ return s ? s->active : NULL; }
