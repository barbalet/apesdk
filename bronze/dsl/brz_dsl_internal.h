/* Shared destruction routines for partially parsed AST nodes. Internal API. */
#ifndef BRZ_DSL_INTERNAL_H
#define BRZ_DSL_INTERNAL_H
#include "brz_dsl.h"
void brz_stmt_dispose(StmtDef* statement);
void brz_task_dispose(TaskDef* task);
void brz_rule_dispose(RuleDef* rule);
void brz_vocation_dispose(VocationDef* vocation);
#endif
