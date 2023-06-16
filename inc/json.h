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

#ifndef LIBJSON_H
#define LIBJSON_H

/*============================================================================
        Includes
============================================================================*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/*============================================================================
        Defines
============================================================================*/

#ifndef EOK
#define EOK 0
#endif

/*============================================================================
        Public Types
============================================================================*/

/*! Variable type enumeration */
typedef enum _JVarType
{
    /*! invalid variable type */
    VARTYPE_INVALID = 0,

    /*! 16-bit unsigned integer */
    VARTYPE_UINT16,

    /*! 16-bit signed integer */
    VARTYPE_INT16,

    /*! 32-bit unsigned integer */
    VARTYPE_UINT32,

    /*! 32-bit signed integer */
    VARTYPE_INT32,

    /*! 64-bit unsigned integer */
    VARTYPE_UINT64,

    /*! 64-bit signed integer */
    VARTYPE_INT64,

    /*! IEEE754 Floating Point Number */
    VARTYPE_FLOAT,

    /*! NUL terminated character string */
    VARTYPE_STR,

    /*! Blob storage type */
    VARTYPE_BLOB,

    /*! end marker for the type enumeration */
    VARTYPE_END_MARKER

} JVarType;

/*! variable data union */
typedef union _JVarData
{
    /*! unsigned 16-bit integer */
    uint16_t ui;

    /*! signed 16-bit integer */
    int16_t i;

    /*! unsigned 32-bit integer */
    uint32_t ul;

    /*! signed 32-bit integer */
    int32_t l;

    /*! unsigned 64-bit integer */
    uint64_t ull;

    /*! signed 64-bit-integer */
    int64_t ll;

    /*! IEEE-754 floating point number */
    float f;

    /*! pointer to a NUL terminated string */
    char *str;

    /*! void pointer to blob data */
    void *blob;
    
} JVarData;

/*! The JType type identifies a JSON node object as one of ARRAY, OBJECT,
    or VAR */
typedef enum _JType
{
    /*! invalid JSON type */
    JSON_INVALID = 0,

    /*! JSON Array */
    JSON_ARRAY = 1,

    /*! JSON Object */
    JSON_OBJECT = 2,

    /*! JSON Variable */
    JSON_VAR = 3,

    /*! JSON Boolean */
    JSON_BOOL = 4
} JType;

/*! The JNode object is the first member of every JSON object.
    It is used to identify the type of the JSON object,
    specify its name, and link to the next JSON object */
typedef struct _JNode
{
    /*! type of the JSON object */
    JType type;

    /*! name of the JSON object */
    char *name;

    /*! pointer to the next JSON object */
    struct _JNode *pNext;
} JNode;


/*! The JArray object is a JSON Array descriptor which
    indicates the array size and links to all the members
    of the Array */
typedef struct _JArray
{
    /*! JSON node */
    JNode node;

    /*! number of elements in the JSON array */
    size_t n;

    /*! pointer to the first JSON object in the JSON Array */
    JNode *pFirst;

    /*! pointer to the last JSON object in the JSON Array */
    JNode *pLast;

} JArray;


/*! The JObject is a JSON object descriptor which
    specifies the number of members in the JSON object
    and links to the members of the JSON object */
typedef struct _JObject
{
    /*! JSON node */
    JNode node;

    /*! number of elements in the JSON object */
    size_t n;

    /*! pointer to the first member of the JSON object */
    JNode *pFirst;

    /*! pointer to the last member of the JSON object */
    JNode *pLast;

} JObject;

/*! A variable object */
typedef struct _JVarObject
{
    /*! variable type */
    JVarType type;

    /*! variable length */
    size_t len;

    /*! variable value */
    JVarData val;

} JVarObject;

/*! The JVar is a JSON value which contains a
    variable server VarObject */
typedef struct _JVar
{
    /*! JSON node */
    JNode node;

    /*! variable data */
    JVarObject var;

} JVar;

/*============================================================================
        Public Function Declarations
============================================================================*/

JNode *JSON_Process( char *inputFile );

JNode *JSON_ProcessBuffer( char *buf );

int JSON_Parse( char *inputFile,
				char *outputFile,
				bool debug );

JNode *JSON_Find( JNode *json, char *key );

int JSON_Iterate( JArray *pArray,
                  int (*fn)(JNode *pNode, void *arg ),
                  void *arg );

JNode *JSON_Attribute( JObject *pObject, char *attribute );

JNode *JSON_Index( JArray *pArray, size_t idx );

int JSON_ArrayAdd( JArray *pArray, JObject *pObject );

int JSON_ObjectAdd( JObject *pObject, JNode *pNode );

void JSON_Free( JNode *json );

void JSON_Print( JNode *json, FILE *fp, bool comma );

JArray *JSON_Array( char *name );

JObject *JSON_Object( char *name );

JVar *JSON_Var( char *name );

JVar *JSON_Num( char *name, int num );

JVar *JSON_ParseNumber( char *name, char *numstr );

JVar *JSON_Float( char *name, float num );

JVar *JSON_Bool( char *name, int num );

JVar *JSON_Str( char *name, char *str );

char *JSON_GetStr( JNode *pNode, char *name );

bool JSON_GetBool( JNode *pNode, char *name );

int JSON_GetNum( JNode *pNode, char *name, int *pVal );

JVarObject *JSON_GetVar( JNode *pNode, char *name );

int JSON_GetFloat( JNode *pNode, char *name, float *pVal );

#endif /* JSON_H */



