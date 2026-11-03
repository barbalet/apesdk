#include "bronze_scenario.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void fixture(const char* content)
{
    FILE* file=fopen("bad.bronze","wb"); assert(file);
    fputs(content,file); assert(!fclose(file));
}
int main(int argc, char** argv)
{
    BronzeScenario* s=bronze_scenario_create();
    assert(argc==4 && s);
    assert(bronze_scenario_load(s,argv[1],"first_slice.bronze"));
    assert(bronze_scenario_vocations(s)==3);
    const BronzeProfessionPolicy* jobs=bronze_scenario_professions(s);
    assert(jobs && jobs->count==3 && !strcmp(jobs->vocations[0].name,"farmer"));
    assert(jobs->vocations[0].weight==3 && jobs->vocations[0].workplace==BRONZE_JOB_FIELD);
    uint32_t seed=bronze_scenario_seed(s);
    assert(!bronze_scenario_load(s,argv[3],"bronze_age_full.bronze"));
    assert(strstr(bronze_scenario_diagnostics(s),"ValidationError:136"));
    assert(!strcmp(bronze_scenario_name(s),"first_slice.bronze") && bronze_scenario_seed(s)==seed);
    assert(!bronze_scenario_load(s,"no-such-file","missing.bronze"));
    assert(strstr(bronze_scenario_diagnostics(s),"missing"));
    assert(!bronze_scenario_load(s,NULL,"missing-bundle.bronze"));
    assert(strstr(bronze_scenario_diagnostics(s),"missing-bundle"));
    fixture("@ malformed");
    assert(!bronze_scenario_load(s,"bad.bronze","bad.bronze"));
    assert(strstr(bronze_scenario_diagnostics(s),"LexError:1:1"));
    fixture("actions { action rest 0 0 } vocations { vocation worker { task work { when true { rest } ");
    assert(!bronze_scenario_load(s,"bad.bronze","syntax.bronze"));
    assert(strstr(bronze_scenario_diagnostics(s),"SyntaxError:1:"));
    {
        FILE* file=fopen("bad.bronze","wb"); assert(file);
        fputs("actions { action rest 0 0 } vocations { vocation worker { task work { ",file);
        for(int i=0;i<70;i++) fputs("when true { ",file);
        fputs("rest ",file);
        for(int i=0;i<70;i++) fputs("} ",file);
        fputs("} rule always { when true do work } } }",file);
        fclose(file);
        assert(!bronze_scenario_load(s,"bad.bronze","nested.bronze"));
        assert(strstr(bronze_scenario_diagnostics(s),"nesting exceeds"));
        file=fopen("bad.bronze","wb"); assert(file);
        assert(!fseek(file,2*1024*1024,SEEK_SET)); fputc('x',file); fclose(file);
        assert(!bronze_scenario_load(s,"bad.bronze","large.bronze"));
        assert(strstr(bronze_scenario_diagnostics(s),"2 MiB"));
    }
    fixture("agents { count 99999999999999999999999999999999 }");
    assert(!bronze_scenario_load(s,"bad.bronze","overflow.bronze"));
    assert(strstr(bronze_scenario_diagnostics(s),"expected an integer"));
    fixture("kinds { resources { grain } } resources { resource grain { habitat field capacity 5 renew 2 nutrition 1 market_target 5 } } actions { action rest 0 0 } vocations { vocation worker { task work { rest } rule always { when true do work } } }");
    assert(!bronze_scenario_load(s,"bad.bronze","bounds.bronze"));
    assert(strstr(bronze_scenario_diagnostics(s),"resource capacity"));
    fixture("scenario { version 1 profile service } actions { action rest 0 0 } vocations { vocation worker { task work { rest } rule always { when true do work } } }");
    assert(!bronze_scenario_load(s,"bad.bronze","unsupported.bronze"));
    assert(strstr(bronze_scenario_diagnostics(s),"service-profile"));
    assert(!strcmp(bronze_scenario_name(s),"first_slice.bronze"));
    fixture("actions { action rest 0 0 } vocations { vocation worker { task work { rest 3 } rule always { when true do work } } }");
    assert(!bronze_scenario_load(s,"bad.bronze","units.bronze"));
    assert(strstr(bronze_scenario_diagnostics(s),"argument count/units"));
    fixture("world { seed 42 field_slope 3 town_capacity 2 town_initial_grain 3 patch_initial_per_cell 4 } actions { action rest 0 0 } vocations { vocation worker { task work { rest } rule always { when true do work } } }");
    assert(bronze_scenario_load(s,"bad.bronze","seeded.bronze"));
    assert(bronze_scenario_seed(s)==42);
    assert(bronze_scenario_land_policy(s).town_capacity==2);
    assert(bronze_scenario_land_policy(s).town_initial_grain==3);
    assert(bronze_scenario_land_policy(s).patch_initial_per_cell==4);
    assert(bronze_scenario_land_policy(s).field_slope==3);
    fixture("world { unknown_option 42 } actions { action rest 0 0 } vocations { vocation worker { task work { rest } rule always { when true do work } } }");
    assert(!bronze_scenario_load(s,"bad.bronze","unknown.bronze"));
    assert(strstr(bronze_scenario_diagnostics(s),"unsupported or nonnumeric"));
    assert(bronze_scenario_seed(s)==42);
    assert(bronze_scenario_land_policy(s).field_slope==3);
    fixture("world { field_slope 3.5 } actions { action rest 0 0 } vocations { vocation worker { task work { rest } rule always { when true do work } } }");
    assert(!bronze_scenario_load(s,"bad.bronze","terrain.bronze"));
    assert(strstr(bronze_scenario_diagnostics(s),"world_field_slope"));
    assert(bronze_scenario_land_policy(s).field_slope==3);
    fixture("world { occupation_missing_weight 3 } actions { action rest 0 0 } vocations { vocation worker { task work { rest } rule always { when true do work } } }");
    assert(!bronze_scenario_load(s,"bad.bronze","occupation.bronze"));
    assert(strstr(bronze_scenario_diagnostics(s),"unknown occupation"));
    for(int i=0;i<12;i++) {
        assert(bronze_scenario_load(s,argv[2],"vertical_slice.bronze"));
        assert(bronze_scenario_vocations(s)==3 && !strcmp(bronze_scenario_name(s),"vertical_slice.bronze"));
        assert(bronze_scenario_load(s,argv[1],"first_slice.bronze"));
    }
    bronze_scenario_destroy(s); remove("bad.bronze");
    puts("Transactional scenario loading, diagnostics, bounds and replacement tests passed.");
    return 0;
}
