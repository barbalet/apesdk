#ifndef BRZ_PARSER_H
#define BRZ_PARSER_H

#include <stdbool.h>
#include "brz_dsl.h"

/* Parse a .bronze file into ParsedConfig (must be init'd with brz_cfg_init()).
   Returns false on error (message printed to stderr). */
/* Strict entry point validates and compiles, rejecting unsupported semantics. */
bool brz_parse_file(const char* path, ParsedConfig* out_cfg);
/* Instance-owned diagnostics: never redirects process stderr. */
bool brz_parse_file_report(const char* path, ParsedConfig* out_cfg, FILE* errors);

/* Same parser and AST, but preserve the old standalone interpreter's defaults
   and accepted surface semantics. No compiled execution is permitted. */
bool brz_parse_file_legacy_runtime(const char* path, ParsedConfig* out_cfg);

#endif /* BRZ_PARSER_H */
