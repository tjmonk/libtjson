%{
#define YY_NO_INPUT

#include "y.h"

%}

%option nounput

letter [a-zA-Z\_]
digit [0-9]
nzdigit [1-9]

nl [\n]
delim [ \t]
ws {delim}+

cmt "//"
lparen "("
rparen ")"
lbrace "{"
rbrace "}"
lbracket "["
rbracket "]"

comma ","
semi ";"
dot "."
colon ":"

true "true"
false "false"

comment {cmt}(.*)$
id {letter}({letter}|{digit})*
num [-]?({digit}|({nzdigit}{digit}*))
floatnum  [-+]?[0-9]*\.[0-9]+([eE][-+]?[0-9]+)?
/*charstr ([\"][^\"]*[\"])*/
charstr \"([^"\\]|\\.)*\"
%%

{ws} {/* No action for white space */}
{comment} { /* No action for comments */};
{nl} {/*incrementLineNumber();*/}

{true} return(TRUE);
{false} return(FALSE);

{lbrace} return(LBRACE);
{rbrace} return(RBRACE);
{lbracket} return(LBRACKET);
{rbracket} return(RBRACKET);

{dot} return(DOT);
{comma} return(COMMA);
{semi} return(SEMI);
{colon} return(COLON);

{charstr} return (CHARSTR);

{id} return(ID);
{num} return(NUM);
{floatnum} return(FLOAT);

%%

int yywrap()
{
    return 1;
}
