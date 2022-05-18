%{
#include <cstdio>
#include <cstring>

#include "compiler/lexer.h"
%}

%option noyywrap
%option c++

identifier [a-zA-Z_][a-zA-Z0-9_]*
string_literal \"([^\\\"]|\\.)*\"

%%

"⊤"                    { return tema::token::tok_truth; }
"⊥"                    { return tema::token::tok_contradiction; }
"→"                    { return tema::token::tok_implies; }
"⟷"                    { return tema::token::tok_equiv; }
"¬"                    { return tema::token::tok_neg; }
"∧"                    { return tema::token::tok_conj; }
"∨"                    { return tema::token::tok_disj; }
"∀"                    { return tema::token::tok_forall; }

"="                    { return tema::token::tok_eq; }
"≠"                    { return tema::token::tok_n_eq; }
"<"                    { return tema::token::tok_less; }
"≮"                    { return tema::token::tok_n_less; }
"≤"                    { return tema::token::tok_eq_less; }
"≰"                    { return tema::token::tok_n_eq_less; }
">"                    { return tema::token::tok_greater; }
"≯"                    { return tema::token::tok_n_greater; }
"≥"                    { return tema::token::tok_eq_greater; }
"≱"                    { return tema::token::tok_n_eq_greater; }
"∈"                    { return tema::token::tok_in; }
"∉"                    { return tema::token::tok_n_in; }
"⊃"                    { return tema::token::tok_includes; }
"⊅"                    { return tema::token::tok_n_includes; }
"⊇"                    { return tema::token::tok_eq_includes; }
"⊉"                    { return tema::token::tok_n_eq_includes; }
"⊂"                    { return tema::token::tok_is_included; }
"⊄"                    { return tema::token::tok_n_is_included; }
"⊆"                    { return tema::token::tok_eq_is_included; }
"⊈"                    { return tema::token::tok_n_eq_is_included; }

"∪"                    { return tema::token::tok_set_union; }
"∩"                    { return tema::token::tok_set_intersection; }
"\\"                   { return tema::token::tok_set_difference; }
"⊖"                    { return tema::token::tok_set_sym_difference; }

{identifier}           { return tema::get_keyword_or_identifier(yytext); }
{string_literal}       { return tema::token::tok_string_literal; }

"#".*                    { }
"("                    { return tema::token::tok_open_paren; }
")"                    { return tema::token::tok_close_paren; }
"\n"                   { return tema::token::tok_eol; }
[ \t]*                 { return tema::token::tok_whitespace; }
.                      { return -1; }

%%
