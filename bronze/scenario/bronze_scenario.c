#include "bronze_scenario.h"
#include "../dsl/brz_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

struct BronzeScenario {
    ParsedConfig* active;
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
    for(i=0;i<c->terms;i++) if(!bounded(c->value[i],1)) return error(report,line,"need/probability values must be in 0..1");
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
            if(op->a2 || (op->action_code!=BRZ_ACTION_TRADE && op->a1))
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
        "agents_count","settlements_count","sim_days","sim_cache_max","sim_snapshot_every","sim_map_every","sim_report_every"};
    size_t i;
    for(i=0;i<sizeof(names)/sizeof(names[0]);i++) if(!strcmp(key,names[i])) return 1;
    for(i=0;i<kind_table_count(&c->resource_kinds);i++) {
        char renewal[256];
        snprintf(renewal,sizeof(renewal),"%s_renew",kind_table_name(&c->resource_kinds,i));
        if(!strcmp(key,renewal)) return 1;
    }
    return 0;
}
static int admission(const ParsedConfig* c, FILE* report)
{
    size_t i,j;
    if(!c->vocations.len) return error(report,0,"scenario requires at least one vocation");
    if(c->scenario_profile || c->map_refs.len || c->places.len || c->roles.len || c->needs.len || c->services.len || c->disruptions.len)
        return error(report,0,"service-profile declarations are not supported by the Bronze graphical host");
    if(c->agent_count>100000 || c->settlement_count>10000) return error(report,0,"population/place count exceeds host limits");
    for(i=0;i<c->resources.len;i++) {
        const ResourceDef* r=brz_vec_cat(&c->resources,i);
        if(!bounded(r->capacity,1e9) || !bounded(r->renew,1) || !bounded(r->nutrition,1) || !bounded(r->market_target,1e9))
            return error(report,r->line,"resource capacity/market target use 0..1e9 units; renew/nutrition use 0..1");
    }
    for(i=0;i<c->recipes.len;i++) {
        const RecipeDef* r=brz_vec_cat(&c->recipes,i);
        if(!bounded(r->output_amount,1e9) || r->output_amount==0) return error(report,r->line,"recipe output must be positive and at most 1e9 units");
        for(j=0;j<r->inputs.len;j++) {
            const RecipeInputDef* in=brz_vec_cat(&r->inputs,j);
            if(!bounded(in->amount,1e9) || in->amount==0) return error(report,r->line,"recipe input must be positive and at most 1e9 units");
        }
    }
    if(!bounded(c->settlement_policy.deposit_threshold,1e9) || !bounded(c->settlement_policy.rest_recovery,1))
        return error(report,0,"settlement policy is outside stock/normalized-recovery bounds");
    for(i=0;i<c->params.len;i++) {
        const ParamDef* p=brz_vec_cat(&c->params,i);
        if(p->has_svalue || !known_parameter(c,p->key)) return error(report,0,"unsupported or nonnumeric parameter '%s'",p->key);
        if(!p->has_svalue && (strstr(p->key,"map_w") || strstr(p->key,"map_h")) &&
           (p->value<1 || p->value>4096 || floor(p->value)!=p->value))
            return error(report,0,"map dimensions must be whole cells in 1..4096");
        if(!p->has_svalue && (strstr(p->key,"_days") || strstr(p->key,"_every")) &&
           (p->value>1000000 || floor(p->value)!=p->value))
            return error(report,0,"durations/report intervals must be whole days in 0..1000000");
        if(!p->has_svalue && !bounded(p->value,4294967295.0)) return error(report,0,"parameter '%s' is outside supported scalar bounds",p->key);
    }
    for(i=0;i<c->vocations.len;i++) {
        const VocationDef* v=brz_vec_cat(&c->vocations,i);
        for(j=0;j<v->rules.len;j++) { const RuleDef* r=brz_vec_cat(&v->rules,j); if(r->weight<1 || !conditions(&r->condition,r->line,report)) return error(report,r->line,"invalid rule weight or condition"); }
        for(j=0;j<v->tasks.len;j++) if(!statements(c,&((const TaskDef*)brz_vec_cat(&v->tasks,j))->stmts,report)) return 0;
    }
    return 1;
}
BronzeScenario* bronze_scenario_create(void) { return calloc(1,sizeof(BronzeScenario)); }
void bronze_scenario_destroy(BronzeScenario* s) { if(s) { if(s->active) { brz_cfg_free(s->active); free(s->active); } free(s); } }
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
    accepted=brz_parse_file_report(path,next,report) && admission(next,report);
    if(accepted) {
        if(s->active) { brz_cfg_free(s->active); free(s->active); }
        s->active=next; snprintf(s->name,sizeof(s->name),"%s",name);
        snprintf(s->diagnostics,sizeof(s->diagnostics),"%s: validated %zu vocations. Scenario-driven profession tasks are not active yet; the native simulation and prototype economy continue.",name,next->vocations.len);
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
uint32_t bronze_scenario_seed(const BronzeScenario* s) { return s && s->active?s->active->seed:0; }
size_t bronze_scenario_vocations(const BronzeScenario* s) { return s && s->active?s->active->vocations.len:0; }
