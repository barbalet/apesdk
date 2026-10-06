#ifndef BRZ_PARSER_H
#define BRZ_PARSER_H

#include <stdbool.h>
#include "brz_dsl.h"

/* Parse a .bronze file into ParsedConfig (must be init'd with brz_cfg_init()).
   Returns false on error (message printed to stderr). */
bool brz_parse_file(const char* path, ParsedConfig* out_cfg);

#endif /* BRZ_PARSER_H */
