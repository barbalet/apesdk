#include "brz_kinds.h"
#include <stdlib.h>
#include <string.h>

static char* brz_strdup(const char* s)
{
    if(!s) return NULL;
    size_t n = strlen(s);
    char* p = (char*)malloc(n + 1);
    if(!p) return NULL;
    memcpy(p, s, n + 1);
    return p;
}

void kind_table_init(KindTable* kt)
{
    if(!kt) return;
    brz_vec_init(&kt->names, sizeof(char*));
}

void kind_table_destroy(KindTable* kt)
{
    if(!kt) return;
    for(size_t i=0;i<brz_vec_len(&kt->names);i++)
    {
        char** sp = (char**)brz_vec_at(&kt->names, i);
        if(sp && *sp) free(*sp);
    }
    brz_vec_destroy(&kt->names);
}

int kind_table_find(const KindTable* kt, const char* name)
{
    if(!kt || !name) return -1;
    for(size_t i=0;i<brz_vec_len(&kt->names);i++)
    {
        char* const* sp = (char* const*)brz_vec_cat(&kt->names, i);
        if(sp && *sp && strcmp(*sp, name)==0) return (int)i;
    }
    return -1;
}

int kind_table_add(KindTable* kt, const char* name)
{
    if(!kt || !name) return -1;
    int existing = kind_table_find(kt, name);
    if(existing >= 0) return existing;

    char* dup = brz_strdup(name);
    if(!dup) return -1;
    if(!brz_vec_push(&kt->names, &dup))
    {
        free(dup);
        return -1;
    }
    return (int)(brz_vec_len(&kt->names) - 1);
}

const char* kind_table_name(const KindTable* kt, int id)
{
    if(!kt) return "";
    if(id < 0) return "";
    size_t i = (size_t)id;
    if(i >= brz_vec_len(&kt->names)) return "";
    char* const* sp = (char* const*)brz_vec_cat(&kt->names, i);
    return (sp && *sp) ? *sp : "";
}
