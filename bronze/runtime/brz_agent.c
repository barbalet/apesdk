
#include "brz_agent.h"
#include "brz_kinds.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

/* ---- Small expression evaluator and DSL executor ported from old brz_sim.c ---- */

static double clamp01(double v){ if(v<0) return 0; if(v>1) return 1; return v; }

static void ex_skip(const char** s){ while(**s && isspace((unsigned char)**s)) (*s)++; }
static int ex_peek_word(const char* s, const char* w){
    ex_skip(&s);
    size_t n=strlen(w);
    return strncmp(s,w,n)==0 && (s[n]==0 || isspace((unsigned char)s[n]) || s[n]==')' || s[n]=='(');
}
static int ex_consume_word(const char** s, const char* w){
    ex_skip(s);
    size_t n=strlen(w);
    if(strncmp(*s,w,n)==0){ *s += n; return 1; }
    return 0;
}
static int ex_read_ident(const char** s, char* out, int out_n){
    ex_skip(s);
    int i=0;
    if(!(isalpha((unsigned char)**s) || **s=='_')) return 0;
    while(**s && (isalnum((unsigned char)**s) || **s=='_' || **s=='.')){
        if(i<out_n-1) out[i++] = **s;
        (*s)++;
    }
    out[i]=0;
    return 1;
}
static int ex_read_num(const char** s, double* out){
    ex_skip(s);
    char* end=NULL;
    double v = strtod(*s,&end);
    if(end==*s) return 0;
    *out=v; *s=end; return 1;
}
static int ex_read_op(const char** s, char* op2){
    ex_skip(s);
    if(strncmp(*s,">=",2)==0 || strncmp(*s,"<=",2)==0 || strncmp(*s,"==",2)==0 || strncmp(*s,"!=",2)==0){
        op2[0]=(*s)[0]; op2[1]=(*s)[1]; op2[2]=0; *s += 2; return 1;
    }
    if(**s=='>'||**s=='<'){
        op2[0]=**s; op2[1]=0; (*s)++; return 1;
    }
    return 0;
}

static double agent_var(const BrzAgent* a, const ParsedConfig* cfg, const char* name){
    (void)cfg;
    if(brz_streq(name,"hunger")) return a->hunger;
    if(brz_streq(name,"fatigue")) return a->fatigue;
    return 0.0;
}

static int eval_cmp_or_prob(const char** s, const BrzAgent* a, const ParsedConfig* cfg, BrzRng* rng){
    /* Supports:
       - hunger < 0.5
       - fatigue >= 0.2
       - chance(0.3)
    */
    ex_skip(s);
    if(ex_consume_word(s,"chance")){
        ex_skip(s);
        if(**s=='('){ (*s)++; }
        double p=0;
        if(!ex_read_num(s,&p)) return 0;
        ex_skip(s);
        if(**s==')') (*s)++;
        int roll = (int)(brz_rng_u32(rng)%10000u);
        int thresh = (int)(clamp01(p)*10000.0);
        return roll < thresh;
    }

    char ident[128];
    if(!ex_read_ident(s,ident,sizeof(ident))) return 0;

    if(brz_streq(ident,"true")) return 1;
    if(brz_streq(ident,"false")) return 0;
    /* Legacy scenarios use `prob 0.25`; retain it as the concise form of
       chance(0.25) while compiled scenario conditions remain deterministic. */
    if(brz_streq(ident,"prob")){
        double p=0; int roll;
        if(!ex_read_num(s,&p)) return 0;
        roll=(int)(brz_rng_u32(rng)%10000u);
        return roll < (int)(clamp01(p)*10000.0);
    }

    double lhs = agent_var(a,cfg,ident);
    char op2[3]={0};
    if(!ex_read_op(s,op2)) return lhs!=0.0; /* truthy */
    double rhs=0;
    if(!ex_read_num(s,&rhs)) return 0;

    if(strcmp(op2,">")==0) return lhs>rhs;
    if(strcmp(op2,"<")==0) return lhs<rhs;
    if(strcmp(op2,">=")==0) return lhs>=rhs;
    if(strcmp(op2,"<=")==0) return lhs<=rhs;
    if(strcmp(op2,"==")==0) return lhs==rhs;
    if(strcmp(op2,"!=")==0) return lhs!=rhs;
    return 0;
}

static int eval_atom(const char** s, const BrzAgent* a, const ParsedConfig* cfg, BrzRng* rng){
    ex_skip(s);
    if(**s=='('){ (*s)++; int v = eval_cmp_or_prob(s,a,cfg,rng); ex_skip(s); if(**s==')') (*s)++; return v; }
    return eval_cmp_or_prob(s,a,cfg,rng);
}
static int eval_and(const char** s, const BrzAgent* a, const ParsedConfig* cfg, BrzRng* rng){
    int v = eval_atom(s,a,cfg,rng);
    for(;;){
        if(ex_peek_word(*s,"and")){ ex_consume_word(s,"and"); v = v && eval_atom(s,a,cfg,rng); }
        else break;
    }
    return v;
}
static int eval_or(const char** s, const BrzAgent* a, const ParsedConfig* cfg, BrzRng* rng){
    int v = eval_and(s,a,cfg,rng);
    for(;;){
        if(ex_peek_word(*s,"or")){ ex_consume_word(s,"or"); v = v || eval_and(s,a,cfg,rng); }
        else break;
    }
    return v;
}
static int eval_when_expr(const char* expr, const BrzAgent* a, const ParsedConfig* cfg, BrzRng* rng){
    if(!expr || !expr[0]) return 1;
    const char* s = expr;
    int v = eval_or(&s,a,cfg,rng);
    return v ? 1 : 0;
}

/* ---- Inventory helpers ---- */

static int res_id(const ParsedConfig* cfg, const char* name){
    return kind_table_find(&cfg->resource_kinds, name);
}
static int item_id(const ParsedConfig* cfg, const char* name){
    return kind_table_find(&cfg->item_kinds, name);
}

static void agent_add_res(BrzAgent* a, int rid, double amt){
    if(rid<0 || (size_t)rid>=a->res_n) return;
    a->res_inv[rid] += amt;
    if(a->res_inv[rid] < 0) a->res_inv[rid] = 0;
}
static void agent_add_item(BrzAgent* a, int iid, double amt){
    if(iid<0 || (size_t)iid>=a->item_n) return;
    a->item_inv[iid] += amt;
    if(a->item_inv[iid] < 0) a->item_inv[iid] = 0;
}

/* ---- Recipes (compiled from scenario content) ---- */
static int craft_with_recipes(BrzAgent* a, const ParsedConfig* cfg, const char* item_name, double n){
    const RecipeDef* recipe=brz_recipe_find(cfg,item_name);
    if(!recipe || recipe->output_amount<=0) return 0;
    double batches=n;
    for(size_t i=0;i<recipe->inputs.len;i++){
        const RecipeInputDef* in=(const RecipeInputDef*)brz_vec_cat(&recipe->inputs,i);
        int rid=res_id(cfg,in->kind), iid=item_id(cfg,in->kind);
        double available=(rid>=0)?a->res_inv[rid]:(iid>=0?a->item_inv[iid]:0.0);
        if(in->amount>0 && available/in->amount<batches) batches=available/in->amount;
    }
    if(batches<=0) return 1; /* valid recipe but insufficient inputs */
    for(size_t i=0;i<recipe->inputs.len;i++){
        const RecipeInputDef* in=(const RecipeInputDef*)brz_vec_cat(&recipe->inputs,i);
        int rid=res_id(cfg,in->kind), iid=item_id(cfg,in->kind);
        if(rid>=0) a->res_inv[rid]-=batches*in->amount;
        else if(iid>=0) a->item_inv[iid]-=batches*in->amount;
    }
    int out_r=res_id(cfg,recipe->output), out_i=item_id(cfg,recipe->output);
    if(out_r>=0) agent_add_res(a,out_r,batches*recipe->output_amount);
    else if(out_i>=0) agent_add_item(a,out_i,batches*recipe->output_amount);
    else return 0;
    return 1;
}

/* ---- Action execution against world/settlements ---- */

static uint16_t tag_for_habitat(const char* arg0){
    if(!arg0) return BRZ_TAG_FOREST;
    if(brz_streq(arg0,"coast")) return BRZ_TAG_COAST;
    if(brz_streq(arg0,"field")) return BRZ_TAG_FIELD;
    if(brz_streq(arg0,"forest")) return BRZ_TAG_FOREST;
    if(brz_streq(arg0,"claypit")) return BRZ_TAG_CLAYPIT;
    if(brz_streq(arg0,"mine_copper")) return BRZ_TAG_MINE_CU;
    if(brz_streq(arg0,"mine_tin")) return BRZ_TAG_MINE_SN;
    return BRZ_TAG_FOREST;
}


static int agent_at_settlement(const BrzAgent* a, const BrzSettlement* s){
    return brz_dist_manhattan(a->pos, s->pos) <= 1;
}

static void exec_op(BrzAgent* a, const ParsedConfig* cfg, BrzWorld* world,
                    BrzSettlement* setts, int sett_n, const OpDef* op, BrzRng* rng,
                    const BronzeEventSink* events, int day)
{
    (void)rng;
    const char* opname = op->op ? op->op : "";
    const char* arg0 = op->a0 ? op->a0 : "";
    const char* arg1 = op->a1 ? op->a1 : "";
    double n = (op->has_n0 ? op->n0 : 1.0);

    if(brz_streq(opname, "gather"))
    {
        int rid = res_id(cfg, arg0);
        double gathered=0;
        if(rid >= 0){
            const ResourceDef* resource=brz_resource_find(cfg,arg0);
            uint16_t need = tag_for_habitat(resource ? resource->habitat : NULL);
            if(need){
                if(!(brz_world_tags_at(world, a->pos) & need)){
                    /* set target toward nearest suitable tile */
                    a->target = brz_world_find_nearest_tag(world, a->pos, need, 32);
                    a->has_target = 1;
                }else{
                    gathered = brz_world_take(world, a->pos, a->res_n, rid, n);
                    agent_add_res(a, rid, gathered);
                }
            }else{
                gathered = brz_world_take(world, a->pos, a->res_n, rid, n);
                agent_add_res(a, rid, gathered);
            }
        }
        a->fatigue += 0.04 + 0.005 * n;
        a->hunger  += 0.02;
        if(gathered>0) bronze_event_emit(events,BRZ_EVENT_GATHERED,a->id,a->home_settlement,arg0,gathered,day);
    }
    else if(brz_streq(opname, "craft"))
    {
        /* crafting mostly at settlement, but allow anywhere */
        if(!craft_with_recipes(a, cfg, arg0, n)){
            int iid = item_id(cfg, arg0);
            if(iid >= 0) agent_add_item(a, iid, n);
        }
        a->fatigue += 0.05 + 0.01 * n;
        a->hunger  += 0.02;
        bronze_event_emit(events,BRZ_EVENT_CRAFTED,a->id,a->home_settlement,arg0,n,day);
    }
    else if(brz_streq(opname, "trade"))
    {
        /* trade give(arg0) for want(arg1) through settlement market */
        int si = brz_find_nearest_settlement(setts, sett_n, a->pos);
        if(si >= 0 && agent_at_settlement(a, &setts[si])){
            BrzSettlement* s = &setts[si];
            int give_r = res_id(cfg,arg0);
            int want_r = res_id(cfg,arg1);
            int give_i = item_id(cfg,arg0);
            int want_i = item_id(cfg,arg1);

            double give_amt = 1.0;
            if(give_r>=0 && a->res_inv[give_r] >= give_amt){
                double pg = brz_settlement_price_res(s, cfg, give_r);
                double pw = (want_r>=0) ? brz_settlement_price_res(s, cfg, want_r)
                                        : (want_i>=0 ? brz_settlement_price_item(s, want_i) : 1.0);
                double want_amt = (pw>0? (give_amt*pg/pw) : 0.0);
                if(want_amt <= 0) want_amt = 0;
                /* settlement accepts give */
                a->res_inv[give_r] -= give_amt;
                s->res_inv[give_r] += give_amt;
                /* settlement pays out want if stock */
                if(want_r>=0){
                    double pay = want_amt;
                    if(s->res_inv[want_r] < pay) pay = s->res_inv[want_r];
                    s->res_inv[want_r] -= pay;
                    a->res_inv[want_r] += pay;
                }else if(want_i>=0){
                    double pay = want_amt;
                    if(s->item_inv[want_i] < pay) pay = s->item_inv[want_i];
                    s->item_inv[want_i] -= pay;
                    a->item_inv[want_i] += pay;
                }
            }else if(give_i>=0 && a->item_inv[give_i] >= give_amt){
                double pg = brz_settlement_price_item(s, give_i);
                double pw = (want_r>=0) ? brz_settlement_price_res(s, cfg, want_r)
                                        : (want_i>=0 ? brz_settlement_price_item(s, want_i) : 1.0);
                double want_amt = (pw>0? (give_amt*pg/pw) : 0.0);
                a->item_inv[give_i] -= give_amt;
                s->item_inv[give_i] += give_amt;
                if(want_r>=0){
                    double pay = want_amt;
                    if(s->res_inv[want_r] < pay) pay = s->res_inv[want_r];
                    s->res_inv[want_r] -= pay;
                    a->res_inv[want_r] += pay;
                }else if(want_i>=0){
                    double pay = want_amt;
                    if(s->item_inv[want_i] < pay) pay = s->item_inv[want_i];
                    s->item_inv[want_i] -= pay;
                    a->item_inv[want_i] += pay;
                }
            }
        }else{
            /* move toward nearest settlement */
            if(si >= 0){
                a->target = setts[si].pos;
                a->has_target = 1;
            }
        }
        a->fatigue += 0.02;
        a->hunger  += 0.01;
        bronze_event_emit(events,BRZ_EVENT_TRADED,a->id,si,arg0,1.0,day);
    }
    else if(brz_streq(opname, "rest"))
    {
        a->fatigue -= 0.1;
        if(a->fatigue < 0) a->fatigue = 0;
        a->hunger += 0.01;
        bronze_event_emit(events,BRZ_EVENT_RESTED,a->id,a->home_settlement,"rest",1.0,day);
    }
    else if(brz_streq(opname, "move_to") || brz_streq(opname, "roam") || brz_streq(opname,"wander"))
    {
        uint16_t t = tag_for_habitat(arg0);
        if(!a->has_target || brz_dist_manhattan(a->pos,a->target) == 0){
            a->target = brz_world_find_nearest_tag(world, a->pos, t, 32);
            a->has_target = 1;
        }
        a->pos = brz_step_toward(a->pos, a->target);
        if(brz_dist_manhattan(a->pos,a->target)==0) a->has_target = 0;
        a->fatigue += 0.04;
        a->hunger  += 0.01;
    }
}


/* statement execution */

static void exec_stmt(BrzAgent* a, const ParsedConfig* cfg, BrzWorld* world, BrzSettlement* setts, int sett_n,
                      const StmtDef* st, BrzRng* rng, const BronzeEventSink* events, int day);

static void exec_stmts_vec(BrzAgent* a, const ParsedConfig* cfg, BrzWorld* world, BrzSettlement* setts, int sett_n,
                           const BrzVec* stmts, BrzRng* rng, const BronzeEventSink* events, int day)
{
    for(size_t i=0;i<stmts->len;i++){
        const StmtDef* st = (const StmtDef*)brz_vec_cat(stmts, i);
        exec_stmt(a, cfg, world, setts, sett_n, st, rng, events, day);
    }
}

static void exec_stmt(BrzAgent* a, const ParsedConfig* cfg, BrzWorld* world, BrzSettlement* setts, int sett_n,
                      const StmtDef* st, BrzRng* rng, const BronzeEventSink* events, int day)
{
    if(st->kind == ST_OP){
        exec_op(a, cfg, world, setts, sett_n, &st->as.op, rng, events, day);
    }else if(st->kind == ST_CHANCE){
        /* percent 0..100 */
        double pct = st->as.chance.chance_pct;
        if(pct < 0) { pct = 0; }
        if(pct > 100) { pct = 100; }
        int roll = (int)(brz_rng_u32(rng)%10000u);
        int thr = (int)((pct/100.0)*10000.0);
        if(roll < thr){
            exec_stmts_vec(a, cfg, world, setts, sett_n, &st->as.chance.body, rng, events, day);
        }
    }else if(st->kind == ST_WHEN){
        if(eval_when_expr(st->as.when_stmt.when_expr, a, cfg, rng)){
            exec_stmts_vec(a, cfg, world, setts, sett_n, &st->as.when_stmt.body, rng, events, day);
        }
    }
}

/* auto-eat from own resources and settlement */
static void agent_auto_eat(BrzAgent* a, const ParsedConfig* cfg, BrzSettlement* setts, int sett_n)
{
    int si = (sett_n>0) ? a->home_settlement : -1;

    if(a->hunger > 0.7)
    {
        double eat = 0.0; int selected=-1;
        for(size_t rid=0;rid<a->res_n;rid++){
            const char* name=kind_table_name(&cfg->resource_kinds,(int)rid);
            const ResourceDef* def=brz_resource_find(cfg,name);
            if(def && def->nutrition>0 && a->res_inv[rid]>0){ selected=(int)rid; eat=def->nutrition; break; }
        }
        if(selected>=0) a->res_inv[selected]-=1;
        if(eat<=0.0 && si>=0 && agent_at_settlement(a,&setts[si])){
            for(size_t rid=0;rid<a->res_n;rid++){
                const char* name=kind_table_name(&cfg->resource_kinds,(int)rid);
                const ResourceDef* def=brz_resource_find(cfg,name);
                if(def && def->nutrition>0 && setts[si].res_inv[rid]>0){ setts[si].res_inv[rid]-=1; eat=def->nutrition; break; }
            }
        }
        a->hunger -= eat;
        if(a->hunger < 0) a->hunger = 0;
    }
}

/* auto-rest: when at home settlement, reduce fatigue (keeps agents active long-term) */
static void agent_auto_rest(BrzAgent* a, const ParsedConfig* cfg, BrzSettlement* setts, int sett_n)
{
    if(sett_n<=0) return;
    int si = a->home_settlement;
    if(si<0 || si>=sett_n) return;

    if(agent_at_settlement(a, &setts[si])){
        /* a little recovery every day at home */
        double recovery=cfg->settlement_policy.rest_recovery;
        if(recovery<=0) recovery=0.04;
        a->fatigue -= recovery;
        /* if exhausted, recover more aggressively */
        if(a->fatigue > 0.85) a->fatigue -= 0.10;
        if(a->fatigue < 0.0) a->fatigue = 0.0;
    }
}


/* rule selection: first matching 'when' or fallback first */

/* weighted rule selection among matching whens (or all if no when) */
static const RuleDef* pick_rule(const BrzAgent* a, const ParsedConfig* cfg, BrzRng* rng)
{
    (void)cfg;
    if(!a->voc) return NULL;

    double total_w = 0.0;
    /* first pass: compute total weight of matching rules */
    for(size_t i=0;i<a->voc->rules.len;i++){
        const RuleDef* r = (const RuleDef*)brz_vec_cat(&a->voc->rules, i);
        int ok = 1;
        if(r->when_expr && r->when_expr[0]){
            ok = eval_when_expr(r->when_expr, a, cfg, rng);
        }
        if(ok){
            int w = (r->weight > 0) ? r->weight : 1;
            total_w += (double)w;
        }
    }
    if(total_w <= 0.0) return NULL;

    double pick = (double)(brz_rng_u32(rng)%100000u) / 100000.0 * total_w;
    double cur = 0.0;
    for(size_t i=0;i<a->voc->rules.len;i++){
        const RuleDef* r = (const RuleDef*)brz_vec_cat(&a->voc->rules, i);
        int ok = 1;
        if(r->when_expr && r->when_expr[0]){
            ok = eval_when_expr(r->when_expr, a, cfg, rng);
        }
        if(ok){
            int w = (r->weight > 0) ? r->weight : 1;
            cur += (double)w;
            if(cur >= pick) return r;
        }
    }
    /* fallback */
    return (const RuleDef*)brz_vec_cat(&a->voc->rules, 0);
}



int brz_agents_alloc_and_spawn(BrzAgent** out, int agent_n, const ParsedConfig* cfg,
                               const BrzSettlement* setts, int sett_n,
                               size_t res_n, size_t item_n, unsigned seed)
{
    *out = (BrzAgent*)calloc((size_t)agent_n, sizeof(BrzAgent));
    if(!*out) return 1;

    BrzRng rng; brz_rng_seed(&rng, seed?seed:0xC0FFEEu);

    for(int i=0;i<agent_n;i++){
        BrzAgent* a = &(*out)[i];
        a->id = (uint32_t)i;
        a->voc = (const VocationDef*)brz_vec_cat(&cfg->vocations, (size_t)i % cfg->vocations.len);
        a->home_settlement = (sett_n>0) ? (i % sett_n) : 0;
        a->pos = (sett_n>0) ? setts[a->home_settlement].pos : (BrzPos){ brz_rng_range(&rng,0,50), brz_rng_range(&rng,0,50) };
        a->hunger = 0.3 + 0.4*(double)(brz_rng_u32(&rng)%1000u)/1000.0;
        a->fatigue = 0.2;
        a->res_n = res_n; a->item_n = item_n;
        a->res_inv = (double*)calloc(res_n, sizeof(double));
        a->item_inv = (double*)calloc(item_n, sizeof(double));
        if(!a->res_inv || !a->item_inv) return 1;
    }
    return 0;
}

void brz_agents_free(BrzAgent* agents, int agent_n){
    if(!agents) return;
    for(int i=0;i<agent_n;i++){
        free(agents[i].res_inv);
        free(agents[i].item_inv);
    }
    free(agents);
}

void brz_agent_step(BrzAgent* a, const ParsedConfig* cfg, BrzWorld* world,
                    BrzSettlement* setts, int sett_n, BrzRng* rng,
                    const BronzeEventSink* events, int day)
{
    /* baseline drift (daily metabolism + rest)
       NOTE: fatigue naturally recovers a bit each day; hard work re-adds fatigue. */
    a->hunger  = clamp01(a->hunger + 0.02);
    a->fatigue = clamp01(a->fatigue + 0.01 - 0.015);

    /* execute one rule per day */
    const RuleDef* r = pick_rule(a, cfg, rng);
    if(r && r->do_task){
        bronze_event_emit(events,BRZ_EVENT_OCCUPATION_SELECTED,a->id,a->home_settlement,
                          r->do_task,1.0,day);
        TaskDef* t = brz_voc_find_task((VocationDef*)a->voc, r->do_task);
        if(t){
            exec_stmts_vec(a, cfg, world, setts, sett_n, &t->stmts, rng, events, day);
        }else if(brz_action_find(cfg,r->do_task)){
            OpDef action; memset(&action,0,sizeof(action));
            action.op=r->do_task; action.line=r->line;
            exec_op(a,cfg,world,setts,sett_n,&action,rng,events,day);
        }
    }

    /* movement toward target if set */
    if(a->has_target){
        a->pos = brz_step_toward(a->pos, a->target);
        if(brz_dist_manhattan(a->pos,a->target)==0) a->has_target = 0;
    }

    /* clamp positions */
    a->pos.x = brz_clamp_i(a->pos.x, 0, world->w-1);
    a->pos.y = brz_clamp_i(a->pos.y, 0, world->h-1);

    agent_auto_rest(a, cfg, setts, sett_n);
    agent_auto_eat(a, cfg, setts, sett_n);

    /* Deposit policy is scenario data: food resources above its threshold are
       communal stock, rather than a special case for named resources. */
    int si = (sett_n>0) ? a->home_settlement : -1;
    if(si>=0 && agent_at_settlement(a,&setts[si])){
        double threshold=cfg->settlement_policy.deposit_threshold;
        if(threshold<=0) threshold=2;
        for(size_t rid=0;rid<a->res_n;rid++){
            const ResourceDef* def=brz_resource_find(cfg,kind_table_name(&cfg->resource_kinds,(int)rid));
            if(def && def->nutrition>0 && a->res_inv[rid]>threshold){
                double move=floor(a->res_inv[rid]-threshold);
                a->res_inv[rid]-=move; setts[si].res_inv[rid]+=move;
                if(move>0) bronze_event_emit(events,BRZ_EVENT_DEPOSITED,a->id,si,
                                              kind_table_name(&cfg->resource_kinds,(int)rid),move,day);
            }
        }
    }
}
