%{ /* -*- C++ -*- */
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <string>
#include "driver.hh"
#include "parser.hh"

// Work around an incompatibility in flex (at least versions
// 2.5.31 through 2.5.33): it generates code that does
// not conform to C89.  See Debian bug 333231
// <http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=333231>.
# undef yywrap
# define yywrap() 1

// The location of the current token.
static yy::location loc;
%}
%option noyywrap nounput batch debug noinput

comm  #.*
id    [a-zA-Z][a-zA-Z_0-9]*
int   [0-9]+
blank [ \t]

%{
  // Code run each time a pattern is matched.
  # define YY_USER_ACTION  loc.columns (yyleng);
%}

%%

%{
  // Code run each time yylex is called.
  loc.step ();
%}

{blank}+        loc.step ();
{comm}        loc.step ();
[\n]+           loc.lines (yyleng); loc.step ();
"use_bdd"       return yy::lre_parser::make_USEBDD(loc);
"use_clauses"   return yy::lre_parser::make_USECLAUSES(loc);
"$$"            return yy::lre_parser::make_SEPARATOR(loc);
"SET"           return yy::lre_parser::make_SET(loc);
"PRIME"         return yy::lre_parser::make_PRIME(loc);
"return"        return yy::lre_parser::make_RETURN(loc);
"region"        return yy::lre_parser::make_REGION(loc);
"cube"          return yy::lre_parser::make_CUBE(loc);
"index"         return yy::lre_parser::make_INDEX(loc);
"("             return yy::lre_parser::make_LPAREN(loc);
")"             return yy::lre_parser::make_RPAREN(loc);
","             return yy::lre_parser::make_COMMA(loc);
"if"            return yy::lre_parser::make_IF(loc);
"else"          return yy::lre_parser::make_ELSE(loc);
"while"         return yy::lre_parser::make_WHILE(loc);
"{"             return yy::lre_parser::make_CLPAREN(loc);
"}"             return yy::lre_parser::make_CRPAREN(loc);
"["             return yy::lre_parser::make_SLPAREN(loc);
"]"             return yy::lre_parser::make_SRPAREN(loc);
"="             return yy::lre_parser::make_ASSIGN(loc);
"^="            return yy::lre_parser::make_CONJUNCT(loc);
"++"            return yy::lre_parser::make_INC(loc);
"--"            return yy::lre_parser::make_DEC(loc);
"~"             return yy::lre_parser::make_NEGATION(loc);
"||"            return yy::lre_parser::make_OR(loc);
"^"             return yy::lre_parser::make_AND(loc);
"!"             return yy::lre_parser::make_NOT(loc);
"<"             return yy::lre_parser::make_lt(loc);
"<="            return yy::lre_parser::make_le(loc);
">"             return yy::lre_parser::make_gt(loc);
">="            return yy::lre_parser::make_ge(loc);
"=="            return yy::lre_parser::make_eq(loc);
"!="            return yy::lre_parser::make_ne(loc);
"SAT"           return yy::lre_parser::make_SAT(loc);
"SMP"           return yy::lre_parser::make_SMP(loc);
"CNF_ARR"       return yy::lre_parser::make_CNFARR(loc);
"CLS_ARR"       return yy::lre_parser::make_CLSARR(loc);

{int}      {
  errno = 0;
  long n = strtol (yytext, NULL, 10);
  if (! (INT_MIN <= n && n <= INT_MAX && errno != ERANGE))
    driver.error (loc, "integer is out of range");
  return yy::lre_parser::make_NUMBER(n, loc);
}

{id}       return yy::lre_parser::make_IDENTIFIER(yytext, loc);
.          driver.error (loc, "invalid character");
<<EOF>>    return yy::lre_parser::make_END(loc);
%%

void
lre_driver::scan_begin ()
{
  yy_flex_debug = trace_scanning;
  if (file.empty () || file == "-")
    yyin = stdin;
  else if (!(yyin = fopen (file.c_str (), "r")))
    {
      error ("cannot open " + file + ": " + strerror(errno));
      exit (EXIT_FAILURE);
    }
}

void
lre_driver::scan_end ()
{
  fclose (yyin);
}

