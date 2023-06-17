/*============================================================================
MIT License

Copyright (c) 2023 Trevor Monk

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
============================================================================*/

/*============================================================================
        Includes
============================================================================*/

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include "json.h"

/*============================================================================
        Defines
============================================================================*/

#ifndef EOK
#define EOK 0
#endif

typedef struct yy_buffer_state * YY_BUFFER_STATE;

/*============================================================================
        External Variables
============================================================================*/

/*! parser debug flag */
extern int yydebug;

/*! input file for lex */
extern FILE *yyin;

/*! parsing function */
extern int yyparse();

/*! string scanning function */
extern YY_BUFFER_STATE yy_scan_string( char *);

/*! scanneri delete buffer */
extern void yy_delete_buffer( YY_BUFFER_STATE );

/*! scanner destroy function */
extern int yylex_destroy( void );

/*! pointer to the root of the parsed JSON object */
extern JNode *root;

/*============================================================================
        Public Types
============================================================================*/

/*============================================================================
        Private Function Declarations
============================================================================*/
static void json_PrintValue( JVar *pVar, FILE *fp );

/*============================================================================
        Public Function Declarations
============================================================================*/

/*==========================================================================*/
/*  JSON_Process                                                            */
/*!
    Process a JSON object from a file

    The JSON_Process function processes a JSON object from a file
    and builds an in-memory JSON object, returning the root node
    to the user

    @param[in]
        inputFile
            name of the JSON input file

    @retval pointer to the parsed JSON object
    @retval NULL if the JSON object is invalid

============================================================================*/
JNode *JSON_Process( char *inputFile )
{
    JNode *node = NULL;

    if ( inputFile != (char *)NULL )
    {
        /* input file was specified */
        if ((yyin = fopen(inputFile, "r")) != (FILE *)NULL)
        {
            yyparse();
            node = root;
            fclose( yyin );
        }
    }

    return node;
}

/*==========================================================================*/
/*  JSON_ProcessBuffer                                                      */
/*!
    Process a JSON object from a string buffer

    The JSON_ProcessiBuffer function processes a JSON object from a string
    buffer and builds an in-memory JSON object, returning the root node
    to the user

    @param[in]
        buf
            pointer to the input buffer

    @retval pointer to the parsed JSON object
    @retval NULL if the JSON object is invalid

============================================================================*/
JNode *JSON_ProcessBuffer( char *buf )
{
    JNode *node = NULL;
    int rc;
    YY_BUFFER_STATE buffer;

    if ( buf != NULL )
    {
        buffer = yy_scan_string(buf);

        rc = yyparse();

        if ( rc == 0 )
        {
            node = root;
        }

       yy_delete_buffer(buffer);

       yylex_destroy();
    }

    return node;
}

/*==========================================================================*/
/*  JSON_Parse                                                              */
/*!
    Parse a JSON object from a file

    The JSON_Parse function parses a JSON object from a file
    and builds an in-memory JSON object.

    @param[in]
        inputFile
            name of the JSON input file

    @param[in]
        outputFile
            name of the output file

    @param[in]
        debug
            true - enable debugging
            false - disabled debugging

    @retval -1 parsing failed
    @retval 0 parsing successful

============================================================================*/
int JSON_Parse( char *inputFile,
				char *outputFile,
				bool debug )
{
    FILE *fp;

	if( debug == true )
	{
		yydebug = 1;
	}

    if ( inputFile != (char *)NULL )
    {
        /* input file was specified */
        if ((yyin = fopen(inputFile, "r")) == (FILE *)NULL)
        {
            fprintf(stderr, "file %s not found.\n", inputFile );
            return -1;
        }
    }

    if( outputFile != NULL )
    {
        fp = fopen( outputFile, "w" );
        if( fp == (FILE *)NULL )
        {
            fprintf(stderr, "Unable to create file: %s\n", outputFile );
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
	fclose( fp );

    return 0;
}

/*==========================================================================*/
/*  JSON_Iterate                                                            */
/*!
    Iterate through a JSON array applying a function to each array element

    The JSON_Iterate function iterates through the specified JSON Array
    invoking the specified function to every element in the array.
    If any function invocation does not return EOK, the return value
    will indicate the error of the last failed function, but the
    iterator will continue to apply the function to all members
    of the array.

    @param[in]
        pArray
            pointer to the JSON Array to iterate over

    @param[in]
        fn
            the function to apply to each JSON object in the array

    @param[in]
        arg
            the argument to pass to the function each time it is invoked

    @retval EOK processed all elements in the JSON array
    @retval EINVAL invalid arguments
    @retval ENOTSUP the JSON specified is not a JSON array
    @retval other error as returned by the invoked function

============================================================================*/
int JSON_Iterate( JArray *pArray,
                  int (*fn)(JNode *pNode, void *arg ),
                  void *arg )
{
    int result = EINVAL;
    int rc;
    JNode *pNode;

    if( ( pArray != NULL ) &&
        ( fn != NULL ) )
    {
        if( pArray->node.type == JSON_ARRAY )
        {
            result = EOK;

            pNode = pArray->pFirst;
            while( pNode != NULL )
            {
                rc = fn( pNode, arg );
                if( rc != EOK )
                {
                    result = rc;
                }
                pNode = pNode->pNext;
            }
        }
        else
        {
            result = ENOTSUP;
        }
    }

    return result;
}

/*==========================================================================*/
/*  JSON_Attribute                                                          */
/*!
    Get the value of the JSON attribute with the specified name

    The JSON_Attribute function gets the value of the JSON attribute
    with the specified name inside the specified JSON object.

    @param[in]
        pObject
            pointer to the JSON Object to search

    @param[in]
        attribute
            the name of the attribute in the JSON object

    @retval pointer to the named attribute
    @retval NULL if the named attribute cannot be found

============================================================================*/
JNode *JSON_Attribute( JObject *pObject, char *attribute )
{
    JNode *pNode = NULL;

    if( ( pObject != NULL ) &&
        ( attribute != NULL ) )
    {
        if( pObject->node.type == JSON_OBJECT )
        {
            pNode = pObject->pFirst;
            while( pNode != NULL )
            {
                if( pNode->name != NULL )
                {
                    if( strcmp( pNode->name, attribute ) == 0 )
                    {
                        break;
                    }
                }

                pNode = pNode->pNext;
            }
        }
    }

    return pNode;
}

/*==========================================================================*/
/*  JSON_Index                                                              */
/*!
    Get the value of the JSON array element at the specified index

    The JSON_Index function gets the value of the JSON array
    element at the specified index

    @param[in]
        pArray
            pointer to the JSON Array to index

    @param[in]
        idx
            the index of the JSON array to retrieve

    @retval pointer to the JSON element at the specified index
    @retval NULL if the named attribute cannot be found

============================================================================*/
JNode *JSON_Index( JArray *pArray, size_t idx )
{
    JNode *pNode;
    JNode *result = NULL;

    size_t n = 0;

    if( pArray != NULL )
    {
        if( pArray->node.type == JSON_ARRAY )
        {
            pNode = pArray->pFirst;
            while( pNode != NULL )
            {
                if( idx == n )
                {
                    result = pNode;
                    break;
                }

                n++;
                pNode = pNode->pNext;
            }
        }
    }

    return result;
}

/*==========================================================================*/
/*  JSON_ArrayAdd                                                           */
/*!
    Add a JSON item to a JSON array

    The JSON_ArrayAdd function adds a JSON item to a JSON array
    at the end of the array.

    @param[in]
        pArray
            pointer to the JSON Array to add to

    @param[in]
        pObject
            a JSON object to add

    @retval EOK the JSON item was added to the array
    @retval EINVAL invalid arguments
    @retval ENOTSUP the specified object is not an array

============================================================================*/
int JSON_ArrayAdd( JArray *pArray, JObject *pObject )
{
    int result = EINVAL;

    if( ( pArray != NULL ) &&
        ( pObject != NULL ) )
    {
        if( pArray->node.type == JSON_ARRAY )
        {
            if( pArray->pFirst == NULL )
            {
                pArray->pFirst = (JNode *)pObject;
                pArray->pLast = (JNode *)pObject;
                result = EOK;
            }
            else
            {
                if( pArray->pLast != NULL )
                {
                    pArray->pLast->pNext = (JNode *)pObject;
                    pArray->pLast = (JNode *)pObject;
                    result = EOK;
                }
            }
        }
        else
        {
            result = ENOTSUP;
        }
    }

    return result;
}

/*==========================================================================*/
/*  JSON_ObjectAdd                                                          */
/*!
    Add a JSON item to a JSON object

    The JSON_ObjectAdd function adds a JSON item to a JSON object.

    @param[in]
        pObject
            pointer to the JSON Object to add to

    @param[in]
        pNode
            a JSON item to add

    @retval EOK the JSON item was added to the object
    @retval EINVAL invalid arguments
    @retval ENOTSUP the specified object is not a JSON object

============================================================================*/
int JSON_ObjectAdd( JObject *pObject, JNode *pNode )
{
    int result = EINVAL;

    if( ( pObject != NULL ) &&
        ( pNode != NULL ) )
    {
        if( pObject->node.type == JSON_OBJECT )
        {
            if( pObject->pFirst == NULL )
            {
                pObject->pFirst = pNode;
                pObject->pLast = pNode;
                result = EOK;
            }
            else
            {
                if( pObject->pLast != NULL )
                {
                    pObject->pLast->pNext = pNode;
                    pObject->pLast = pNode;
                    result = EOK;
                }
            }
        }
        else
        {
            result = ENOTSUP;
        }
    }

    return result;
}

/*==========================================================================*/
/*  JSON_Free                                                               */
/*!
    Free a JSON Node and all its children

    The JSON_Free function frees the JSON object recursively

    @param[in]
        json
            pointer to the JSON Object to free

============================================================================*/
void JSON_Free( JNode *json )
{
    JArray *pArray;
    JVar *pVar;
    JObject *pObject;
    JNode *pNode;
    JNode *pDelete;

    if ( json != NULL )
    {
        if( json->name != NULL )
        {
            free(json->name);
            json->name = NULL;
        }

        switch( json->type )
        {
            case JSON_ARRAY:
                pArray = (JArray *)json;
                pNode = pArray->pFirst;
                while( pNode != NULL )
                {
                    pDelete = pNode;
                    pNode = pNode->pNext;
                    JSON_Free( pDelete );
                }
                memset( pArray, 0, sizeof( JArray ) );
                break;

            case JSON_OBJECT:
                pObject = (JObject *)json;
                pNode = pObject->pFirst;
                while( pNode != NULL )
                {
                    pDelete = pNode;
                    pNode = pNode->pNext;
                    JSON_Free( pDelete );
                }
                memset( pObject, 0, sizeof( JObject ) );
                break;

            case JSON_BOOL:
            case JSON_VAR:
                pVar = (JVar *)json;
                if ( pVar->var.type == JVARTYPE_STR )
                {
                    if ( pVar->var.val.str != NULL )
                    {
                        free( pVar->var.val.str );
                        pVar->var.val.str = NULL;
                    }
                }
                memset( pVar, 0, sizeof( JVar ) );
                break;

            default:
                break;
        }

        free( json );
    }
}

/*==========================================================================*/
/*  JSON_Print                                                              */
/*!
    Output a JSON object to a file

    The JSON_Print function outputs the JSON object recursively
    to the specified output stream

    @param[in]
        json
            pointer to the JSON Object to output

    @param[in]
        fp
            pointer to the output stream

    @param[in]
        comma
            true - output leading comma
            false - no leading comma

============================================================================*/
void JSON_Print( JNode *json, FILE *fp, bool comma )
{
    JArray *pArray;
    JVar *pVar;
    JObject *pObject;
    JNode *pNode;

    if( ( json != NULL ) &&
        ( fp != NULL ) )
    {
        if( comma == true )
        {
            fprintf(fp, "," );
            comma = false;
        }

        if( json->name != NULL )
        {
            fprintf(fp, "\"%s\" : ", json->name );
        }

        switch( json->type )
        {
            case JSON_ARRAY:
                pArray = (JArray *)json;
                fprintf( fp, "[" );
                pNode = pArray->pFirst;
                while( pNode != NULL )
                {
                    JSON_Print( pNode, fp, comma );
                    comma = true;
                    pNode = pNode->pNext;
                }
                fprintf( fp, "]" );
                break;

            case JSON_OBJECT:
                pObject = (JObject *)json;
                fprintf( fp, "{" );
                pNode = pObject->pFirst;
                while( pNode != NULL )
                {
                    JSON_Print( pNode, fp, comma );
                    comma = true;
                    pNode = pNode->pNext;
                }
                fprintf( fp, "}" );
                break;

            case JSON_BOOL:
            case JSON_VAR:
                pVar = (JVar *)json;
                json_PrintValue( pVar, fp );
                break;

            default:
                break;
        }
    }
}

/*==========================================================================*/
/*  json_PrintValue                                                         */
/*!
    Output a JSON value to a file

    The json_PrintValue function outputs the JSON value
    to the specified output stream

    @param[in]
        pVar
            pointer to the JSON Variable to output

    @param[in]
        fp
            pointer to the output stream

============================================================================*/
static void json_PrintValue( JVar *pVar, FILE *fp )
{
    if( ( pVar != NULL ) &&
        ( fp != NULL ) )
    {
        if( pVar->node.type == JSON_BOOL )
        {
            fprintf( fp, "%s", (pVar->var.val.ui > 0 ) ? "true" : "false " );
        }
        else
        {
            switch( pVar->var.type )
            {
                case JVARTYPE_UINT16:
                    fprintf( fp, "%d", pVar->var.val.ui );
                    break;

                case JVARTYPE_UINT32:
                    fprintf( fp, "%d", pVar->var.val.ul );
                    break;

                case JVARTYPE_FLOAT:
                    fprintf( fp, "%f", pVar->var.val.f );
                    break;

                case JVARTYPE_STR:
                    fprintf( fp, "\"%s\"", pVar->var.val.str );
                    break;

                default:
                    break;
            }
        }
    }
}

/*==========================================================================*/
/*  JSON_Find                                                               */
/*!
    Find the specified element in the JSON object

    The JSON_Find function recursively searches through the specified JSON
    object looking for the attribute with the specified key name

    @param[in]
        json
            pointer to the JSON object to search in

    @param[in]
        key
            name of the element to search for in the JSON object

    @retval pointer to the JSON node found
    @retval NULL if the JSON node was not found

============================================================================*/
JNode *JSON_Find( JNode *json, char *key )
{
    JNode *found = NULL;
    JArray *pArray;
    JObject *pObject;

    if( ( key != NULL ) &&
        ( json != NULL ) )
    {
        switch( json->type )
        {
            case JSON_ARRAY:
                if( json->name != NULL )
                {
                    /* see if this array is the one we are looking for */
                    if( strcmp( key, json->name ) == 0 )
                    {
                        found = json;
                    }
                }

                if( found == NULL )
                {
                    /* look for the name inside the array */
                    pArray = (JArray *)json;
                    found = JSON_Find( pArray->pFirst, key );
                }
                break;

            case JSON_OBJECT:
                if( json->name != NULL )
                {
                    /* see if this object is the one we are looking for */
                    if( strcmp( key, json->name ) == 0 )
                    {
                        found = json;
                    }
                }

                if( found == NULL )
                {
                    /* look for the name inside the object */
                    pObject = (JObject *)json;
                    found = JSON_Find( pObject->pFirst, key );
                }
                break;

            case JSON_BOOL:
            case JSON_VAR:
                if( json->name != NULL )
                {
                    if( strcmp( key, json->name ) == 0 )
                    {
                        found = json;
                    }
                }
                break;

            default:
                break;
        }

        if( found == NULL )
        {
            /* look at the next element in the object or array */
            found = JSON_Find( json->pNext, key );
        }
    }

    return found;
}

/*==========================================================================*/
/*  JSON_Array                                                              */
/*!
    Create a JSON array

    The JSON_Array function creates a new JSON array

    @param[in]
        name
            pointer to the name of the JSON object.  This value
            must be on the heap.  This function takes a reference
            to it.

    @retval pointer to a new JSON array
    @retval NULL if the JSON array could not be created

============================================================================*/
JArray *JSON_Array( char *name )
{
    JArray *pArray = calloc( 1, sizeof( JArray ) );
    if( pArray != NULL )
    {
        pArray->node.name = name;
        pArray->node.type = JSON_ARRAY;
    }

    return pArray;
}

/*==========================================================================*/
/*  JSON_Object                                                             */
/*!
    Create a JSON object

    The JSON_Object function creates a new JSON object

    @param[in]
        name
            pointer to the name of the JSON object.  This value
            must be on the heap.  This function takes a reference
            to it.

    @retval pointer to a new JSON array object
    @retval NULL if the JSON array object could not be created

============================================================================*/
JObject *JSON_Object( char *name )
{
    JObject *pObject = calloc( 1, sizeof( JObject ) );
    if( pObject != NULL )
    {
        pObject->node.name = name;
        pObject->node.type = JSON_OBJECT;
    }

    return pObject;
}

/*==========================================================================*/
/*  JSON_Var                                                                */
/*!
    Create a JSON object

    The JSON_Var function creates a new JSON variable

    @param[in]
        name
            pointer to the JSON string object name.  This value
            must be on the heap.  This function takes a reference
            to it.

    @retval pointer to a new JSON variable
    @retval NULL if the JSON variable could not be created

============================================================================*/
JVar *JSON_Var( char *name )
{
    JVar *pVar = calloc( 1, sizeof( JVar ) );
    if( pVar != NULL )
    {
        pVar->node.name = name;
        pVar->node.type = JSON_VAR;
    }

    return pVar;
}

/*==========================================================================*/
/*  JSON_Num                                                                */
/*!
    Create a JSON object

    The JSON_Num function creates a new JSON number variable

    @param[in]
        name
            pointer to the JSON string object name.  This value
            must be on the heap.  This function takes a reference
            to it.

    @param[in]
        num
            value of the JSON Number

    @retval pointer to a new JSON variable
    @retval NULL if the JSON variable could not be created

============================================================================*/
JVar *JSON_Num( char *name, int num )
{
    JVar *pVar = JSON_Var( name );
    if( pVar != NULL )
    {
        pVar->var.type = JVARTYPE_UINT32;
        pVar->var.len = sizeof( uint32_t );
        pVar->var.val.ul = num;
    }

    return pVar;
}

/*==========================================================================*/
/*  JSON_ParseNumber                                                        */
/*!
    Create a JSON Number variable

    The JSON_ParseNumber function creates a new JSON number variable of
    the appropriate type given the number text.  It will parse the number
    into one of the following variable types depending on the value
    of the number:

    - JVARTYPE_UINT16
    - JVARTYPE_INT16
    - JVARTYPE_UINT32
    - JVARTYPE_INT32
    - JVARTYPE_UINT64
    - JVARTYPE_INT64

    @param[in]
        name
            pointer to the JSON string object name.  This value
            must be on the heap.  This function takes a reference
            to it.

    @param[in]
        numstr
            pointer to the number value string

    @retval pointer to a new JSON variable
    @retval NULL if the JSON variable could not be created

============================================================================*/
JVar *JSON_ParseNumber( char *name, char *numstr )
{
    int64_t lli;
    uint64_t llu;

    JVar *pVar = JSON_Var( name );
    if ( ( pVar != NULL ) && ( numstr != NULL ) )
    {
        /* check if number is negative */
        if( numstr[0] == '-' )
        {
            lli = strtoll( numstr, NULL, 10 );
            if ( ( lli > -32768 ) && ( lli < 32767 ) )
            {
                pVar->var.type = JVARTYPE_INT16;
                pVar->var.len = sizeof( int16_t );
                pVar->var.val.i = (int16_t)lli;
            }
            else if ( ( lli > -2147483648 ) && ( lli < 2147483647 ) )
            {
                pVar->var.type = JVARTYPE_INT32;
                pVar->var.len = sizeof( int32_t );
                pVar->var.val.l = (int32_t)lli;
            }
            else
            {
                pVar->var.type = JVARTYPE_INT64;
                pVar->var.len = sizeof( int64_t );
                pVar->var.val.ll = (int64_t)lli;
            }
        }
        else
        {
            llu = strtoull( numstr, NULL, 10 );
            if ( llu < 65535 )
            {
                pVar->var.type = JVARTYPE_UINT16;
                pVar->var.len = sizeof( uint16_t );
                pVar->var.val.ui = (uint16_t)llu;
            }
            else if ( llu < 4294967295 )
            {
                pVar->var.type = JVARTYPE_UINT32;
                pVar->var.len = sizeof( uint32_t );
                pVar->var.val.ul = (uint32_t)llu;
            }
            else
            {
                pVar->var.type = JVARTYPE_UINT64;
                pVar->var.len = sizeof( uint64_t );
                pVar->var.val.ull = llu;
            }
        }
    }

    return pVar;
}

/*==========================================================================*/
/*  JSON_Float                                                              */
/*!
    Create a JSON float object

    The JSON_Float function creates a new JSON float variable

    @param[in]
        name
            pointer to the JSON string object name.  This value
            must be on the heap.  This function takes a reference
            to it.

    @param[in]
        num
            value of the JSON Number

    @retval pointer to a new JSON float variable
    @retval NULL if the JSON float variable could not be created

============================================================================*/
JVar *JSON_Float( char *name, float num )
{
    JVar *pVar = NULL;

    pVar = JSON_Var( name );
    if( pVar != NULL )
    {
        pVar->var.type = JVARTYPE_FLOAT;
        pVar->var.val.f = num;
        pVar->var.len = sizeof( float );
    }

    return pVar;
}

/*==========================================================================*/
/*  JSON_Bool                                                               */
/*!
    Create a JSON bool object

    The JSON_Bool function creates a new JSON bool variable

    @param[in]
        name
            pointer to the JSON string object name.  This value
            must be on the heap.  This function takes a reference
            to it.

    @param[in]
        num
            value of the JSON Number: 0=False, 1=True

    @retval pointer to a new JSON bool variable
    @retval NULL if the JSON bool variable could not be created

============================================================================*/
JVar *JSON_Bool( char *name, int num )
{
    JVar *pVar = NULL;

    pVar = JSON_Var( name );
    if( pVar != NULL )
    {
        pVar->node.type = JSON_BOOL;
        pVar->var.type = JVARTYPE_UINT16;
        pVar->var.len = sizeof( uint16_t );
        pVar->var.val.ui = ( num > 0 ) ? 1 : 0;
    }

    return pVar;
}

/*==========================================================================*/
/*  JSON_Str                                                                */
/*!
    Create a JSON string object

    The JSON_Str function creates a new JSON string variable

    @param[in]
        name
            pointer to the JSON string object name.  This value
            must be on the heap.  This function takes a reference
            to it.

    @param[in]
        str
            pointer to the JSON string object value.  This value
            must be on the heap.  This function takes a reference
            to it.

    @retval pointer to a new JSON string variable
    @retval NULL if the JSON string variable could not be created

============================================================================*/
JVar *JSON_Str( char *name, char *str )
{
    JVar *pVar = NULL;

    pVar = JSON_Var( name );
    if( ( pVar != NULL ) &&
        ( str != NULL ) )
    {
        pVar->var.type = JVARTYPE_STR;
        pVar->var.val.str = str;
        pVar->var.len = strlen( str );
    }

    return pVar;
}

/*============================================================================*/
/*  JSON_GetStr                                                               */
/*!
    Get a JSON string attribute from the specified node

    The JSON_GetStr function checks that the specified node is a JSON object,
    and looks up the object attribute with the specified name and
    returns a pointer to the string value associated with the name.

    @param[in]
        pNode
            pointer to the JSON object to get the value from

    @param[in]
        name
            pointer to the name of the JSON object attribute to search for

    @retval pointer to the value associated with the specified JSON attribute
    @retval NULL unable to retrieve the JSON attribute value

==============================================================================*/
char *JSON_GetStr( JNode *pNode, char *name )
{
    JObject *pObject;
    JVar *pValue;
    char *result = NULL;

    /* check that the node is an object */
    if ( ( pNode != NULL ) &&
         ( pNode->type == JSON_OBJECT ) &&
         ( name != NULL ) )
    {
        /* get a pointer to the first attribute of the object */
        pObject = (JObject *)pNode;
        pNode = pObject->pFirst;
        while( pNode != NULL )
        {
            if( strcmp( pNode->name, name ) == 0 )
            {
                if( pNode->type == JSON_VAR )
                {
                    pValue = (JVar *)pNode;
                    if( pValue->var.type == JVARTYPE_STR )
                    {
                        result = pValue->var.val.str;
                        break;
                    }
                }
            }

            /* move to the next attribute in the object */
            pNode = pNode->pNext;
        }
    }

    return result;
}

/*============================================================================*/
/*  JSON_GetBool                                                              */
/*!
    Get a boolean value attribute from the specified node

    The JSON_GetBool function checks that the specified node is a JSON object,
    and looks up the object attribute with the specified name and
    returns a pointer to the boolean value associated with the name.

    @param[in]
        pNode
            pointer to the JSON object to get the value from

    @param[in]
        name
            pointer to the name of the JSON object attribute to search for

    @retval true the value is present and is true
    @retval false the value is present and is false or the value is not present

==============================================================================*/
bool JSON_GetBool( JNode *pNode, char *name )
{
    JObject *pObject;
    JVar *pValue;
    bool result = false;

    /* check that the node is an object */
    if ( ( pNode != NULL ) &&
         ( pNode->type == JSON_OBJECT ) &&
         ( name != NULL ) )
    {
        /* get a pointer to the first attribute of the object */
        pObject = (JObject *)pNode;
        pNode = pObject->pFirst;
        while( pNode != NULL )
        {
            if( strcmp( pNode->name, name ) == 0 )
            {
                if( pNode->type == JSON_BOOL )
                {
                    pValue = (JVar *)pNode;
                    if( pValue->var.type == JVARTYPE_UINT16 )
                    {
                        result = ( pValue->var.val.ui == 0 ) ? false : true;
                        break;
                    }
                }
            }

            /* move to the next attribute in the object */
            pNode = pNode->pNext;
        }
    }

    return result;
}

/*============================================================================*/
/*  JSON_GetNum                                                               */
/*!
    Get an integer value attribute from the specified node

    The JSON_GetNum function checks that the specified node is a JSON object,
    and looks up the object attribute with the specified name and
    returns the integer value associated with the name.

    @param[in]
        pNode
            pointer to the JSON object to get the value from

    @param[in]
        name
            pointer to the name of the JSON object attribute to search for

    @param[in]
        pVal
            pointer to a location to store the retrieved value

    @retval 0 the number was found and returned
    @retval -1 the number could not be retrieved

==============================================================================*/
int JSON_GetNum( JNode *pNode, char *name, int *pVal )
{
    JObject *pObject;
    JVar *pValue;
    int result = -1;

    /* check that the node is an object */
    if ( ( pNode != NULL ) &&
         ( pNode->type == JSON_OBJECT ) &&
         ( name != NULL ) &&
         ( pVal != NULL ) )
    {
        /* get a pointer to the first attribute of the object */
        pObject = (JObject *)pNode;
        pNode = pObject->pFirst;
        while( pNode != NULL )
        {
            if( strcmp( pNode->name, name ) == 0 )
            {
                if( pNode->type == JSON_VAR )
                {
                    pValue = (JVar *)pNode;
                    if( pValue->var.type == JVARTYPE_UINT32 )
                    {
                        *pVal = pValue->var.val.ul;
                        result = 0;
                        break;
                    }
                }
            }

            /* move to the next attribute in the object */
            pNode = pNode->pNext;
        }
    }

    return result;
}

/*============================================================================*/
/*  JSON_GetVar                                                               */
/*!
    Get a variable object value attribute from the specified node

    The JSON_GetVar function checks that the specified node is a JSON object,
    and looks up the object attribute with the specified name and
    returns the variable object associated with that name.
    Note that the variable object returned is a reference to the JSON object
    value and should not be modified by the caller.

    @param[in]
        pNode
            pointer to the JSON object to get the value from

    @param[in]
        name
            pointer to the name of the JSON object attribute to search for

    @retval pointer to the VarObject associated with the specified name
    @retval NULL if the VarObject was not found

==============================================================================*/
JVarObject *JSON_GetVar( JNode *pNode, char *name )
{
    JObject *pObject;
    JVar *pValue;
    int result = -1;
    JVarObject *pVar = NULL;

    /* check that the node is an object */
    if ( ( pNode != NULL ) &&
         ( pNode->type == JSON_OBJECT ) &&
         ( name != NULL ) )
    {
        /* get a pointer to the first attribute of the object */
        pObject = (JObject *)pNode;
        pNode = pObject->pFirst;
        while( pNode != NULL )
        {
            if( strcmp( pNode->name, name ) == 0 )
            {
                if( pNode->type == JSON_VAR )
                {
                    pValue = (JVar *)pNode;
                    pVar = &(pValue->var);
                    break;
                }
            }

            /* move to the next attribute in the object */
            pNode = pNode->pNext;
        }
    }

    return pVar;
}

/*============================================================================*/
/*  JSON_GetFloat                                                             */
/*!
    Get a floating point value attribute from the specified node

    The JSON_GetFloat function checks that the specified node is a JSON object,
    and looks up the object attribute with the specified name and
    returns the floating point value associated with the name.

    @param[in]
        pNode
            pointer to the JSON object to get the value from

    @param[in]
        name
            pointer to the name of the JSON object attribute to search for

    @param[in]
        pVal
            pointer to a location to store the retrieved value

    @retval 0 the number was found and returned
    @retval -1 the number could not be retrieved

==============================================================================*/
int JSON_GetFloat( JNode *pNode, char *name, float *pVal )
{
    JObject *pObject;
    JVar *pValue;
    int result = -1;

    /* check that the node is an object */
    if ( ( pNode != NULL ) &&
         ( pNode->type == JSON_OBJECT ) &&
         ( name != NULL ) &&
         ( pVal != NULL ) )
    {
        /* get a pointer to the first attribute of the object */
        pObject = (JObject *)pNode;
        pNode = pObject->pFirst;
        while( pNode != NULL )
        {
            if( strcmp( pNode->name, name ) == 0 )
            {
                if( pNode->type == JSON_VAR )
                {
                    pValue = (JVar *)pNode;
                    if( pValue->var.type == JVARTYPE_FLOAT )
                    {
                        *pVal = pValue->var.val.f;
                        result = 0;
                        break;
                    }
                }
            }

            /* move to the next attribute in the object */
            pNode = pNode->pNext;
        }
    }

    return result;
}

