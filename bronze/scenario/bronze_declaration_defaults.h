#ifndef BRONZE_DECLARATION_DEFAULTS_H
#define BRONZE_DECLARATION_DEFAULTS_H

/* Older declaration-only documents list kinds without ecological or recipe
 * definitions. Supply the native host's documented baseline for that format.
 * Explicit resource/recipe documents never enter this path. */
static int declaration_defaults(ParsedConfig* config)
{
    if (config->resources.len || config->recipes.len || config->actions.len) return 1;
    for (size_t i = 0; i < kind_table_count(&config->resource_kinds); ++i) {
        const char* name = kind_table_name(&config->resource_kinds, (int)i);
        const char* habitat = "field";
        if (!strcmp(name, "wood") || !strcmp(name, "charcoal")) habitat = "forest";
        if (!strcmp(name, "copper")) habitat = "mine_copper";
        if (!strcmp(name, "tin")) habitat = "mine_tin";
        ResourceDef resource = {0};
        resource.name = brz_strdup(name);
        resource.habitat = brz_strdup(habitat);
        resource.capacity = 10;
        resource.renew = 0.01;
        resource.market_target = 1;
        resource.nutrition = !strcmp(name, "grain") || !strcmp(name, "fish") ||
                             !strcmp(name, "cattle") || !strcmp(name, "sheep") ||
                             !strcmp(name, "pig") || !strcmp(name, "goat") ? 0.20 : 0;
        if (!resource.name || !resource.habitat || !brz_vec_push(&config->resources, &resource)) {
            free(resource.name); free(resource.habitat); return 0;
        }
    }
    const ActionDef defaults[] = {
        {.name="gather", .min_args=1, .max_args=2},
        {.name="craft", .min_args=1, .max_args=2}
    };
    for (size_t i = 0; i < sizeof(defaults)/sizeof(defaults[0]); ++i) {
        ActionDef action = defaults[i];
        action.name = brz_strdup(action.name);
        if (!action.name || !brz_vec_push(&config->actions, &action)) { free(action.name); return 0; }
    }
    return 1;
}
#endif
