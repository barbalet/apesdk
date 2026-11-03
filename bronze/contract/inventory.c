/* One compiler, two explicit host profiles; no runtime behavior inferred. */
#include "brz_parser.h"
#include <stdio.h>
static void statements(const BrzVec *list)
{
    size_t i;
    for (i = 0; i < list->len; i++)
    {
        const StmtDef *s = brz_vec_cat(list, i);
        if (s->kind == ST_OP)
            printf("op\t%s\t%s\t%s\t%s\t%.17g\t%d\n",s->as.op.op,
                   s->as.op.a0 ? s->as.op.a0 : "", s->as.op.a1 ? s->as.op.a1 : "",
                   s->as.op.a2 ? s->as.op.a2 : "",s->as.op.n0,s->as.op.has_n0);
        else if (s->kind == ST_CHANCE) { printf("chance\t%.17g\n",s->as.chance.chance_pct); statements(&s->as.chance.body); }
        else if (s->kind == ST_WHEN) { printf("when\t%s\n",s->as.when_stmt.when_expr); statements(&s->as.when_stmt.body); }
    }
}
int main(int argc, char **argv)
{
    ParsedConfig c; size_t i,j;
    if (argc != 2) return 2;
    brz_cfg_init(&c);
#ifdef LEGACY_RUNTIME_PROFILE
    printf("accepted\t%d\n", brz_parse_file_legacy_runtime(argv[1], &c) ? 1 : 0);
#else
    printf("accepted\t%d\n", brz_parse_file(argv[1], &c) ? 1 : 0);
#endif
    printf("world\t%u\t%d\t%d\t%d\n",c.seed,c.agent_count,c.settlement_count,c.years);
    for (i=0;i<kind_table_count(&c.resource_kinds);i++) printf("resource_kind\t%s\n",kind_table_name(&c.resource_kinds,i));
    for (i=0;i<kind_table_count(&c.item_kinds);i++) printf("item_kind\t%s\n",kind_table_name(&c.item_kinds,i));
    for (i=0;i<c.params.len;i++) { const ParamDef *p=brz_vec_cat(&c.params,i); printf("param\t%s\t%.17g\t%s\n",p->key,p->value,p->has_svalue?p->svalue:""); }
    for (i=0;i<c.actions.len;i++) { const ActionDef *a=brz_vec_cat(&c.actions,i);printf("action\t%s\t%d\t%d\n",a->name,a->min_args,a->max_args); }
    for (i=0;i<c.resources.len;i++) { const ResourceDef *r=brz_vec_cat(&c.resources,i);printf("resource\t%s\t%s\t%.17g\t%.17g\t%.17g\t%.17g\n",r->name,r->habitat,r->capacity,r->renew,r->nutrition,r->market_target); }
    for (i=0;i<c.recipes.len;i++) {
        const RecipeDef *r=brz_vec_cat(&c.recipes,i); printf("recipe\t%s\t%s\t%.17g\n",r->name,r->output,r->output_amount);
        for(j=0;j<r->inputs.len;j++){const RecipeInputDef *in=brz_vec_cat(&r->inputs,j);printf("input\t%s\t%.17g\n",in->kind,in->amount);}
    }
    printf("policy\t%s\t%.17g\t%.17g\n",c.settlement_policy.food_fallback?c.settlement_policy.food_fallback:"",c.settlement_policy.deposit_threshold,c.settlement_policy.rest_recovery);
    for (i=0;i<c.vocations.len;i++) {
        const VocationDef *v=brz_vec_cat(&c.vocations,i);printf("vocation\t%s\n",v->name);
        for(j=0;j<v->tasks.len;j++){const TaskDef *t=brz_vec_cat(&v->tasks,j);printf("task\t%s\n",t->name);statements(&t->stmts);}
        for(j=0;j<v->rules.len;j++){const RuleDef *r=brz_vec_cat(&v->rules,j);printf("rule\t%s\t%s\t%s\t%d\n",r->name,r->when_expr?r->when_expr:"",r->do_task,r->weight);}
    }
    brz_cfg_free(&c); return 0;
}
