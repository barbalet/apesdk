
#include "brz_sim.h"
#include "brz_util.h"
#include "brz_kinds.h"
#include "brz_world.h"
#include "brz_settlement.h"
#include "brz_agent.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ---------------- config helpers ---------------- */

static const ParamDef* cfg_find_param(const ParsedConfig* cfg, const char* key)
{
    for(size_t i=0;i<cfg->params.len;i++)
    {
        const ParamDef* p = (const ParamDef*)brz_vec_cat(&cfg->params, i);
        if(p->key && brz_streq(p->key, key)) return p;
    }
    return NULL;
}

static int cfg_get_int(const ParsedConfig* cfg, const char* key, int defv)
{
    const ParamDef* p = cfg_find_param(cfg, key);
    if(!p) return defv;
    if(p->has_svalue) return defv;
    return (int)(p->value);
}

static const char* cfg_get_str(const ParsedConfig* cfg, const char* key, const char* defv)
{
    const ParamDef* p = cfg_find_param(cfg, key);
    if(!p) return defv;
    if(!p->has_svalue) return defv;
    return p->svalue ? p->svalue : defv;
}

/* ---------------- snapshot json ---------------- */

static void write_snapshot_json(const ParsedConfig* cfg,
                                const BrzWorld* world,
                                const BrzSettlement* setts, int sett_n,
                                const BrzAgent* agents, int agent_n,
                                int day, const char* filename)
{
    FILE* f = fopen(filename, "wb");
    if(!f){ fprintf(stderr, "Warning: cannot write %s\n", filename); return; }

    const size_t res_n = kind_table_count(&cfg->resource_kinds);
    const size_t item_n = kind_table_count(&cfg->item_kinds);

    /* world totals */
    double* world_tot = (double*)calloc(res_n, sizeof(double));
    if(world_tot){
        for(int y=0;y<world->h;y++){
            for(int x=0;x<world->w;x++){
                const double* r = &world->res[(size_t)(y*world->w+x)*res_n];
                for(size_t i=0;i<res_n;i++) world_tot[i] += r[i];
            }
        }
    }

    fprintf(f, "{\n");
    fprintf(f, "  \"day\": %d,\n", day);
    fprintf(f, "  \"world\": { \"w\": %d, \"h\": %d },\n", world->w, world->h);

    fprintf(f, "  \"resource_kinds\": [");
    for(size_t i=0;i<res_n;i++){
        const char* nm = kind_table_name(&cfg->resource_kinds, (int)i);
        fprintf(f, "%s\"%s\"", (i? ", ":" "), nm?nm:"");
    }
    fprintf(f, " ],\n");

    fprintf(f, "  \"item_kinds\": [");
    for(size_t i=0;i<item_n;i++){
        const char* nm = kind_table_name(&cfg->item_kinds, (int)i);
        fprintf(f, "%s\"%s\"", (i? ", ":" "), nm?nm:"");
    }
    fprintf(f, " ],\n");

    fprintf(f, "  \"world_resources_total\": [");
    for(size_t i=0;i<res_n;i++){
        double v = world_tot ? world_tot[i] : 0.0;
        fprintf(f, "%s%.3f", (i? ", ":" "), v);
    }
    fprintf(f, " ],\n");

    /* settlements */
    fprintf(f, "  \"settlements\": [\n");
    for(int si=0; si<sett_n; si++){
        fprintf(f, "    { \"name\": \"%s\", \"x\": %d, \"y\": %d, \"population\": %d,\n",
                setts[si].name, setts[si].pos.x, setts[si].pos.y, setts[si].population);
        fprintf(f, "      \"resources\": [");
        for(size_t i=0;i<res_n;i++) fprintf(f, "%s%.3f", (i? ", ":" "), setts[si].res_inv[i]);
        fprintf(f, " ],\n");
        fprintf(f, "      \"items\": [");
        for(size_t i=0;i<item_n;i++) fprintf(f, "%s%.3f", (i? ", ":" "), setts[si].item_inv[i]);
        fprintf(f, " ] }%s\n", (si==sett_n-1? "":","));
    }
    fprintf(f, "  ],\n");

    /* agents */
    fprintf(f, "  \"agents\": [\n");
    for(int ai=0; ai<agent_n; ai++){
        const BrzAgent* a = &agents[ai];
        fprintf(f, "    { \"id\": %u, \"vocation\": \"%s\", \"x\": %d, \"y\": %d, \"home\": %d, \"hunger\": %.3f, \"fatigue\": %.3f,\n",
                (unsigned)a->id,
                (a->voc && a->voc->name)?a->voc->name:"",
                a->pos.x, a->pos.y,
                a->home_settlement,
                a->hunger, a->fatigue);
        fprintf(f, "      \"resources\": [");
        for(size_t i=0;i<res_n;i++) fprintf(f, "%s%.3f", (i? ", ":" "), a->res_inv[i]);
        fprintf(f, " ],\n");
        fprintf(f, "      \"items\": [");
        for(size_t i=0;i<item_n;i++) fprintf(f, "%s%.3f", (i? ", ":" "), a->item_inv[i]);
        fprintf(f, " ] }%s\n", (ai==agent_n-1? "":","));
    }
    fprintf(f, "  ]\n");
    fprintf(f, "}\n");

    free(world_tot);
    fclose(f);
}

/* ---------------- ascii map ---------------- */

static void dump_ascii_map(const ParsedConfig* cfg,
                           const BrzWorld* world,
                           const BrzSettlement* setts, int sett_n,
                           const BrzAgent* agents, int agent_n,
                           int day, const char* filename, int w, int h)
{
    (void)cfg;
    FILE* f = fopen(filename, "wb");
    if(!f){ fprintf(stderr, "Warning: cannot write %s\n", filename); return; }

    fprintf(f, "Day %d\n", day);

    /* build map buffer */
    char* buf = (char*)malloc((size_t)(w*h));
    if(!buf){ fclose(f); return; }

    for(int y=0;y<h;y++){
        for(int x=0;x<w;x++){
            buf[y*w+x] = brz_world_tile_glyph(world, x, y);
        }
    }

    /* settlements */
    for(int si=0; si<sett_n; si++){
        int x=setts[si].pos.x, y=setts[si].pos.y;
        if(x>=0&&y>=0&&x<w&&y<h) buf[y*w+x] = 'S';
    }

    /* agents */
    for(int ai=0; ai<agent_n; ai++){
        int x=agents[ai].pos.x, y=agents[ai].pos.y;
        if(x<0||y<0||x>=w||y>=h) continue;
        char c='a';
        if(agents[ai].voc && agents[ai].voc->name && agents[ai].voc->name[0])
            c = agents[ai].voc->name[0];
        buf[y*w+x] = c;
    }

    for(int y=0;y<h;y++){
        for(int x=0;x<w;x++) fputc(buf[y*w+x], f);
        fputc('\n', f);
    }
    free(buf);
    fclose(f);
}

/* ---------------- reporting ---------------- */

static void print_day_summary(int day, const ParsedConfig* cfg,
                              const BrzSettlement* setts, int sett_n,
                              const BrzAgent* agents, int agent_n)
{
    const size_t res_n = kind_table_count(&cfg->resource_kinds);
    const size_t item_n = kind_table_count(&cfg->item_kinds);

    double* tot_res = (double*)calloc(res_n, sizeof(double));
    double* tot_item = (double*)calloc(item_n, sizeof(double));
    double avg_h=0, avg_f=0;

    for(int ai=0; ai<agent_n; ai++){
        avg_h += agents[ai].hunger;
        avg_f += agents[ai].fatigue;
        for(size_t i=0;i<res_n;i++) tot_res[i] += agents[ai].res_inv[i];
        for(size_t i=0;i<item_n;i++) tot_item[i] += agents[ai].item_inv[i];
    }
    if(agent_n>0){ avg_h/=agent_n; avg_f/=agent_n; }

    printf("Day %d | agents=%d settlements=%d | avg_hunger=%.3f avg_fatigue=%.3f\n",
           day, agent_n, sett_n, avg_h, avg_f);

    /* print a short top resources line */
    printf("  Resources:");
    for(size_t i=0;i<res_n;i++){
        const char* nm = kind_table_name(&cfg->resource_kinds, (int)i);
        if(!nm) nm="";
        if(i<6) printf(" %s=%.1f", nm, tot_res[i]);
    }
    if(res_n>6) printf(" ...");
    printf("\n");

    printf("  Items:");
    for(size_t i=0;i<item_n;i++){
        const char* nm = kind_table_name(&cfg->item_kinds, (int)i);
        if(!nm) nm="";
        if(i<6) printf(" %s=%.1f", nm, tot_item[i]);
    }
    if(item_n>6) printf(" ...");
    printf("\n");

    /* settlement food stores snapshot */
    if(sett_n>0){
        int grain = kind_table_find(&cfg->resource_kinds, "grain");
        int fish  = kind_table_find(&cfg->resource_kinds, "fish");
        for(int si=0; si<sett_n && si<3; si++){
            double g = (grain>=0)? setts[si].res_inv[grain] : 0;
            double fi= (fish>=0)? setts[si].res_inv[fish] : 0;
            printf("  %s at (%d,%d): grain=%.1f fish=%.1f\n", setts[si].name, setts[si].pos.x, setts[si].pos.y, g, fi);
        }
    }

    free(tot_res);
    free(tot_item);
}

/* ---------------- main runner ---------------- */

int brz_run_with_events(const ParsedConfig* cfg, const BronzeEventSink* events)
{
    if(!cfg) return 1;

    const size_t res_n  = kind_table_count(&cfg->resource_kinds);
    const size_t item_n = kind_table_count(&cfg->item_kinds);

    int days          = cfg_get_int(cfg, "sim_days", 365);
    int report_every  = cfg_get_int(cfg, "report_every", 30);
    int snapshot_every= cfg_get_int(cfg, "snapshot_every", 0);
    int map_every     = cfg_get_int(cfg, "map_every", 0);

    int map_w = cfg_get_int(cfg, "sim_map_w", 80);
    int map_h = cfg_get_int(cfg, "sim_map_h", 40);
    (void)cfg_get_str(cfg, "output_dir", "");

    int agent_n = (cfg->agent_count > 0) ? cfg->agent_count : (int)cfg->vocations.len;
    if(agent_n <= 0){ fprintf(stderr, "No agents (agents.count or vocations)\n"); return 1; }
    if(cfg->vocations.len == 0){ fprintf(stderr, "No vocations\n"); return 1; }

    int sett_n = (cfg->settlement_count > 0) ? cfg->settlement_count : 1;

    BrzWorld world;
    if(brz_world_init(&world, cfg, map_w, map_h, res_n) != 0){
        fprintf(stderr, "World init failed\n");
        return 1;
    }
    BronzeWorldPort world_port;
    bronze_world_port_init(&world_port,&world,res_n);
    if(!world_port.context){ brz_world_free(&world); return 1; }

    BrzSettlement* setts = NULL;
    if(brz_settlements_alloc(&setts, sett_n, res_n, item_n) != 0){
        fprintf(stderr, "Settlement alloc failed\n");
        bronze_world_port_destroy(&world_port);
        brz_world_free(&world);
        return 1;
    }
    brz_settlements_place(setts, sett_n, map_w, map_h, cfg->seed ? cfg->seed : 0xC0FFEEu);
    brz_world_stamp_fields_around_settlements(&world, setts, sett_n, 8);

    BrzAgent* agents = NULL;
    if(brz_agents_alloc_and_spawn(&agents, agent_n, cfg, setts, sett_n, res_n, item_n,
                                  cfg->seed ? cfg->seed : 0xC0FFEEu) != 0){
        fprintf(stderr, "Agent alloc failed\n");
        brz_settlements_free(setts, sett_n);
        bronze_world_port_destroy(&world_port);
        brz_world_free(&world);
        return 1;
    }

    /* simple population count */
    for(int si=0; si<sett_n; si++) setts[si].population = 0;
    for(int ai=0; ai<agent_n; ai++){
        int h = agents[ai].home_settlement;
        if(h>=0 && h<sett_n) setts[h].population++;
    }

    BrzRng rng;
    brz_rng_seed(&rng, cfg->seed ? cfg->seed : 0xC0FFEEu);

    for(int day=1; day<=days; day++)
    {
        world_port.step_regen(world_port.context);
        brz_settlements_begin_day(setts, sett_n);

        for(int i=0;i<agent_n;i++)
            brz_agent_step(&agents[i], cfg, &world, setts, sett_n, &rng, events, day);

        if(day==1 || (report_every>0 && day%report_every==0) || day==days)
            print_day_summary(day, cfg, setts, sett_n, agents, agent_n);

        if(snapshot_every > 0 && (day % snapshot_every)==0){
            char fn[128];
            snprintf(fn, sizeof(fn), "snapshot_day%05d.json", day);
            write_snapshot_json(cfg, &world, setts, sett_n, agents, agent_n, day, fn);
        }

        if(map_every > 0 && (day % map_every)==0){
            char fn[128];
            snprintf(fn, sizeof(fn), "map_day%05d.txt", day);
            dump_ascii_map(cfg, &world, setts, sett_n, agents, agent_n, day, fn, map_w, map_h);
        }
    }

    brz_agents_free(agents, agent_n);
    brz_settlements_free(setts, sett_n);
    bronze_world_port_destroy(&world_port);
    brz_world_free(&world);

    return 0;
}

int brz_run(const ParsedConfig* cfg)
{
    return brz_run_with_events(cfg, NULL);
}
