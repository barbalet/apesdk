#include "brz_dsl.h"
#include "brz_util.h"
#include <stdlib.h>
#include <string.h>


/*
DSL_GRAMMAR_BEGIN
# NOTE: This block is the single source of truth for the BRONZESIM DSL grammar.
# It is extracted and injected into DSL_MANUAL.md automatically (make -C src docs).
#
# Conventions:
#   - 'literal' denotes a keyword or symbol token.
#   - identifier and number are lexical tokens.
#   - { X } means repetition (zero or more).
#   - [ X ] means optional.
#
# This grammar describes the *surface syntax*. The engine imposes additional semantic rules.

program             := { top_level_block } EOF ;

top_level_block     := world_block
                    | kinds_block
                    | resources_block
                    | items_block
                    | recipes_block
                    | actions_block
                    | settlement_policy_block
                    | vocations_block
                    | compat_block ;

# ----- Blocks -----

world_block          := 'world' block_open { world_stmt } block_close ;
kinds_block          := 'kinds' block_open { kind_def } block_close ;
resources_block      := 'resources' block_open { resource_def } block_close ;
resource_def         := identifier number
                    | 'resource' identifier block_open { resource_property } block_close ;
resource_property    := 'habitat' identifier
                    | ('capacity' | 'renew' | 'nutrition' | 'market_target') number ;
items_block          := 'items' block_open { item_def } block_close ;
recipes_block        := 'recipes' block_open { recipe_def } block_close ;
recipe_def           := 'recipe' identifier block_open 'output' identifier number { 'input' identifier number } block_close ;
actions_block        := 'actions' block_open { 'action' identifier number number } block_close ;
settlement_policy_block := 'settlement_policy' block_open { settlement_property } block_close ;
settlement_property  := 'food_fallback' identifier
                    | ('deposit_threshold' | 'rest_recovery') number ;

vocations_block      := 'vocations' block_open { vocation_def } block_close ;
vocation_def         := 'vocation' identifier block_open { vocation_member } block_close ;
vocation_member      := task_def | rule_def ;

task_def             := 'task' identifier block_open { task_stmt } block_close ;
rule_def             := 'rule' identifier block_open { rule_stmt } block_close ;

# ----- World statements -----
# The world block is intentionally permissive: keys are identifiers.
# Values can be number or identifier.

world_stmt           := identifier value ;
value                := number | identifier ;

# ----- Registry definitions -----

kind_def             := 'resources' block_open { identifier } block_close
                    | 'items' block_open { identifier } block_close ;
item_def             := identifier 'item' ;

# ----- Rule / task language -----

rule_stmt            := 'when' condition
                    | 'do' identifier
                    | 'weight' number ;

task_stmt            := action_stmt | when_block | chance_block ;

# Common structured statements
when_block           := 'when' condition block_open { task_stmt } block_close ;
chance_block         := 'chance' number block_open { task_stmt } block_close ;

# Conditions are intentionally simple in the core grammar.
# The engine may accept additional operators in future revisions.

condition            := comparison | 'true' | 'false' | 'prob' number | 'chance' '(' number ')' ;
comparison           := identifier cond_op cond_rhs ;
cond_op              := '<' | '<=' | '>' | '>=' | '==' | '!=' ;
cond_rhs             := number | identifier ;

# Actions are a small, engine-defined set of verbs.
# Extend the verb set in the engine and keep the grammar here in sync.

action_stmt          := identifier { identifier | number } ;

# ----- Lexical helpers -----

block_open           := '{' ;
block_close          := '}' ;

# ----- Compatibility blocks -----
# Older examples may use these blocks. They are accepted for backwards compatibility
# and may be mapped internally onto the newer registries.

compat_block         := ('sim' | 'agents' | 'settlements') block_open { identifier value } block_close ;
DSL_GRAMMAR_END
*/

static void op_free(OpDef* op)
{
    if(!op) return;
    free(op->op);
    free(op->a0);
    free(op->a1);
    free(op->a2);
    memset(op, 0, sizeof(*op));
}

static void stmt_free(StmtDef* st);

static void stmt_vec_free(BrzVec* v)
{
    if(!v) return;
    for(size_t i=0;i<v->len;i++)
    {
        StmtDef* s = (StmtDef*)brz_vec_at(v, i);
        stmt_free(s);
    }
    brz_vec_destroy(v);
}

static void stmt_free(StmtDef* st)
{
    if(!st) return;
    switch(st->kind)
    {
        case ST_OP:
            op_free(&st->as.op);
            break;
        case ST_CHANCE:
            stmt_vec_free(&st->as.chance.body);
            break;
        case ST_WHEN:
            free(st->as.when_stmt.when_expr);
            stmt_vec_free(&st->as.when_stmt.body);
            break;
        default:
            break;
    }
    memset(st, 0, sizeof(*st));
}

static void task_free(TaskDef* t)
{
    if(!t) return;
    free(t->name);
    for(size_t i=0;i<t->stmts.len;i++)
    {
        StmtDef* s = (StmtDef*)brz_vec_at(&t->stmts, i);
        stmt_free(s);
    }
    brz_vec_destroy(&t->stmts);
    memset(t, 0, sizeof(*t));
}

static void rule_free(RuleDef* r)
{
    if(!r) return;
    free(r->name);
    free(r->when_expr);
    free(r->do_task);
    memset(r, 0, sizeof(*r));
}

static void voc_free(VocationDef* v)
{
    if(!v) return;
    free(v->name);

    for(size_t i=0;i<v->tasks.len;i++)
    {
        TaskDef* t = (TaskDef*)brz_vec_at(&v->tasks, i);
        task_free(t);
    }
    brz_vec_destroy(&v->tasks);

    for(size_t i=0;i<v->rules.len;i++)
    {
        RuleDef* r = (RuleDef*)brz_vec_at(&v->rules, i);
        rule_free(r);
    }
    brz_vec_destroy(&v->rules);

    memset(v, 0, sizeof(*v));
}

static void param_free(ParamDef* p)
{
    if(!p) return;
    free(p->key);
    free(p->svalue);
    memset(p, 0, sizeof(*p));
}

static void resource_free(ResourceDef* r)
{
    if(!r) return;
    free(r->name); free(r->habitat);
    memset(r, 0, sizeof(*r));
}

static void recipe_free(RecipeDef* r)
{
    if(!r) return;
    free(r->name); free(r->output);
    for(size_t i=0;i<r->inputs.len;i++){
        RecipeInputDef* in=(RecipeInputDef*)brz_vec_at(&r->inputs,i);
        free(in->kind);
    }
    brz_vec_destroy(&r->inputs);
    memset(r, 0, sizeof(*r));
}

static void action_free(ActionDef* a)
{
    if(!a) return;
    free(a->name);
    memset(a, 0, sizeof(*a));
}
static void map_ref_free(MapRefDef* d) { free(d->id); memset(d,0,sizeof(*d)); }
static void place_free(PlaceDef* d) { free(d->name); free(d->kind); free(d->map_ref); memset(d,0,sizeof(*d)); }
static void role_free(RoleDef* d) { free(d->name); free(d->workplace); memset(d,0,sizeof(*d)); }
static void need_free(NeedDef* d) { free(d->name); memset(d,0,sizeof(*d)); }
static void service_free(ServiceDef* d) { free(d->name); free(d->provider_role); free(d->recipient_role); free(d->place); free(d->input); free(d->output); memset(d,0,sizeof(*d)); }
static void disruption_free(DisruptionDef* d) { free(d->name); free(d->affects); memset(d,0,sizeof(*d)); }

void brz_cfg_init(ParsedConfig* cfg)
{
    if(!cfg) return;
    memset(cfg, 0, sizeof(*cfg));
    cfg->seed = 0xC0FFEEu;
    cfg->years = 60;
    cfg->agent_count = 0;
    cfg->settlement_count = 0;
    cfg->language_version = 1;
    /* Legacy documents inherit these declared policy defaults. */
    cfg->settlement_policy.deposit_threshold=2.0;
    cfg->settlement_policy.rest_recovery=0.04;
    kind_table_init(&cfg->resource_kinds);
    kind_table_init(&cfg->item_kinds);
    brz_vec_init(&cfg->params, sizeof(ParamDef));
    brz_vec_init(&cfg->resources, sizeof(ResourceDef));
    brz_vec_init(&cfg->recipes, sizeof(RecipeDef));
    brz_vec_init(&cfg->actions, sizeof(ActionDef));
    brz_vec_init(&cfg->map_refs,sizeof(MapRefDef));
    brz_vec_init(&cfg->places,sizeof(PlaceDef));
    brz_vec_init(&cfg->roles,sizeof(RoleDef));
    brz_vec_init(&cfg->needs,sizeof(NeedDef));
    brz_vec_init(&cfg->services,sizeof(ServiceDef));
    brz_vec_init(&cfg->disruptions,sizeof(DisruptionDef));
    brz_vec_init(&cfg->vocations, sizeof(VocationDef));
}

void brz_cfg_free(ParsedConfig* cfg)
{
    if(!cfg) return;

    for(size_t i=0;i<cfg->params.len;i++)
    {
        ParamDef* p = (ParamDef*)brz_vec_at(&cfg->params, i);
        param_free(p);
    }
    brz_vec_destroy(&cfg->params);

    for(size_t i=0;i<cfg->resources.len;i++) resource_free((ResourceDef*)brz_vec_at(&cfg->resources,i));
    brz_vec_destroy(&cfg->resources);
    for(size_t i=0;i<cfg->recipes.len;i++) recipe_free((RecipeDef*)brz_vec_at(&cfg->recipes,i));
    brz_vec_destroy(&cfg->recipes);
    for(size_t i=0;i<cfg->actions.len;i++) action_free((ActionDef*)brz_vec_at(&cfg->actions,i));
    brz_vec_destroy(&cfg->actions);
    for(size_t i=0;i<cfg->map_refs.len;i++) map_ref_free((MapRefDef*)brz_vec_at(&cfg->map_refs,i));
    brz_vec_destroy(&cfg->map_refs);
    for(size_t i=0;i<cfg->places.len;i++) place_free((PlaceDef*)brz_vec_at(&cfg->places,i));
    brz_vec_destroy(&cfg->places);
    for(size_t i=0;i<cfg->roles.len;i++) role_free((RoleDef*)brz_vec_at(&cfg->roles,i));
    brz_vec_destroy(&cfg->roles);
    for(size_t i=0;i<cfg->needs.len;i++) need_free((NeedDef*)brz_vec_at(&cfg->needs,i));
    brz_vec_destroy(&cfg->needs);
    for(size_t i=0;i<cfg->services.len;i++) service_free((ServiceDef*)brz_vec_at(&cfg->services,i));
    brz_vec_destroy(&cfg->services);
    for(size_t i=0;i<cfg->disruptions.len;i++) disruption_free((DisruptionDef*)brz_vec_at(&cfg->disruptions,i));
    brz_vec_destroy(&cfg->disruptions);
    free(cfg->scenario_profile);
    free(cfg->settlement_policy.food_fallback);

    for(size_t i=0;i<cfg->vocations.len;i++)
    {
        VocationDef* v = (VocationDef*)brz_vec_at(&cfg->vocations, i);
        voc_free(v);
    }
    brz_vec_destroy(&cfg->vocations);

    kind_table_destroy(&cfg->resource_kinds);
    kind_table_destroy(&cfg->item_kinds);

    memset(cfg, 0, sizeof(*cfg));
}

TaskDef* brz_voc_find_task(VocationDef* voc, const char* name)
{
    if(!voc || !name) return NULL;
    for(size_t i=0;i<voc->tasks.len;i++)
    {
        TaskDef* t = (TaskDef*)brz_vec_at(&voc->tasks, i);
        if(t->name && brz_streq(t->name, name)) return t;
    }
    return NULL;
}

const ResourceDef* brz_resource_find(const ParsedConfig* cfg, const char* name)
{
    if(!cfg || !name) return NULL;
    for(size_t i=0;i<cfg->resources.len;i++){
        const ResourceDef* r=(const ResourceDef*)brz_vec_cat(&cfg->resources,i);
        if(r->name && brz_streq(r->name,name)) return r;
    }
    return NULL;
}

const RecipeDef* brz_recipe_find(const ParsedConfig* cfg, const char* name)
{
    if(!cfg || !name) return NULL;
    for(size_t i=0;i<cfg->recipes.len;i++){
        const RecipeDef* r=(const RecipeDef*)brz_vec_cat(&cfg->recipes,i);
        if(r->name && brz_streq(r->name,name)) return r;
    }
    return NULL;
}

const ActionDef* brz_action_find(const ParsedConfig* cfg, const char* name)
{
    if(!cfg || !name) return NULL;
    for(size_t i=0;i<cfg->actions.len;i++){
        const ActionDef* a=(const ActionDef*)brz_vec_cat(&cfg->actions,i);
        if(a->name && brz_streq(a->name,name)) return a;
    }
    return NULL;
}

int brz_action_code(const char* name)
{
    if(brz_streq(name,"gather")) return BRZ_ACTION_GATHER;
    if(brz_streq(name,"craft")) return BRZ_ACTION_CRAFT;
    if(brz_streq(name,"trade")) return BRZ_ACTION_TRADE;
    if(brz_streq(name,"rest")) return BRZ_ACTION_REST;
    if(brz_streq(name,"move_to")) return BRZ_ACTION_MOVE_TO;
    if(brz_streq(name,"roam")) return BRZ_ACTION_ROAM;
    if(brz_streq(name,"wander")) return BRZ_ACTION_WANDER;
    if(brz_streq(name,"deposit")) return BRZ_ACTION_DEPOSIT;
    if(brz_streq(name,"eat")) return BRZ_ACTION_EAT;
    return BRZ_ACTION_INVALID;
}

uint16_t brz_terrain_query(const char* name)
{
    if(brz_streq(name,"coast")) return 1u;
    if(brz_streq(name,"field")) return 2u;
    if(brz_streq(name,"forest")) return 4u;
    if(brz_streq(name,"claypit")) return 8u;
    if(brz_streq(name,"mine_copper")) return 16u;
    if(brz_streq(name,"mine_tin")) return 32u;
    if(brz_streq(name,"fire")) return 64u;
    return 0;
}

static int named_exists(const BrzVec* values, size_t name_offset, const char* name)
{
    if(!name) return 0;
    for(size_t i=0;i<values->len;i++){
        const char* const* candidate=(const char* const*)((const char*)brz_vec_cat(values,i)+name_offset);
        if(*candidate && brz_streq(*candidate,name)) return 1;
    }
    return 0;
}

static int condition_atom(const char* text, BrzConditionKind* kind, BrzCompareCode* cmp, double* value)
{
    char name[32]={0}, op[3]={0}; double number=0;
    while(*text==' ') text++;
    if(strcmp(text,"true")==0){ *kind=BRZ_COND_TRUE; *cmp=BRZ_CMP_TRUTHY; *value=1; return 1; }
    if(strcmp(text,"false")==0){ *kind=BRZ_COND_FALSE; *cmp=BRZ_CMP_TRUTHY; *value=0; return 1; }
    if(strcmp(text,"hungry")==0){ *kind=BRZ_COND_HUNGER; *cmp=BRZ_CMP_GT; *value=.7; return 1; }
    if(sscanf(text,"prob %lf",&number)==1 || sscanf(text,"chance(%lf)",&number)==1){ *kind=BRZ_COND_PROB; *cmp=BRZ_CMP_TRUTHY; *value=number; return number>=0 && number<=1; }
    if(sscanf(text,"%31s %2[<>=!] %lf",name,op,&number)!=3) return 0;
    if(brz_streq(name,"hunger")) *kind=BRZ_COND_HUNGER;
    else if(brz_streq(name,"fatigue")) *kind=BRZ_COND_FATIGUE;
    else return 0;
    if(strcmp(op,">")==0) *cmp=BRZ_CMP_GT; else if(strcmp(op,"<")==0) *cmp=BRZ_CMP_LT;
    else if(strcmp(op,">=")==0) *cmp=BRZ_CMP_GE; else if(strcmp(op,"<=")==0) *cmp=BRZ_CMP_LE;
    else if(strcmp(op,"==")==0) *cmp=BRZ_CMP_EQ; else if(strcmp(op,"!=")==0) *cmp=BRZ_CMP_NE; else return 0;
    *value=number; return 1;
}

int brz_condition_compile(const char* expression, CompiledCondition* condition, int line, FILE* errors)
{
    const char* join; char left[128], right[128]; size_t n;
    if(!condition) return 0; memset(condition,0,sizeof(*condition));
    if(!expression || !*expression) expression="true";
    join=strstr(expression," and "); condition->join_or=0;
    if(!join){ join=strstr(expression," or "); condition->join_or=1; }
    if(join){ n=(size_t)(join-expression); if(n>=sizeof(left)) goto bad; memcpy(left,expression,n); left[n]=0;
        strncpy(right,join+(condition->join_or?4:5),sizeof(right)-1); right[sizeof(right)-1]=0; condition->terms=2;
        if(!condition_atom(left,&condition->kind[0],&condition->comparison[0],&condition->value[0]) || !condition_atom(right,&condition->kind[1],&condition->comparison[1],&condition->value[1])) goto bad;
    } else { condition->terms=1; if(!condition_atom(expression,&condition->kind[0],&condition->comparison[0],&condition->value[0])) goto bad; }
    return 1;
bad: if(errors) fprintf(errors,"ValidationError:%d: invalid or unknown condition '%s'\n",line,expression); return 0;
}

static int validate_stmts(const ParsedConfig* cfg, const BrzVec* statements, const char* vocation, FILE* errors)
{
    int ok=1;
    for(size_t i=0;i<statements->len;i++){
        const StmtDef* st=(const StmtDef*)brz_vec_cat(statements,i);
        if(st->kind==ST_OP && cfg->actions.len){
            const ActionDef* action=brz_action_find(cfg,st->as.op.op);
            int argc=(st->as.op.a0?1:0)+(st->as.op.a1?1:0)+(st->as.op.a2?1:0);
            if(!action){
                fprintf(errors,"ValidationError:%d: vocation '%s' uses undeclared action '%s'\n",st->line,vocation,st->as.op.op); ok=0;
            } else if(argc<action->min_args || argc>action->max_args) {
                fprintf(errors,"ValidationError:%d: action '%s' has %d arguments; expected %d..%d\n",st->line,action->name,argc,action->min_args,action->max_args); ok=0;
            }
        } else if(st->kind==ST_CHANCE) {
            if(!validate_stmts(cfg,&st->as.chance.body,vocation,errors)) ok=0;
        } else if(st->kind==ST_WHEN) {
            if(!validate_stmts(cfg,&st->as.when_stmt.body,vocation,errors)) ok=0;
        }
    }
    return ok;
}

bool brz_cfg_validate(const ParsedConfig* cfg, FILE* errors)
{
    int ok=1;
    if(!errors) errors=stderr;
    if(cfg->language_version != 1){ fprintf(errors,"ValidationError: unsupported scenario language version %d\n",cfg->language_version); ok=0; }
    for(size_t i=0;i<cfg->resources.len;i++){
        const ResourceDef* r=(const ResourceDef*)brz_vec_cat(&cfg->resources,i);
        if(kind_table_find(&cfg->resource_kinds,r->name)<0){
            fprintf(errors,"ValidationError:%d: resource '%s' is not declared in kinds.resources\n",r->line,r->name); ok=0;
        }
        if(r->habitat && !brz_terrain_query(r->habitat)){
            fprintf(errors,"ValidationError:%d: resource '%s' has unknown terrain query '%s'\n",r->line,r->name,r->habitat); ok=0;
        }
    }
    if(cfg->settlement_policy.food_fallback &&
       kind_table_find(&cfg->resource_kinds,cfg->settlement_policy.food_fallback)<0){
        fprintf(errors,"ValidationError: settlement food_fallback '%s' is not a declared resource\n",
                cfg->settlement_policy.food_fallback); ok=0;
    }
    if(cfg->settlement_policy.deposit_threshold<0 || cfg->settlement_policy.rest_recovery<0){
        fprintf(errors,"ValidationError: settlement policy values must be non-negative\n"); ok=0;
    }
    for(size_t i=0;i<cfg->recipes.len;i++){
        const RecipeDef* r=(const RecipeDef*)brz_vec_cat(&cfg->recipes,i);
        if(kind_table_find(&cfg->item_kinds,r->output)<0 && kind_table_find(&cfg->resource_kinds,r->output)<0){
            fprintf(errors,"ValidationError:%d: recipe '%s' has unknown output '%s'\n",r->line,r->name,r->output); ok=0;
        }
        for(size_t j=0;j<r->inputs.len;j++){
            const RecipeInputDef* in=(const RecipeInputDef*)brz_vec_cat(&r->inputs,j);
            if(kind_table_find(&cfg->item_kinds,in->kind)<0 && kind_table_find(&cfg->resource_kinds,in->kind)<0){
                fprintf(errors,"ValidationError:%d: recipe '%s' has unknown input '%s'\n",r->line,r->name,in->kind); ok=0;
            }
        }
    }
    for(size_t vi=0;vi<cfg->vocations.len;vi++){
        const VocationDef* v=(const VocationDef*)brz_vec_cat(&cfg->vocations,vi);
        for(size_t ti=0;ti<v->tasks.len;ti++)
            if(!validate_stmts(cfg,&((const TaskDef*)brz_vec_cat(&v->tasks,ti))->stmts,v->name,errors)) ok=0;
        for(size_t ri=0;ri<v->rules.len;ri++){
            const RuleDef* r=(const RuleDef*)brz_vec_cat(&v->rules,ri);
            if(!brz_voc_find_task((VocationDef*)v,r->do_task) && !brz_action_find(cfg,r->do_task)){
                fprintf(errors,"ValidationError: vocation '%s' rule '%s' references unknown task or action '%s'\n",v->name,r->name,r->do_task); ok=0;
            }
        }
    }
    for(size_t i=0;i<cfg->places.len;i++){
        const PlaceDef* place=(const PlaceDef*)brz_vec_cat(&cfg->places,i);
        if(place->map_ref && !named_exists(&cfg->map_refs,offsetof(MapRefDef,id),place->map_ref)){
            fprintf(errors,"ValidationError:%d: place '%s' references unknown map '%s'\n",place->line,place->name,place->map_ref); ok=0;
        }
    }
    for(size_t i=0;i<cfg->roles.len;i++){
        const RoleDef* role=(const RoleDef*)brz_vec_cat(&cfg->roles,i);
        if(role->workplace && !named_exists(&cfg->places,offsetof(PlaceDef,name),role->workplace)){
            fprintf(errors,"ValidationError:%d: role '%s' references unknown workplace '%s'\n",role->line,role->name,role->workplace); ok=0;
        }
    }
    for(size_t i=0;i<cfg->services.len;i++){
        const ServiceDef* service=(const ServiceDef*)brz_vec_cat(&cfg->services,i);
        if(!named_exists(&cfg->roles,offsetof(RoleDef,name),service->provider_role)){
            fprintf(errors,"ValidationError:%d: service '%s' references unknown provider role '%s'\n",service->line,service->name,service->provider_role); ok=0;
        }
        if(service->recipient_role && !named_exists(&cfg->roles,offsetof(RoleDef,name),service->recipient_role)){
            fprintf(errors,"ValidationError:%d: service '%s' references unknown recipient role '%s'\n",service->line,service->name,service->recipient_role); ok=0;
        }
        if(!named_exists(&cfg->places,offsetof(PlaceDef,name),service->place)){
            fprintf(errors,"ValidationError:%d: service '%s' references unknown place '%s'\n",service->line,service->name,service->place); ok=0;
        }
        if(service->input && kind_table_find(&cfg->resource_kinds,service->input)<0 && kind_table_find(&cfg->item_kinds,service->input)<0){
            fprintf(errors,"ValidationError:%d: service '%s' references unknown input '%s'\n",service->line,service->name,service->input); ok=0;
        }
        if(service->output && kind_table_find(&cfg->resource_kinds,service->output)<0 && kind_table_find(&cfg->item_kinds,service->output)<0){
            fprintf(errors,"ValidationError:%d: service '%s' references unknown output '%s'\n",service->line,service->name,service->output); ok=0;
        }
    }
    for(size_t i=0;i<cfg->disruptions.len;i++){
        const DisruptionDef* disruption=(const DisruptionDef*)brz_vec_cat(&cfg->disruptions,i);
        if(!named_exists(&cfg->services,offsetof(ServiceDef,name),disruption->affects)){
            fprintf(errors,"ValidationError:%d: disruption '%s' references unknown service '%s'\n",disruption->line,disruption->name,disruption->affects); ok=0;
        }
    }
    return ok!=0;
}

static int compile_stmts(ParsedConfig* cfg, BrzVec* statements, const char* vocation, FILE* errors)
{
    int ok=1;
    for(size_t i=0;i<statements->len;i++){
        StmtDef* st=(StmtDef*)brz_vec_at(statements,i);
        if(st->kind==ST_CHANCE){ if(!compile_stmts(cfg,&st->as.chance.body,vocation,errors)) ok=0; continue; }
        if(st->kind==ST_WHEN){ if(!brz_condition_compile(st->as.when_stmt.when_expr,&st->as.when_stmt.condition,st->line,errors)) ok=0; if(!compile_stmts(cfg,&st->as.when_stmt.body,vocation,errors)) ok=0; continue; }
        OpDef* op=&st->as.op;
        op->action_code=brz_action_code(op->op);
        op->arg0_resource_id=kind_table_find(&cfg->resource_kinds,op->a0);
        op->arg1_resource_id=kind_table_find(&cfg->resource_kinds,op->a1);
        op->arg0_item_id=kind_table_find(&cfg->item_kinds,op->a0);
        op->arg1_item_id=kind_table_find(&cfg->item_kinds,op->a1);
        op->terrain_query=0;
        op->recipe_index=-1;
        if(op->action_code==BRZ_ACTION_INVALID){
            fprintf(errors,"ValidationError:%d: vocation '%s' uses unsupported action '%s'\n",st->line,vocation,op->op); ok=0; continue;
        }
        if(op->action_code==BRZ_ACTION_GATHER || op->action_code==BRZ_ACTION_DEPOSIT || op->action_code==BRZ_ACTION_EAT){
            if(op->arg0_resource_id<0){ fprintf(errors,"ValidationError:%d: action '%s' requires a declared resource '%s'\n",st->line,op->op,op->a0); ok=0; }
        } else if(op->action_code==BRZ_ACTION_CRAFT){
            for(size_t ri=0;ri<cfg->recipes.len;ri++){
                RecipeDef* recipe=(RecipeDef*)brz_vec_at(&cfg->recipes,ri);
                if(brz_streq(recipe->name,op->a0)){ op->recipe_index=(int)ri; break; }
            }
            if(op->recipe_index<0 && op->arg0_item_id<0){ fprintf(errors,"ValidationError:%d: craft requires a declared recipe or item '%s'\n",st->line,op->a0); ok=0; }
        } else if(op->action_code==BRZ_ACTION_TRADE){
            if((op->arg0_resource_id<0 && op->arg0_item_id<0) || (op->arg1_resource_id<0 && op->arg1_item_id<0)){
                fprintf(errors,"ValidationError:%d: trade requires declared give and want kinds\n",st->line); ok=0;
            }
        } else if(op->action_code==BRZ_ACTION_MOVE_TO || op->action_code==BRZ_ACTION_ROAM || op->action_code==BRZ_ACTION_WANDER){
            op->terrain_query=brz_terrain_query(op->a0);
            if(!op->terrain_query){ fprintf(errors,"ValidationError:%d: action '%s' has unknown terrain query '%s'\n",st->line,op->op,op->a0); ok=0; }
        }
    }
    return ok;
}

bool brz_cfg_compile(ParsedConfig* cfg, FILE* errors)
{
    int ok=1;
    if(!cfg) return false;
    if(!errors) errors=stderr;
    for(size_t i=0;i<cfg->actions.len;i++){
        ActionDef* action=(ActionDef*)brz_vec_at(&cfg->actions,i);
        action->code=brz_action_code(action->name);
        if(action->code==BRZ_ACTION_INVALID){
            fprintf(errors,"ValidationError:%d: unsupported declared action '%s'\n",action->line,action->name); ok=0;
        }
    }
    for(size_t i=0;i<cfg->resources.len;i++){
        ResourceDef* resource=(ResourceDef*)brz_vec_at(&cfg->resources,i);
        resource->kind_id=kind_table_find(&cfg->resource_kinds,resource->name);
        resource->terrain_query=brz_terrain_query(resource->habitat);
    }
    for(size_t i=0;i<cfg->recipes.len;i++){
        RecipeDef* recipe=(RecipeDef*)brz_vec_at(&cfg->recipes,i);
        recipe->output_resource_id=kind_table_find(&cfg->resource_kinds,recipe->output);
        recipe->output_item_id=kind_table_find(&cfg->item_kinds,recipe->output);
        for(size_t j=0;j<recipe->inputs.len;j++){
            RecipeInputDef* input=(RecipeInputDef*)brz_vec_at(&recipe->inputs,j);
            input->resource_id=kind_table_find(&cfg->resource_kinds,input->kind);
            input->item_id=kind_table_find(&cfg->item_kinds,input->kind);
        }
    }
    for(size_t i=0;i<cfg->vocations.len;i++){
        VocationDef* vocation=(VocationDef*)brz_vec_at(&cfg->vocations,i);
        for(size_t j=0;j<vocation->rules.len;j++){
            RuleDef* rule=(RuleDef*)brz_vec_at(&vocation->rules,j);
            if(!brz_condition_compile(rule->when_expr,&rule->condition,rule->line,errors)) ok=0;
        }
        for(size_t j=0;j<vocation->tasks.len;j++)
            if(!compile_stmts(cfg,&((TaskDef*)brz_vec_at(&vocation->tasks,j))->stmts,vocation->name,errors)) ok=0;
    }
    return ok;
}
