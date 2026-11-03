/* Regression coverage for the shared AST and explicit host profile boundary. */
#include "brz_parser.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void fixture(const char *text)
{
    FILE *file = fopen("profile-test.bronze", "wb");
    assert(file);
    assert(fwrite(text, 1, strlen(text), file) == strlen(text));
    assert(fclose(file) == 0);
}
int main(int argc, char **argv)
{
    ParsedConfig config;
    int pass;
    assert(argc == 3);
    for (pass=0; pass<8; pass++)
    {
        brz_cfg_init(&config);
        assert(brz_parse_file(argv[1], &config));
        assert(config.compiled && !config.legacy_runtime);
        assert(brz_cfg_compile(&config, stderr)); /* recompilation stays valid */
        assert(config.settlement_policy.deposit_threshold == 2);
        brz_cfg_free(&config);
        brz_cfg_init(&config);
        assert(brz_parse_file_legacy_runtime(argv[2], &config));
        assert(config.legacy_runtime && !config.compiled);
        assert(config.settlement_policy.deposit_threshold == 0);
        assert(config.settlement_policy.rest_recovery == 0);
        assert(!brz_cfg_compile(&config, NULL));
        assert(!config.compiled);
        brz_cfg_free(&config);
    }
    fixture("actions { action rest 0 0 } vocations { vocation worker { task work { when hunger > 0.2 { chance 50 { rest } } } rule always { when true do work } } }");
    brz_cfg_init(&config);
    assert(brz_parse_file("profile-test.bronze", &config));
    {
        const VocationDef *voc = brz_vec_cat(&config.vocations, 0);
        const TaskDef *task = brz_vec_cat(&voc->tasks, 0);
        const StmtDef *when = brz_vec_cat(&task->stmts, 0);
        const StmtDef *chance = brz_vec_cat(&when->as.when_stmt.body, 0);
        const StmtDef *rest = brz_vec_cat(&chance->as.chance.body, 0);
        assert(when->kind == ST_WHEN && when->as.when_stmt.condition.terms == 1);
        assert(chance->kind == ST_CHANCE && rest->as.op.action_code == BRZ_ACTION_REST);
    }
    brz_cfg_free(&config);
    fixture("actions { action rest 0 0 } vocations { vocation worker { task work { rest } rule bad { when true do missing } } }");
    brz_cfg_init(&config); assert(!brz_parse_file("profile-test.bronze", &config)); assert(!config.compiled); brz_cfg_free(&config);
    brz_cfg_init(&config); assert(!brz_parse_file_legacy_runtime("profile-test.bronze", &config)); brz_cfg_free(&config);
    fixture("scenario { version 1 profile service } actions { action rest 0 0 } vocations { vocation worker { task work { rest } rule always { when true do work } } }");
    brz_cfg_init(&config); assert(brz_parse_file("profile-test.bronze", &config)); brz_cfg_free(&config);
    brz_cfg_init(&config); assert(!brz_parse_file_legacy_runtime("profile-test.bronze", &config)); brz_cfg_free(&config);
    remove("profile-test.bronze");
    puts("Canonical compiler profile, nested AST, failure and cleanup tests passed.");
    return 0;
}
