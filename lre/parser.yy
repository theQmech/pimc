
%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.0.2"
%defines
%define parser_class_name {lre_parser}

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires
{
#include <string>
#include "symbols.h"
#include "symtab.h"
class lre_driver;
}

// The parsing context.
%param { lre_driver& driver }

%locations
%initial-action
{
  // Initialize the initial location.
  @$.begin.filename = @$.end.filename = &driver.file;
};

%define parse.trace
%define parse.error verbose

%code
{
#include "driver.hh"
}

%define api.token.prefix {TOK_}
%token
  END  0        "end of file"
  LPAREN        "("
  RPAREN        ")"
  COMMA         ","
  USEBDD        "USEBDD"
  USECLAUSES    "USECLAUSES"
  PRIME         "PRIME"
  RETURN        "RETURN"
  REGION        "REGION"
  CUBE          "CUBE"
  INDEX         "INDEX"
  IF            "IF"
  ELSE          "ELSE"
  WHILE         "WHILE"
  CLPAREN       "{"
  CRPAREN       "}"
  SLPAREN       "["
  SRPAREN       "]"
  ASSIGN        "="
  CONJUNCT      "^="
  INC           "INC"
  DEC           "DEC"
  OR            "||"
  AND           "AND"
  NEGATION      "NEGATION"
  NOT           "NOT"
  lt            "lt"
  le            "le"
  gt            "gt"
  ge            "ge"
  eq            "eq"
  ne            "ne"
  SAT           "SAT"
  SMP           "SMP"
  CNFARR        "CNF_ARR"
  CLSARR        "CLS_ARR"
  BREAK         "BREAK"
  SEPARATOR     "%%"
;

%token <std::string> IDENTIFIER "identifier"
%token <int> NUMBER "number"
%type <opd> label_rev num_opd;
%type <ast_node *> body breakpoint while_block if_block exp bool;
%type <type> type_ds decl;

// %printer { yyoutput << (void *)$$; } <*>;

%nonassoc "THEN"
%nonassoc "ELSE"
%%
%start unit;

unit:
  %empty
  {
    yy::lre_parser::error(@$, "Empty program\n");
  }
| mode decl_list "%%" body
  {
    driver.root = $4;
  }
;

mode:
  "USEBDD"
  {
    driver.DS_used = DS::BDD;
  }
| "USECLAUSES"
  {
    driver.DS_used = DS::CLAUSES;
  }
;

decl_list:
  decl decl decl              {}
;

decl:
  type_ds                     { $$ = $1; }
| decl "identifier"           { symtab.addsymbol($1, $2); $$ = $1; }
;

type_ds:
  "REGION"                    { $$ = type::region; }
| "CUBE"                      { $$ = type::cube; }
| "INDEX"                     { $$ = type::index; }
;

body:
  %empty                      { $$ = new lre_node(node_type::empty);}
| body breakpoint             { $$ = new lre_node($1, $2, node_type::compose); }
| body while_block            { $$ = new lre_node($1, $2, node_type::compose); }
| body if_block               { $$ = new lre_node($1, $2, node_type::compose); }
| body exp                    { $$ = new lre_node($1, $2, node_type::compose); }
;

breakpoint:
  "BREAK"                     { $$ = new lre_node(NULL, NULL, node_type::_break_); }
;

while_block:
  "WHILE" "(" bool ")" "{" body "}"       { $$ = new lre_node($3, $6, node_type::while_stmt); }
;

if_block:
  "IF" "(" bool ")" "{" body "}" %prec "THEN"
  { $$ = new lre_node($3, $6, NULL, node_type::if_stmt); }
| "IF" "(" bool ")" "{" body "}" "ELSE" "{" body "}"
  { $$ = new lre_node($3, $6, $10, node_type::if_stmt); }
;

%left "OR";
%left "AND";
%right "NOT";
bool:
  bool "OR" bool                { $$ = new bool_node($1, $3, bool_op::_or); }
| bool "AND" bool               { $$ = new bool_node($1, $3, bool_op::_and); }
| "NOT" bool                    { $$ = new bool_node($2, bool_op::_not); }
| label_rev                     { $$ = new bool_node($1, bool_op::_cast); }
| label_rev eq label_rev
  {
    if (symtab[$1.val].entry_type != symtab[$3.val].entry_type)
      yy::lre_parser::error(@$, "unequal data types");
    $$ = new bool_node($1, $3, bool_op::_eq);
  }
| label_rev gt label_rev
  {
    if (symtab[$1.val].entry_type != type::index)
      yy::lre_parser::error(@$, "data type incompat");
    if (symtab[$3.val].entry_type != type::index)
      yy::lre_parser::error(@$, "data type incompat");
    $$ = new bool_node($1, $3, bool_op::_gt);
  }
| label_rev ge label_rev
  {
    if (symtab[$1.val].entry_type != type::index)
      yy::lre_parser::error(@$, "data type incompat");
    if (symtab[$3.val].entry_type != type::index)
      yy::lre_parser::error(@$, "data type incompat");
    $$ = new bool_node($1, $3, bool_op::_ge);
  }
| label_rev lt label_rev
  {
    if (symtab[$1.val].entry_type != type::index)
      yy::lre_parser::error(@$, "data type incompat");
    if (symtab[$3.val].entry_type != type::index)
      yy::lre_parser::error(@$, "data type incompat");
    $$ = new bool_node($1, $3, bool_op::_lt);
  }
| label_rev le label_rev
  {
    if (symtab[$1.val].entry_type != type::index)
      yy::lre_parser::error(@$, "data type incompat");
    if (symtab[$3.val].entry_type != type::index)
      yy::lre_parser::error(@$, "data type incompat");
    $$ = new bool_node($1, $3, bool_op::_le);
  }
| label_rev eq num_opd
  {
    if (symtab[$1.val].entry_type != type::index)
      yy::lre_parser::error(@$, "data type incompat");
    $$ = new bool_node($1, $3, bool_op::_eq);
  }
| label_rev gt num_opd
  {
    if (symtab[$1.val].entry_type != type::index)
      yy::lre_parser::error(@$, "data type incompat");
    $$ = new bool_node($1, $3, bool_op::_gt);
  }
| label_rev ge num_opd
  {
    if (symtab[$1.val].entry_type != type::index)
      yy::lre_parser::error(@$, "data type incompat");
    $$ = new bool_node($1, $3, bool_op::_ge);
  }
| label_rev lt num_opd
  {
    if (symtab[$1.val].entry_type != type::index)
      yy::lre_parser::error(@$, "data type incompat");
    $$ = new bool_node($1, $3, bool_op::_lt);
  }
| label_rev le num_opd
  {
    if (symtab[$1.val].entry_type != type::index)
      yy::lre_parser::error(@$, "data type incompat");
    $$ = new bool_node($1, $3, bool_op::_le);
  }
;

num_opd:
  "number"                      { $$ = opd($1, opd_type::num); }
;

exp:
  label_rev "=" label_rev
  {
    if (symtab[$1.val].entry_type != symtab[$3.val].entry_type)
      yy::lre_parser::error(@$, "unequal data types");
    $$ = new comp_node($1, $3, comp_type::_copy);
  }
| label_rev "=" num_opd
  {
    if (symtab[$1.val].entry_type != type::index)
      yy::lre_parser::error(@$, "unequal data types");
    $$ = new comp_node($1, $3, comp_type::_copy);
  }
| label_rev "^=" label_rev
  {
    if (symtab[$1.val].entry_type == type::index)
      yy::lre_parser::error(@$, "data type incompat");
    if (symtab[$3.val].entry_type == type::index)
      yy::lre_parser::error(@$, "data type incompat");
    $$ = new comp_node($1, $3, comp_type::_conjunct);
  }
| "INC" label_rev
  {
    if (symtab[$2.val].entry_type != type::index)
      yy::lre_parser::error(@$, "data type incompat");
    $$ = new comp_node($2, comp_type::_inc);
  }
| "DEC" label_rev
  {
    if (symtab[$2.val].entry_type != type::index)
      yy::lre_parser::error(@$, "data type incompat");
    $$ = new comp_node($2, comp_type::_dec);
  }
| "PRIME" "(" label_rev ")"
  {
    if (symtab[$3.val].entry_type == type::index)
      yy::lre_parser::error(@$, "data type incompat");
    $$ = new comp_node($3, comp_type::_prime);
  }
| "SAT" "(" label_rev "AND" label_rev "AND" label_rev "," label_rev ")"
  {
    if (symtab[$3.val].entry_type == type::index)
      yy::lre_parser::error(@$, "data type incompat");
    if (symtab[$5.val].entry_type == type::index)
      yy::lre_parser::error(@$, "data type incompat");
    if (symtab[$7.val].entry_type == type::index)
      yy::lre_parser::error(@$, "data type incompat");
    if (symtab[$9.val].entry_type != type::cube)
      yy::lre_parser::error(@$, "data type incompat");
    $$ = new comp_node($3, $5, $7, $9, comp_type::_sat);
  }
| "SMP" "(" label_rev "," label_rev "," label_rev ")"
  {
    if (symtab[$3.val].entry_type != type::region)
      yy::lre_parser::error(@$, "data type incompat");
    if (symtab[$5.val].entry_type != type::region)
      yy::lre_parser::error(@$, "data type incompat");
    if (symtab[$7.val].entry_type != type::cube)
      yy::lre_parser::error(@$, "data type incompat");
    $$ = new comp_node($3, $5, $7, comp_type::_smp);
  }
| "RETURN" "number"
  { $$ = new comp_node($2, comp_type::_return); }
;

label_rev:
  "identifier"
  {
    if(!symtab.name_exists($1)){
      yy::lre_parser::error(@$, "identifier not found\n");
    }
    $$ = opd(symtab.get_symbol($1), opd_type::var);
  }
| "CNF_ARR" "[" "identifier" "]"              { $$ = opd(symtab.get_symbol("CNF_ARR"), symtab.get_symbol($3), opd_type::arr); }
| "CLS_ARR" "[" "identifier" "]"              { $$ = opd(symtab.get_symbol("CLS_ARR"), symtab.get_symbol($3), opd_type::arr); }
;

%%

void
yy::lre_parser::error (const location_type& l,
                          const std::string& m)
{
  driver.error (l, m);
}
