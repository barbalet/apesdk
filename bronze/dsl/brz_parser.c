#include "brz_parser.h"
#include "brz_util.h"
#include "brz_vec.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------- lexer ---------- */

typedef enum {
    TK_EOF=0,
    TK_WORD,
    TK_NUM,
    TK_LBRACE,
    TK_RBRACE
} TokKind;

typedef struct {
    TokKind kind;
    char* text; /* for WORD/NUM */
    int line;
    int col;
} Token;

typedef struct {
    const char* src;
    size_t len;
    size_t pos;
    int line;
    int col;
    BrzVec toks; /* Token */
} Lexer;

static int peekc(Lexer* lx)
{
    if(lx->pos >= lx->len) return 0;
    return (unsigned char)lx->src[lx->pos];
}
static int getc_advance(Lexer* lx)
{
    if(lx->pos >= lx->len) return 0;
    int c = (unsigned char)lx->src[lx->pos++];
    if(c=='\n'){ lx->line++; lx->col=1; }
    else { lx->col++; }
    return c;
}
static char* dup_span(const char* s, size_t n)
{
    char* out = (char*)malloc(n+1);
    if(!out) return NULL;
    memcpy(out, s, n);
    out[n]=0;
    return out;
}

static bool push_tok(Lexer* lx, TokKind k, char* text, int line, int col)
{
    Token t;
    t.kind = k;
    t.text = text;
    t.line = line;
    t.col  = col;
    return brz_vec_push(&lx->toks, &t);
}

static void skip_ws_and_comments(Lexer* lx)
{
    for(;;)
    {
        int c = peekc(lx);
        /* whitespace */
        if(c==' '||c=='\t'||c=='\r'||c=='\n'){ getc_advance(lx); continue; }

        /* # line comment */
        if(c=='#'){
            while((c=getc_advance(lx))!=0 && c!='\n'){}
            continue;
        }

        /* // line comment */
        if(c=='/' && lx->pos+1<lx->len && lx->src[lx->pos+1]=='/'){
            getc_advance(lx); getc_advance(lx);
            while((c=getc_advance(lx))!=0 && c!='\n'){}
            continue;
        }

        /* block comment */
        if(c=='/' && lx->pos+1<lx->len && lx->src[lx->pos+1]=='*'){
            getc_advance(lx); getc_advance(lx);
            while((c=peekc(lx))!=0){
                if(c=='*' && lx->pos+1<lx->len && lx->src[lx->pos+1]=='/'){
                    getc_advance(lx); getc_advance(lx);
                    break;
                }
                getc_advance(lx);
            }
            continue;
        }
        break;
    }
}

static bool lex_all(Lexer* lx)
{
    lx->line = 1;
    lx->col = 1;
    brz_vec_init(&lx->toks, sizeof(Token));

    while(1)
    {
        skip_ws_and_comments(lx);
        int c = peekc(lx);
        int line = lx->line;
        int col  = lx->col;

        if(c==0){
            if(!push_tok(lx, TK_EOF, NULL, line, col)) return false;
            return true;
        }

        if(c=='{'){ getc_advance(lx); if(!push_tok(lx,TK_LBRACE,NULL,line,col)) return false; continue; }
        if(c=='}'){ getc_advance(lx); if(!push_tok(lx,TK_RBRACE,NULL,line,col)) return false; continue; }

        if(c==';' || c==':' || c==','){ getc_advance(lx); continue; }


        /* operators / punctuation used in conditions: > < >= <= == != ( ) */
        if(c=='>'||c=='<'||c=='='||c=='!'||c=='('||c==')')
        {
            size_t start = lx->pos;
            getc_advance(lx);
            if((c=='>'||c=='<'||c=='='||c=='!') && peekc(lx)=='=') getc_advance(lx);
            char* s = dup_span(lx->src + start, lx->pos - start);
            if(!s) return false;
            if(!push_tok(lx, TK_WORD, s, line, col)) return false;
            continue;
        }

        /* number: int or float (e.g., 12, 0.08, 10.0, 0.0005) */
        if(isdigit((unsigned char)c))
        {
            size_t start = lx->pos;
            while(isdigit((unsigned char)peekc(lx))) getc_advance(lx);
            if(peekc(lx)=='.'){
                getc_advance(lx);
                while(isdigit((unsigned char)peekc(lx))) getc_advance(lx);
            }
            char* s = dup_span(lx->src + start, lx->pos - start);
            if(!s) return false;
            if(!push_tok(lx, TK_NUM, s, line, col)) return false;
            continue;
        }

        /* word / identifier */
        if(isalpha((unsigned char)c) || c=='_')
        {
            size_t start = lx->pos;
            while(1)
            {
                int p = peekc(lx);
                if(isalnum((unsigned char)p) || p=='_' ) { getc_advance(lx); continue; }
                break;
            }
            char* s = dup_span(lx->src + start, lx->pos - start);
            if(!s) return false;
            if(!push_tok(lx, TK_WORD, s, line, col)) return false;
            continue;
        }

        fprintf(stderr, "LexError:%d:%d: Unexpected character '%c'\n", line, col, (char)c);
        return false;
    }
}

static void free_lexer(Lexer* lx)
{
    for(size_t i=0;i<lx->toks.len;i++){
        Token* t = (Token*)brz_vec_at(&lx->toks, i);
        free(t->text);
    }
    brz_vec_destroy(&lx->toks);
}

/* ---------- parser ---------- */

typedef struct {
    Token* toks;
    size_t count;
    size_t pos;
} Parser;

static Token* cur(Parser* p)
{
    if(p->pos >= p->count) return NULL;
    return &p->toks[p->pos];
}
static bool accept(Parser* p, TokKind k)
{
    Token* t = cur(p);
    if(t && t->kind==k){ p->pos++; return true; }
    return false;
}
static bool expect(Parser* p, TokKind k, const char* what)
{
    Token* t = cur(p);
    if(t && t->kind==k){ p->pos++; return true; }
    fprintf(stderr, "SyntaxError:%d:%d: Expected %s\n", t?t->line:0, t?t->col:0, what);
    return false;
}
static bool expect_word(Parser* p, const char** out)
{
    Token* t = cur(p);
    if(t && t->kind==TK_WORD){ *out = t->text; p->pos++; return true; }
    fprintf(stderr, "SyntaxError:%d:%d: Expected identifier\n", t?t->line:0, t?t->col:0);
    return false;
}
static bool accept_word(Parser* p, const char* w)
{
    Token* t = cur(p);
    if(t && t->kind==TK_WORD && brz_streq(t->text, w)){ p->pos++; return true; }
    return false;
}
static bool expect_num(Parser* p, double* out)
{
    Token* t = cur(p);
    if(t && t->kind==TK_NUM){ *out = strtod(t->text, NULL); p->pos++; return true; }
    fprintf(stderr, "SyntaxError:%d:%d: Expected number\n", t?t->line:0, t?t->col:0);
    return false;
}

/* join token texts from [start_pos, end_pos) into a single malloc'd string */
static char* join_tokens(Token* toks, size_t start, size_t end)
{
    size_t bytes=1;
    for(size_t i=start;i<end;i++){
        const char* s = toks[i].text ? toks[i].text : "";
        bytes += strlen(s) + 1;
    }
    char* out = (char*)malloc(bytes);
    if(!out) return NULL;
    out[0]=0;
    for(size_t i=start;i<end;i++){
        const char* s = toks[i].text ? toks[i].text : "";
        if(*s){
            strcat(out, s);
            if(i+1<end) strcat(out, " ");
        }
    }
    return out;
}

/* kinds { resources { a b c } items { x y z } } */
static bool parse_kinds(Parser* p, ParsedConfig* cfg)
{
    if(!expect(p, TK_LBRACE, "'{'")) return false;

    while(!accept(p, TK_RBRACE))
    {
        const char* section=NULL;
        if(!expect_word(p, &section)) return false;

        if(brz_streq(section, "resources"))
        {
            if(!expect(p, TK_LBRACE, "'{'")) return false;
            while(!accept(p, TK_RBRACE))
            {
                const char* name=NULL;
                if(!expect_word(p, &name)) return false;
                if(kind_table_add(&cfg->resource_kinds, name) < 0) {
                    fprintf(stderr, "Error: OOM adding resource kind '%s'\n", name);
                    return false;
                }
            }
        }
        else if(brz_streq(section, "items"))
        {
            if(!expect(p, TK_LBRACE, "'{'")) return false;
            while(!accept(p, TK_RBRACE))
            {
                const char* name=NULL;
                if(!expect_word(p, &name)) return false;
                if(kind_table_add(&cfg->item_kinds, name) < 0) {
                    fprintf(stderr, "Error: OOM adding item kind '%s'\n", name);
                    return false;
                }
            }
        }
        else if(brz_streq(section, "resource") || brz_streq(section, "item"))
        {
            /* example_large.bronze uses: kinds { resource; item; } -- accept/ignore */
            continue;
        }
        else
        {
            Token* t = cur(p);
            fprintf(stderr, "SyntaxError:%d:%d: Unknown kinds section '%s'\n", t?t->line:0, t?t->col:0, section);
            return false;
        }
    }
    return true;
}

/* world { seed 1337 years 30 } agents { count 220 } settlements { count 6 } */


/* resources can be either:
   1) regen params: resources { fish_renew 0.08 ... }
   2) kind mapping: resources { grain : resource; fish : resource; ... }
   We dispatch by looking at the token kind after the first identifier. */
static bool parse_resources_block(Parser* p, ParsedConfig* cfg)
{
    if(!expect(p, TK_LBRACE, "'{'")) return false;

    while(!accept(p, TK_RBRACE))
    {
        const char* name=NULL;
        if(!expect_word(p, &name)) return false;

        if(brz_streq(name, "resource"))
        {
            Token* start=cur(p);
            const char* resource_name=NULL;
            if(!expect_word(p,&resource_name) || !expect(p,TK_LBRACE,"'{'")) return false;
            ResourceDef rd; memset(&rd,0,sizeof(rd));
            rd.name=brz_strdup(resource_name); rd.capacity=10.0; rd.renew=0.01;
            rd.market_target=1.0; rd.line=start?start->line:0;
            if(!rd.name) return false;
            while(!accept(p,TK_RBRACE)){
                const char* key=NULL; Token* value=cur(p);
                if(!expect_word(p,&key) || !value) goto resource_free;
                if(brz_streq(key,"habitat")){
                    const char* habitat=NULL; if(!expect_word(p,&habitat)) goto resource_free;
                    free(rd.habitat); rd.habitat=brz_strdup(habitat); if(!rd.habitat) goto resource_free;
                } else {
                    double number=0.0; if(!expect_num(p,&number)) goto resource_free;
                    if(brz_streq(key,"capacity")) rd.capacity=number;
                    else if(brz_streq(key,"renew")) rd.renew=number;
                    else if(brz_streq(key,"nutrition")) rd.nutrition=number;
                    else if(brz_streq(key,"market_target")) rd.market_target=number;
                    else { fprintf(stderr,"SyntaxError:%d:%d: Unknown resource field '%s'\n",value->line,value->col,key); goto resource_free; }
                }
            }
            if(kind_table_add(&cfg->resource_kinds,resource_name)<0 || !brz_vec_push(&cfg->resources,&rd)) goto resource_free;
            continue;
resource_free:
            free(rd.name); free(rd.habitat); return false;
        }

        Token* t = cur(p);
        if(!t){ fprintf(stderr, "SyntaxError: Unexpected EOF\n"); return false; }

        if(t->kind==TK_NUM)
        {
            /* regen param form: name number */
            double v=0.0;
            if(!expect_num(p, &v)) return false;
            ParamDef pd; memset(&pd,0,sizeof(pd));
            pd.key = brz_strdup(name);
            if(!pd.key) return false;
            pd.value = v;
            if(!brz_vec_push(&cfg->params, &pd)) return false;
        }
        else if(t->kind==TK_WORD)
        {
            /* kind mapping form: name resource */
            const char* kind=NULL;
            if(!expect_word(p, &kind)) return false;
            (void)kind;
            if(kind_table_add(&cfg->resource_kinds, name) < 0){
                fprintf(stderr, "Error: OOM adding resource kind '%s'\n", name);
                return false;
            }
        }
        else
        {
            fprintf(stderr, "SyntaxError:%d:%d: Expected number or identifier\n", t->line, t->col);
            return false;
        }
    }
    return true;
}

/* recipes { recipe NAME { output KIND AMOUNT; input KIND AMOUNT; ... } } */
static bool parse_recipes_block(Parser* p, ParsedConfig* cfg)
{
    if(!expect(p,TK_LBRACE,"'{'")) return false;
    while(!accept(p,TK_RBRACE)){
        Token* first=cur(p); const char* keyword=NULL; const char* name=NULL;
        if(!expect_word(p,&keyword) || !brz_streq(keyword,"recipe") || !expect_word(p,&name) || !expect(p,TK_LBRACE,"'{'")) return false;
        RecipeDef r; memset(&r,0,sizeof(r)); r.name=brz_strdup(name); r.output_amount=1.0;
        r.line=first?first->line:0; brz_vec_init(&r.inputs,sizeof(RecipeInputDef));
        if(!r.name) return false;
        while(!accept(p,TK_RBRACE)){
            const char* key=NULL; const char* kind=NULL; double amount=0;
            if(!expect_word(p,&key) || !expect_word(p,&kind) || !expect_num(p,&amount)) goto recipe_fail;
            if(brz_streq(key,"output")){
                if(r.output) { fprintf(stderr,"SyntaxError:%d: duplicate recipe output\n",r.line); goto recipe_fail; }
                r.output=brz_strdup(kind); r.output_amount=amount; if(!r.output) goto recipe_fail;
            } else if(brz_streq(key,"input")) {
                RecipeInputDef in; in.kind=brz_strdup(kind); in.amount=amount;
                if(!in.kind || !brz_vec_push(&r.inputs,&in)){ free(in.kind); goto recipe_fail; }
            } else { fprintf(stderr,"SyntaxError:%d: expected recipe input or output\n",r.line); goto recipe_fail; }
        }
        if(!r.output || r.inputs.len==0 || !brz_vec_push(&cfg->recipes,&r)) goto recipe_fail;
        continue;
recipe_fail:
        free(r.name); free(r.output);
        for(size_t i=0;i<r.inputs.len;i++) free(((RecipeInputDef*)brz_vec_at(&r.inputs,i))->kind);
        brz_vec_destroy(&r.inputs); return false;
    }
    return true;
}

/* actions { action NAME MIN MAX } */
static bool parse_actions_block(Parser* p, ParsedConfig* cfg)
{
    if(!expect(p,TK_LBRACE,"'{'")) return false;
    while(!accept(p,TK_RBRACE)){
        Token* t=cur(p); const char* kw=NULL; const char* name=NULL; double min=0,max=0;
        if(!expect_word(p,&kw) || !brz_streq(kw,"action") || !expect_word(p,&name) || !expect_num(p,&min) || !expect_num(p,&max)) return false;
        ActionDef a; a.name=brz_strdup(name); a.min_args=(int)min; a.max_args=(int)max; a.line=t?t->line:0;
        if(!a.name || !brz_vec_push(&cfg->actions,&a)){ free(a.name); return false; }
    }
    return true;
}

/* settlement_policy { food_fallback grain deposit_threshold 2 rest_recovery .04 } */
static bool parse_settlement_policy(Parser* p, ParsedConfig* cfg)
{
    if(!expect(p,TK_LBRACE,"'{'")) return false;
    while(!accept(p,TK_RBRACE)){
        const char* key=NULL; if(!expect_word(p,&key)) return false;
        if(brz_streq(key,"food_fallback")){
            const char* val=NULL; if(!expect_word(p,&val)) return false;
            free(cfg->settlement_policy.food_fallback); cfg->settlement_policy.food_fallback=brz_strdup(val);
            if(!cfg->settlement_policy.food_fallback) return false;
        } else {
            double val=0; if(!expect_num(p,&val)) return false;
            if(brz_streq(key,"deposit_threshold")) cfg->settlement_policy.deposit_threshold=val;
            else if(brz_streq(key,"rest_recovery")) cfg->settlement_policy.rest_recovery=val;
            else return false;
        }
    }
    return true;
}

/* Generic scenario declarations.  These compile content identifiers and leave
   map geometry and runtime adapters outside the DSL. */
static bool parse_scenario_block(Parser* p, ParsedConfig* cfg)
{
    if(!expect(p,TK_LBRACE,"'{'")) return false;
    while(!accept(p,TK_RBRACE)){
        const char* key=NULL;
        if(!expect_word(p,&key)) return false;
        if(brz_streq(key,"version")){ double v=0; if(!expect_num(p,&v)) return false; cfg->language_version=(int)v; }
        else if(brz_streq(key,"profile")){ const char* value=NULL; if(!expect_word(p,&value)) return false; free(cfg->scenario_profile); cfg->scenario_profile=brz_strdup(value); if(!cfg->scenario_profile) return false; }
        else return false;
    }
    return true;
}

static bool parse_map_refs_block(Parser* p, ParsedConfig* cfg)
{
    if(!expect(p,TK_LBRACE,"'{'")) return false;
    while(!accept(p,TK_RBRACE)){
        Token* start=cur(p); const char* keyword=NULL; const char* id=NULL;
        MapRefDef def; memset(&def,0,sizeof(def));
        if(!expect_word(p,&keyword) || !brz_streq(keyword,"map") || !expect_word(p,&id)) return false;
        def.id=brz_strdup(id); def.line=start?start->line:0;
        if(!def.id || !brz_vec_push(&cfg->map_refs,&def)){ free(def.id); return false; }
    }
    return true;
}

static bool parse_places_block(Parser* p, ParsedConfig* cfg)
{
    if(!expect(p,TK_LBRACE,"'{'")) return false;
    while(!accept(p,TK_RBRACE)){
        Token* start=cur(p); const char* keyword=NULL; const char* name=NULL; PlaceDef def;
        memset(&def,0,sizeof(def)); def.capacity=0; def.line=start?start->line:0;
        if(!expect_word(p,&keyword) || !brz_streq(keyword,"place") || !expect_word(p,&name) || !expect(p,TK_LBRACE,"'{'")) return false;
        def.name=brz_strdup(name); if(!def.name) return false;
        while(!accept(p,TK_RBRACE)){
            const char* key=NULL; if(!expect_word(p,&key)) goto fail;
            if(brz_streq(key,"kind") || brz_streq(key,"map_ref")){ const char* value=NULL; char** dst=brz_streq(key,"kind") ? &def.kind : &def.map_ref; if(!expect_word(p,&value)) goto fail; free(*dst); *dst=brz_strdup(value); if(!*dst) goto fail; }
            else if(brz_streq(key,"capacity")){ if(!expect_num(p,&def.capacity)) goto fail; }
            else goto fail;
        }
        if(!def.kind || !brz_vec_push(&cfg->places,&def)) goto fail;
        continue;
fail: free(def.name); free(def.kind); free(def.map_ref); return false;
    }
    return true;
}

static bool parse_roles_block(Parser* p, ParsedConfig* cfg)
{
    if(!expect(p,TK_LBRACE,"'{'")) return false;
    while(!accept(p,TK_RBRACE)){
        Token* start=cur(p); const char* keyword=NULL; const char* name=NULL; RoleDef def;
        memset(&def,0,sizeof(def)); def.line=start?start->line:0;
        if(!expect_word(p,&keyword) || !brz_streq(keyword,"role") || !expect_word(p,&name) || !expect(p,TK_LBRACE,"'{'")) return false;
        def.name=brz_strdup(name); if(!def.name) return false;
        while(!accept(p,TK_RBRACE)){ const char* key=NULL; if(!expect_word(p,&key)) goto fail; if(brz_streq(key,"workplace")){ const char* value=NULL; if(!expect_word(p,&value)) goto fail; free(def.workplace); def.workplace=brz_strdup(value); if(!def.workplace) goto fail; } else if(brz_streq(key,"min_age")){ if(!expect_num(p,&def.min_age)) goto fail; } else goto fail; }
        if(!brz_vec_push(&cfg->roles,&def)) goto fail;
        continue;
fail: free(def.name); free(def.workplace); return false;
    }
    return true;
}

static bool parse_needs_block(Parser* p, ParsedConfig* cfg)
{
    if(!expect(p,TK_LBRACE,"'{'")) return false;
    while(!accept(p,TK_RBRACE)){
        Token* start=cur(p); const char* keyword=NULL; const char* name=NULL; const char* priority=NULL; NeedDef def;
        memset(&def,0,sizeof(def)); def.line=start?start->line:0;
        if(!expect_word(p,&keyword) || !brz_streq(keyword,"need") || !expect_word(p,&name) || !expect(p,TK_LBRACE,"'{'")) return false;
        if(!expect_word(p,&priority) || !brz_streq(priority,"priority") || !expect_num(p,&def.priority) || !expect(p,TK_RBRACE,"'}'")) return false;
        def.name=brz_strdup(name); if(!def.name || !brz_vec_push(&cfg->needs,&def)){ free(def.name); return false; }
    }
    return true;
}

static bool parse_services_block(Parser* p, ParsedConfig* cfg)
{
    if(!expect(p,TK_LBRACE,"'{'")) return false;
    while(!accept(p,TK_RBRACE)){
        Token* start=cur(p); const char* keyword=NULL; const char* name=NULL; ServiceDef def;
        memset(&def,0,sizeof(def)); def.line=start?start->line:0;
        if(!expect_word(p,&keyword) || !brz_streq(keyword,"service") || !expect_word(p,&name) || !expect(p,TK_LBRACE,"'{'")) return false;
        def.name=brz_strdup(name); if(!def.name) return false;
        while(!accept(p,TK_RBRACE)){
            const char* key=NULL; const char* value=NULL; char** dst=NULL;
            if(!expect_word(p,&key)) goto fail;
            if(brz_streq(key,"price")){ if(!expect_num(p,&def.price)) goto fail; continue; }
            if(!expect_word(p,&value)) goto fail;
            if(brz_streq(key,"provider")) dst=&def.provider_role; else if(brz_streq(key,"recipient")) dst=&def.recipient_role; else if(brz_streq(key,"place")) dst=&def.place; else if(brz_streq(key,"input")) dst=&def.input; else if(brz_streq(key,"output")) dst=&def.output; else goto fail;
            free(*dst); *dst=brz_strdup(value); if(!*dst) goto fail;
        }
        if(!def.provider_role || !def.place || !brz_vec_push(&cfg->services,&def)) goto fail;
        continue;
fail: free(def.name); free(def.provider_role); free(def.recipient_role); free(def.place); free(def.input); free(def.output); return false;
    }
    return true;
}

static bool parse_disruptions_block(Parser* p, ParsedConfig* cfg)
{
    if(!expect(p,TK_LBRACE,"'{'")) return false;
    while(!accept(p,TK_RBRACE)){
        Token* start=cur(p); const char* keyword=NULL; const char* name=NULL; const char* key=NULL; const char* affects=NULL; DisruptionDef def;
        memset(&def,0,sizeof(def)); def.line=start?start->line:0;
        if(!expect_word(p,&keyword) || !brz_streq(keyword,"disruption") || !expect_word(p,&name) || !expect(p,TK_LBRACE,"'{'")) return false;
        if(!expect_word(p,&key) || !brz_streq(key,"affects") || !expect_word(p,&affects) || !expect(p,TK_RBRACE,"'}'")) return false;
        def.name=brz_strdup(name); def.affects=brz_strdup(affects);
        if(!def.name || !def.affects || !brz_vec_push(&cfg->disruptions,&def)){ free(def.name); free(def.affects); return false; }
    }
    return true;
}

static bool parse_items_block(Parser* p, ParsedConfig* cfg)
{
    if(!expect(p, TK_LBRACE, "'{'")) return false;

    while(!accept(p, TK_RBRACE))
    {
        const char* name=NULL;
        if(!expect_word(p, &name)) return false;

        Token* t = cur(p);
        if(!t){ fprintf(stderr, "SyntaxError: Unexpected EOF\n"); return false; }

        if(t->kind==TK_WORD)
        {
            /* kind mapping form: name item */
            const char* kind=NULL;
            if(!expect_word(p, &kind)) return false;
            (void)kind;
            if(kind_table_add(&cfg->item_kinds, name) < 0){
                fprintf(stderr, "Error: OOM adding item kind '%s'\n", name);
                return false;
            }
        }
        else
        {
            fprintf(stderr, "SyntaxError:%d:%d: Expected identifier\n", t->line, t->col);
            return false;
        }
    }
    return true;
}

static bool parse_simple_kv_block(Parser* p, const char* block_name, ParsedConfig* cfg)
{
    if(!expect(p, TK_LBRACE, "'{'")) return false;

    while(!accept(p, TK_RBRACE))
    {
        const char* key=NULL;
        if(!expect_word(p, &key)) return false;

        Token* vtok = cur(p);
        if(!vtok){ fprintf(stderr, "SyntaxError: Unexpected EOF\n"); return false; }

        bool is_num = (vtok->kind==TK_NUM);
        bool is_word = (vtok->kind==TK_WORD);

        double num = 0.0;
        const char* sval = NULL;

        if(is_num)
        {
            if(!expect_num(p, &num)) return false;
        }
        else if(is_word)
        {
            if(!expect_word(p, &sval)) return false;
        }
        else
        {
            fprintf(stderr, "SyntaxError:%d:%d: Expected number or word\n", vtok->line, vtok->col);
            return false;
        }

        /* typed fields we care about */
        if(brz_streq(block_name, "world") && is_num)
        {
            if(brz_streq(key, "seed")) cfg->seed = (uint32_t)num;
            else if(brz_streq(key, "years")) cfg->years = (int)num;
        }
        else if(brz_streq(block_name, "agents") && is_num)
        {
            if(brz_streq(key, "count")) cfg->agent_count = (int)num;
        }
        else if(brz_streq(block_name, "settlements") && is_num)
        {
            if(brz_streq(key, "count")) cfg->settlement_count = (int)num;
        }

        /* store everything as a param as well (helps debugging / future sim) */
        ParamDef pd;
        memset(&pd, 0, sizeof(pd));

        /* prefix sim_ (and world_/agents_/settlements_) to avoid collisions */
        const char* prefix = NULL;
        if(brz_streq(block_name, "sim")) prefix = "sim_";
        else if(brz_streq(block_name, "world")) prefix = "world_";
        else if(brz_streq(block_name, "agents")) prefix = "agents_";
        else if(brz_streq(block_name, "settlements")) prefix = "settlements_";
        else if(brz_streq(block_name, "resources")) prefix = ""; /* already descriptive */
        else prefix = "";

        if(prefix && prefix[0])
        {
            size_t n1 = strlen(prefix);
            size_t n2 = strlen(key);
            pd.key = (char*)malloc(n1 + n2 + 1);
            if(!pd.key) return false;
            memcpy(pd.key, prefix, n1);
            memcpy(pd.key + n1, key, n2 + 1);
        }
        else
        {
            pd.key = brz_strdup(key);
            if(!pd.key) return false;
        }

        if(is_num)
        {
            pd.value = num;
        }
        else
        {
            pd.has_svalue = true;
            pd.svalue = brz_strdup(sval);
            if(!pd.svalue) return false;
        }

        if(!brz_vec_push(&cfg->params, &pd)) return false;
    }
    return true;
}
/* parse an op line inside a task: <word> [arg] [arg] [arg] [num] ... until line changes */


/* ---------- statements inside tasks ---------- */

static bool parse_stmt_list(Parser* p, BrzVec* out_stmts);

static bool parse_op_only(Parser* p, OpDef* out_op)
{
    Token* t = cur(p);
    if(!t) return false;
    if(t->kind!=TK_WORD)
    {
        fprintf(stderr, "SyntaxError:%d:%d: Expected operation\n", t->line, t->col);
        return false;
    }

    int line = t->line;
    const char* opw = t->text;
    p->pos++;

    OpDef op;
    memset(&op, 0, sizeof(op));
    op.op = brz_strdup(opw);
    op.line = line;
    if(!op.op) return false;

    /* collect up to 3 word args and 1 numeric on the same line;
       stop on brace (block start) or end-of-line. */
    while(1)
    {
        Token* n = cur(p);
        if(!n) break;
        if(n->kind==TK_LBRACE || n->kind==TK_RBRACE) break;
        if(n->line != line) break;

        if(n->kind==TK_WORD)
        {
            if(!op.a0) op.a0 = brz_strdup(n->text);
            else if(!op.a1) op.a1 = brz_strdup(n->text);
            else if(!op.a2) op.a2 = brz_strdup(n->text);
            p->pos++;
            continue;
        }
        if(n->kind==TK_NUM)
        {
            op.n0 = strtod(n->text, NULL);
            op.has_n0 = true;
            p->pos++;
            continue;
        }

        break;
    }

    *out_op = op;
    return true;
}

static char* collect_expr_until_lbrace(Parser* p)
{
    /* Collect WORD/NUM tokens into a single space-joined string until '{' */
    size_t cap = 128;
    size_t len = 0;
    char* buf = (char*)malloc(cap);
    if(!buf) return NULL;
    buf[0] = 0;

    while(1)
    {
        Token* n = cur(p);
        if(!n){ free(buf); return NULL; }
        if(n->kind==TK_LBRACE) break;

        const char* s = (n->text ? n->text : "");
        size_t sl = strlen(s);
        /* +1 for optional space, +1 for terminator */
        size_t need = len + (len?1:0) + sl + 1;
        if(need > cap)
        {
            while(cap < need) cap *= 2;
            char* nb = (char*)realloc(buf, cap);
            if(!nb){ free(buf); return NULL; }
            buf = nb;
        }
        if(len)
        {
            buf[len++] = ' ';
        }
        memcpy(buf + len, s, sl);
        len += sl;
        buf[len] = 0;

        p->pos++;
    }
    return buf;
}

/* statement := chance NUM { stmt* } | when <expr...> { stmt* } | op_line */
static bool parse_stmt(Parser* p, StmtDef* out_stmt)
{
    Token* t = cur(p);
    if(!t){ fprintf(stderr, "SyntaxError: Unexpected EOF\n"); return false; }

    if(t->kind==TK_WORD && t->text && brz_streq(t->text, "chance"))
    {
        int line = t->line;
        p->pos++;

        double pct = 0.0;
        if(!expect_num(p, &pct)) return false;
        if(!expect(p, TK_LBRACE, "'{'")) return false;

        StmtDef st;
        memset(&st, 0, sizeof(st));
        st.kind = ST_CHANCE;
        st.line = line;
        st.as.chance.chance_pct = pct;
        brz_vec_init(&st.as.chance.body, sizeof(StmtDef));

        if(!parse_stmt_list(p, &st.as.chance.body)) return false;

        *out_stmt = st;
        return true;
    }

    if(t->kind==TK_WORD && t->text && brz_streq(t->text, "when"))
    {
        int line = t->line;
        p->pos++;

        char* expr = collect_expr_until_lbrace(p);
        if(!expr){ fprintf(stderr, "Error: OOM collecting when expr\n"); return false; }

        if(!expect(p, TK_LBRACE, "'{'")){ free(expr); return false; }

        StmtDef st;
        memset(&st, 0, sizeof(st));
        st.kind = ST_WHEN;
        st.line = line;
        st.as.when_stmt.when_expr = expr;
        brz_vec_init(&st.as.when_stmt.body, sizeof(StmtDef));

        if(!parse_stmt_list(p, &st.as.when_stmt.body)) return false;

        *out_stmt = st;
        return true;
    }

    /* op line */
    OpDef op;
    if(!parse_op_only(p, &op)) return false;
    StmtDef st;
    memset(&st, 0, sizeof(st));
    st.kind = ST_OP;
    st.line = op.line;
    st.as.op = op;
    *out_stmt = st;
    return true;
}

/* Parses statements until matching '}' (consumes the '}' as terminator) */
static bool parse_stmt_list(Parser* p, BrzVec* out_stmts)
{
    while(1)
    {
        Token* t = cur(p);
        if(!t){ fprintf(stderr, "SyntaxError: Unexpected EOF in block\n"); return false; }

        if(t->kind==TK_RBRACE)
        {
            p->pos++; /* consume */
            return true;
        }

        StmtDef st;
        memset(&st, 0, sizeof(st));
        if(!parse_stmt(p, &st)) return false;
        if(!brz_vec_push(out_stmts, &st)) return false;
    }
}

/* task NAME { ... } */
static bool parse_task(Parser* p, VocationDef* voc)
{
    const char* name=NULL;
    if(!expect_word(p, &name)) return false;

    TaskDef t;
    memset(&t, 0, sizeof(t));
    t.name = brz_strdup(name);
    if(!t.name) return false;
    brz_vec_init(&t.stmts, sizeof(StmtDef));

    if(!expect(p, TK_LBRACE, "'{'")) return false;
    if(!parse_stmt_list(p, &t.stmts)) return false;

    if(!brz_vec_push(&voc->tasks, &t)) return false;
    return true;
}


/* rule NAME { when <...> do TASK weight N } */

static bool parse_rule(Parser* p, VocationDef* voc)
{
    Token* start=cur(p);
    const char* name=NULL;
    if(!expect_word(p, &name)) return false;

    if(!expect(p, TK_LBRACE, "'{'")) return false;

    int depth = 0; /* nested braces inside rule */
    char* when_expr = NULL;
    char* do_task = NULL;
    int weight = 1;

    while(1)
    {
        Token* t = cur(p);
        if(!t){ fprintf(stderr, "SyntaxError: Unexpected EOF in rule\n"); return false; }

        if(t->kind==TK_LBRACE)
        {
            depth++;
            p->pos++;
            continue;
        }

        if(t->kind==TK_RBRACE)
        {
            if(depth > 0)
            {
                depth--;
                p->pos++;
                continue;
            }
            /* end rule */
            p->pos++;
            break;
        }

        if(t->kind==TK_WORD && brz_streq(t->text, "when"))
        {
            p->pos++; /* consume when */
            size_t start = p->pos;
            /* collect until we hit '{' at same depth (most common) or 'do' keyword */
            while(1)
            {
                Token* u = cur(p);
                if(!u){ fprintf(stderr, "SyntaxError: Unexpected EOF in when\n"); return false; }
                if(u->kind==TK_LBRACE || u->kind==TK_RBRACE) break;
                if(u->kind==TK_WORD && brz_streq(u->text, "do")) break;
                p->pos++;
            }
            free(when_expr);
            when_expr = join_tokens(p->toks, start, p->pos);
            if(!when_expr) return false;
            continue;
        }

        if(t->kind==TK_WORD && brz_streq(t->text, "do"))
        {
            p->pos++; /* consume do */
            const char* task=NULL;
            if(cur(p) && cur(p)->kind==TK_WORD)
            {
                if(!expect_word(p, &task)) return false;
                if(!do_task) { do_task = brz_strdup(task); if(!do_task) return false; }
            }
            continue;
        }

        if(t->kind==TK_WORD && brz_streq(t->text, "weight"))
        {
            p->pos++;
            if(cur(p) && cur(p)->kind==TK_NUM)
            {
                double n=0; if(!expect_num(p, &n)) return false;
                weight = (int)n;
            }
            continue;
        }

        /* otherwise skip */
        p->pos++;
    }

    RuleDef r;
    memset(&r, 0, sizeof(r));
    r.name = brz_strdup(name);
    r.when_expr = when_expr ? when_expr : brz_strdup("true");
    r.do_task = do_task ? do_task : brz_strdup("");
    r.weight = weight;
    r.line = start ? start->line : 0;
    if(!r.name || !r.when_expr || !r.do_task) return false;

    if(!brz_vec_push(&voc->rules, &r)) return false;
    return true;
}


/* vocation NAME { ... } */
static bool parse_vocation(Parser* p, ParsedConfig* cfg)
{
    (void)cfg;
    const char* name=NULL;
    if(!expect_word(p, &name)) return false;

    VocationDef v;
    memset(&v, 0, sizeof(v));
    v.name = brz_strdup(name);
    if(!v.name) return false;
    brz_vec_init(&v.tasks, sizeof(TaskDef));
    brz_vec_init(&v.rules, sizeof(RuleDef));

    if(!expect(p, TK_LBRACE, "'{'")) return false;

    while(!accept(p, TK_RBRACE))
    {
        if(accept_word(p, "task"))
        {
            if(!parse_task(p, &v)) return false;
            continue;
        }
        if(accept_word(p, "rule"))
        {
            if(!parse_rule(p, &v)) return false;
            continue;
        }

        Token* t = cur(p);
        fprintf(stderr, "SyntaxError:%d:%d: Expected 'task' or 'rule' in vocation\n", t?t->line:0, t?t->col:0);
        return false;
    }

    if(!brz_vec_push(&cfg->vocations, &v)) return false;
    return true;
}

/* vocations { vocation X { ... } ... } */
static bool parse_vocations(Parser* p, ParsedConfig* cfg)
{
    if(!expect(p, TK_LBRACE, "'{'")) return false;
    while(!accept(p, TK_RBRACE))
    {
        if(!accept_word(p, "vocation"))
        {
            Token* t = cur(p);
            fprintf(stderr, "SyntaxError:%d:%d: Expected 'vocation'\n", t?t->line:0, t?t->col:0);
            return false;
        }
        if(!parse_vocation(p, cfg)) return false;
    }
    return true;
}

/* ---------- public API ---------- */

bool brz_parse_file(const char* path, ParsedConfig* out_cfg)
{
    size_t n=0;
    char* src = brz_read_entire_file(path, &n);
    if(!src)
    {
        fprintf(stderr, "Error: failed to read '%s'\n", path);
        return false;
    }

    Lexer lx;
    memset(&lx, 0, sizeof(lx));
    lx.src = src;
    lx.len = n;
    lx.pos = 0;

    if(!lex_all(&lx))
    {
        free(src);
        free_lexer(&lx);
        return false;
    }

    Parser p;
    p.toks = (Token*)lx.toks.data;
    p.count = lx.toks.len;
    p.pos = 0;

    while(1)
    {
        Token* t = cur(&p);
        if(!t) break;
        if(t->kind==TK_EOF) break;

        const char* top=NULL;
        if(!expect_word(&p, &top)){ free(src); free_lexer(&lx); return false; }

        if(brz_streq(top, "kinds"))
        {
            if(!parse_kinds(&p, out_cfg)){ free(src); free_lexer(&lx); return false; }
        }
        else if(brz_streq(top,"scenario"))
        {
            if(!parse_scenario_block(&p,out_cfg)){ free(src); free_lexer(&lx); return false; }
        }
        else if(brz_streq(top,"map_refs"))
        {
            if(!parse_map_refs_block(&p,out_cfg)){ free(src); free_lexer(&lx); return false; }
        }
        else if(brz_streq(top,"places"))
        {
            if(!parse_places_block(&p,out_cfg)){ free(src); free_lexer(&lx); return false; }
        }
        else if(brz_streq(top,"roles"))
        {
            if(!parse_roles_block(&p,out_cfg)){ free(src); free_lexer(&lx); return false; }
        }
        else if(brz_streq(top,"needs"))
        {
            if(!parse_needs_block(&p,out_cfg)){ free(src); free_lexer(&lx); return false; }
        }
        else if(brz_streq(top,"services"))
        {
            if(!parse_services_block(&p,out_cfg)){ free(src); free_lexer(&lx); return false; }
        }
        else if(brz_streq(top,"disruptions"))
        {
            if(!parse_disruptions_block(&p,out_cfg)){ free(src); free_lexer(&lx); return false; }
        }
        else if(brz_streq(top, "world"))
        {
            if(!parse_simple_kv_block(&p, "world", out_cfg)){ free(src); free_lexer(&lx); return false; }
        }
        else if(brz_streq(top, "sim"))
        {
            if(!parse_simple_kv_block(&p, "sim", out_cfg)){ free(src); free_lexer(&lx); return false; }
        }
        else if(brz_streq(top, "agents"))
        {
            if(!parse_simple_kv_block(&p, "agents", out_cfg)){ free(src); free_lexer(&lx); return false; }
        }
        else if(brz_streq(top, "settlements"))
        {
            if(!parse_simple_kv_block(&p, "settlements", out_cfg)){ free(src); free_lexer(&lx); return false; }
        }
        else if(brz_streq(top, "resources"))
        {
            if(!parse_resources_block(&p, out_cfg)){ free(src); free_lexer(&lx); return false; }
        }
        else if(brz_streq(top, "items"))
        {
            if(!parse_items_block(&p, out_cfg)){ free(src); free_lexer(&lx); return false; }
        }
        else if(brz_streq(top, "vocations"))
        {
            if(!parse_vocations(&p, out_cfg)){ free(src); free_lexer(&lx); return false; }
        }
        else if(brz_streq(top, "recipes"))
        {
            if(!parse_recipes_block(&p, out_cfg)){ free(src); free_lexer(&lx); return false; }
        }
        else if(brz_streq(top, "actions"))
        {
            if(!parse_actions_block(&p, out_cfg)){ free(src); free_lexer(&lx); return false; }
        }
        else if(brz_streq(top, "settlement_policy"))
        {
            if(!parse_settlement_policy(&p, out_cfg)){ free(src); free_lexer(&lx); return false; }
        }
        else
        {
            fprintf(stderr, "SyntaxError:%d:%d: Unknown top-level section '%s'\n", t->line, t->col, top);
            free(src);
            free_lexer(&lx);
            return false;
        }
    }

    if(!brz_cfg_validate(out_cfg, stderr) || !brz_cfg_compile(out_cfg, stderr)){ free(src); free_lexer(&lx); return false; }
    free(src);
    free_lexer(&lx);
    return true;
}
