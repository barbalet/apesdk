#include "brz_parser.h"
#include "brz_sim.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    ParsedConfig config;
    const char *scenario=(argc>1) ? argv[1] : "content/bronze_age_full.bronze";
    brz_cfg_init(&config);
    if(!brz_parse_file(scenario,&config)){
        brz_cfg_free(&config);
        return 1;
    }
    printf("Bronze package: %zu vocations, %zu resources, %zu recipes\n",
           config.vocations.len,config.resources.len,config.recipes.len);
    int status=brz_run(&config);
    brz_cfg_free(&config);
    return status;
}
