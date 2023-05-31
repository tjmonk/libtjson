%{

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <limits.h>
#include <signal.h>
#include "json.h"

/* pointer to the token text */
extern char *yytext;

/* input file for lex */
extern FILE *yyin;

/* error flag */
static bool errorFlag = false;

/* define the parser object to be a JNode pointer */
#define YYSTYPE JNode *

#define YYDEBUG 1

/* function declarations */
void yyerror();
static char *get_charstr( char *str );
static char escape( char c );
int yylex();

/* root of the parsed JSON object */
JNode *root;

/* debug flag */
extern int yydebug;

%}

%token LBRACE
%token RBRACE
%token LBRACKET
%token RBRACKET

%token COMMA
%token SEMI
%token DQUOTE
%token DOT
%token COLON

%token ID
%token NUM
%token FLOAT
%token CHARSTR

%token TRUE
%token FALSE


%%

json           :  json_list
				{
					$$ = $1;
					root = $$;
				}
			   |  json_object
			    {
					$$ = $1;
					root = $$;
			    }
			   ;

json_object    :  LBRACE attribute_list RBRACE
			    {
                    JObject *pObject;
					pObject = JSON_Object( NULL );
                    if( pObject != NULL )
                    {
                        pObject->pFirst = $2;
                        $$ = &pObject->node;
                    }
                    else
                    {
                        $$ = NULL;
                    }
			    }
			   ;

json_list      : LBRACKET value_list RBRACKET
			    {
                    JArray *pArray;

					pArray = JSON_Array( NULL );
                    if( pArray != NULL )
                    {
                        pArray->pFirst = $2;
                        $$ = &pArray->node;
                    }
			    }
			   ;

value_list    : value COMMA value_list
                {
                    $$ = $1;
                    $1->pNext = $3;
                }
              | value
                {
                    $$ = $1;
                }
              ;

attribute_list : attribute COMMA attribute_list
				{
					$$ = $1;
					$1->pNext = $3;
				}
			   |  attribute
				{
					$$ = $1;
				}
			   ;

attribute      : key COLON value
				{
					$$ = $3;
					$$->name = (char *)$1;
				}
			   ;

key            :  CHARSTR
				{
					$$ = (JNode *)get_charstr( yytext );
				}
			   ;

value          : NUM
				{
					$$ = (JNode *)JSON_ParseNumber( NULL, yytext );
				}
			   | FLOAT
				{
					$$ = (JNode *)JSON_Float( NULL, atof( yytext ));
				}
			   | TRUE
				{
					$$ = (JNode *)JSON_Bool( NULL, 1 );
				}
			   | FALSE
				{
					$$ = (JNode *)JSON_Bool( NULL, 0 );
				}
			   | CHARSTR
				{
					$$ = (JNode *)JSON_Str( NULL, get_charstr( yytext ) );
				}
			   | json
				{
					$$ = $1;
				}
			   ;

%%
#include <stdio.h>

/*==========================================================================*/
/*  yyerror                                                                 */
/*!
	Display syntax error and set error flag to true

    The yyerror function is invoked by the parser when a parse failure
    occurs.  It outputs a "syntax error" error message and sets the
    global error flag.

============================================================================*/
void yyerror()
{
	printf("syntax error\n");
    errorFlag = true;
}

/*==========================================================================*/
/*  get_charstr                                                             */
/*!
	Process the received CHARSTR token

    The get_charstr function strips of the leading and trailing double
    quotes and translates escape character sequences, eg \r \n \t, etc.

    @param[in]
        str
            pointer to the NUL terminated character string to be processed

    @return pointer to the processed character string

============================================================================*/
static char *get_charstr( char *str )
{
    char *s = NULL;
    size_t l;
    int i = 0;  /* input index */
    int j = 0;  /* output index */
    int state = 0;
    char c;

    if( str != NULL )
    {
        /* make a duplicate of the character string (removing the leading
           double quote) so we can modify it */
        s = strdup( &str[1] );
        if( s != NULL )
        {
            /* remove the trailing double quotes */
            l = strlen( s );
            s[l-1] = 0;

            /* handle escaped characters in the string */
            for ( i = 0; i < l-1 ; i++ )
            {
                /* simple state machine for handling escape processing */
                switch( state )
                {
                    case 0:
                        /* looking for '/' */
                        if( s[i] == '\\' )
                        {
                            /* found '/' so set the next state to handle the
                               escaped character */
                            state = 1;
                        }
                        else
                        {
                            /* not an escape so just store it */
                            s[j++] = s[i];
                        }
                        break;

                    case 1:
                        /* process the escaped character */
                        s[j++] = escape(s[i]);

                        /* reset back to looking for regular characters */
                        state = 0;
                        break;
                }
            }

            /* NUL terminate */
            s[j++] = 0;
        }
    }

    return s;

}

/*==========================================================================*/
/*  escape                                                                  */
/*!
	Convert an escape character to its binary equivalent

    The escape function converts the escaped character into its binary
    equivalent.  The specified escape character is the one with would
    follow a backlash, eg n r t 0, etc.

    The following characters will be escaped: \ 0 r n t ' "

    @param[in]
        c
            character to be escaped

    @return escaped character or the original character if no escape performed

============================================================================*/
static char escape( char c )
{
    char escaped;

    switch( c )
    {
        case '\\':
            escaped = '\\';
            break;

        case '0':
            escaped = '\0';
            break;

        case 'r':
            escaped = '\r';
            break;

        case 'n':
            escaped = '\n';
            break;

        case 't':
            escaped = '\t';
            break;

        case '\'':
            escaped = '\'';
            break;

        case '"':
            escaped = '"';
            break;

        default:
            escaped = c;
    }

    return escaped;
}

#if 0
/*==========================================================================*/
/*  main                                                                    */
/*!
    Main entry point for the json parser

    @param[in]
        argc
            number of arguments on the command line
            (including the command itself)

    @param[in]
        argv
            array of pointers to the command line arguments

    @return none

============================================================================*/
int main(int argC, char *argV[])
{
    int c;
    int errflag = 0;
    char *filename;
    char *output_filename = (char *)NULL;
    JNode *pNode;

    if( argC < 2)
    {
        fprintf(stderr,
                "usage: %s [-t] [-o <outputfile>] <sourcefile>\n"
                "\t-t output parse tree\n"
                "\t-o output file\n",
                argV[0]);
        return -1;
    }

    /* parse the command line options */
    while( ( c = getopt( argC, argV, "do:" ) ) != -1 )
    {
        switch( c )
        {
            case 'o':
                output_filename = optarg;
                break;

			case 'd':
				yydebug = 1;
				break;

            case '?':
                ++errflag;
                break;

            default:
                break;
        }
    }

    filename = argV[argC - 1];
    if ( filename != (char *)NULL )
    {
        /* input file was specified */
        if ((yyin = fopen(filename, "r")) == (FILE *)NULL)
        {
            fprintf(stderr, "file %s not found.\n", filename);
            return -1;
        }
    }

    if( output_filename != NULL )
    {
        fp = fopen( output_filename, "w" );
        if( fp == (FILE *)NULL )
        {
            fprintf(stderr, "Unable to create file: %s\n", output_filename);
            return -1;
        }
    }
    else
    {
        fp = stdout;
    }

	/* parse the input file */
    yyparse();

	JSON_Print(root, stdout, false );
	printf("\n");

    pNode = JSON_Find( root, "letters" );
    printf("letters:\n");
    JSON_Print( pNode, stdout, false );
    printf("\n");

    pNode = JSON_Find( root, "numbers" );
    printf("numbers:\n");
    JSON_Print( pNode, stdout, false );
    printf("\n");

    printf("D:");
    pNode = JSON_Find( root, "D" );
    JSON_Print( pNode, stdout, false );
    printf("\n");

    printf("Schedule:\n");
    pNode = JSON_Find( root, "schedule" );
    JSON_Print( pNode, stdout, false );
    printf("\n");

    if( fp != stdout )
    {
        fclose( fp );
    }

    return 0;
}
#endif
