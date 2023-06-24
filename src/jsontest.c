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

/*!
 * @defgroup json_test json_test
 * @brief Test application for the libjson.so library
 * @{
 */

/*==========================================================================*/
/*!
@file jsontest.c

    JSON tester

    The jsontest Application processes an input JSON
    file and dumps the JSON object to the specified
    output file (or stdout) if no file is specified

*/
/*==========================================================================*/

/*============================================================================
        Includes
============================================================================*/

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <tjson/json.h>

/*============================================================================
        Defines
============================================================================*/

/*============================================================================
        External Variables
============================================================================*/

/*============================================================================
        Public Types
============================================================================*/

/*============================================================================
        Private Function Declarations
============================================================================*/
static void usage( void );
static void BuildObj( void );

/*============================================================================
        Public Function Declarations
============================================================================*/


/*==========================================================================*/
/*  main                                                                    */
/*!
    Main entry point for the JSON test application

    The main function starts the JSON test application

    @param[in]
        argc
            number of arguments on the command line
            (including the command itself)

    @param[in]
        argv
            array of pointers to the command line arguments

    @return none

============================================================================*/
void main(int argc, char **argv)
{
    char *inputFile = NULL;
    char *outputFile = NULL;
    int c;
    bool debug = false;
    char *inbuf;
    JNode *pNode;

    while( ( c = getopt( argc, argv, "do:hb" ) ) != -1 )
    {
        switch( c )
        {
            case 'b':
                BuildObj();
                break;

            case 'd':
                debug = true;
                break;

            case 'o':
                outputFile = optarg;
                break;

            case 'h':
                usage();
                break;

            default:
                printf("invalid option: %c\n", c );
                break;
        }
    }

    /* get the name of the input file */
    inputFile = argv[optind];

    if( inputFile != NULL )
    {
        JSON_Parse( inputFile,
		    outputFile,
		    debug );
    }
    else
    {
        inbuf = "{\"sensorId\":\"0x000070B3D5750F0B\",\"timestamp\":\"2023-01-27T01:08:25Z\",\"channels\":[{\"type\":\"PHASE_A_CONSUMPTION\",\"ch\":1,\"eImp_Ws\":95060308549,\"eExp_Ws\":2231,\"p_W\":915,\"q_VAR\":-82,\"v_V\":120.398},{\"type\":\"PHASE_B_CONSUMPTION\",\"ch\":2,\"eImp_Ws\":64627172802,\"eExp_Ws\":2671,\"p_W\":275,\"q_VAR\":-56,\"v_V\":121.061},{\"type\":\"CONSUMPTION\",\"ch\":3,\"eImp_Ws\":159687481246,\"eExp_Ws\":4541,\"p_W\":1189,\"q_VAR\":-138,\"v_V\":120.729}],\"cts\":[{\"ct\":1,\"p_W\":915,\"q_VAR\":-82,\"v_V\":120.398},{\"ct\":2,\"p_W\":275,\"q_VAR\":-56,\"v_V\":121.061},{\"ct\":3,\"p_W\":0,\"q_VAR\":0,\"v_V\":0.000},{\"ct\":4,\"p_W\":0,\"q_VAR\":0,\"v_V\":120.399}]}";

        pNode = JSON_ProcessBuffer( inbuf );
        JSON_Print( pNode, stdout, false );
        JSON_Free( pNode );
    }
}

/*==========================================================================*/
/*  usage                                                                   */
/*!
    Program usage message

    The usage function outputs the program usage message to stdout
    and aborts the application

============================================================================*/
static void usage( void )
{
    printf("usage: jsontest [-d] [-o output_file] [-h] [-b]\n" );
    printf("\t-d enable debug output\n");
    printf("\t-h display this help\n");
    printf("\t-b build a sample object\n");
    printf("\t-o <filename> specifies the output file\n");

    exit( 0 );
}

/*==========================================================================*/
/*  BuildObj                                                                */
/*!
    Build a JSON Object

    The BuildObj function builds a JSON object using the basic JSON primitive
    construction functions.

============================================================================*/
static void BuildObj( void )
{
    JObject *root;
    JArray *constants;
    JObject *meta;
    JObject *pi;
    JObject *phi;
    JObject *e;
    JObject *ln2;

    root = JSON_Object( NULL );
    JSON_ObjectAdd( root, (JNode *)JSON_Str( "date", "2020/10/13" ) );
    JSON_ObjectAdd( root, (JNode *)JSON_Str( "time", "21:12") );

    meta = JSON_Object("meta" );
    JSON_ObjectAdd( meta, (JNode *)JSON_Bool( "enabled", 0 ) );
    JSON_ObjectAdd( meta, (JNode *)JSON_Str( "priority", "high" ) );

    pi = JSON_Object( NULL );
    JSON_ObjectAdd( pi, (JNode *)JSON_Str( "name" , "pi" ) );
    JSON_ObjectAdd( pi, (JNode *)JSON_Float( "value", 3.1415 ) );

    phi = JSON_Object( NULL );
    JSON_ObjectAdd( phi, (JNode *)JSON_Str( "name" , "phi" ) );
    JSON_ObjectAdd( phi, (JNode *)JSON_Float( "value", 1.61803 ) );

    e = JSON_Object( NULL );
    JSON_ObjectAdd( e, (JNode *)JSON_Str( "name" , "e" ) );
    JSON_ObjectAdd( e, (JNode *)JSON_Float( "value", 2.71828 ) );

    ln2 = JSON_Object( NULL );
    JSON_ObjectAdd( ln2, (JNode *)JSON_Str( "name" , "ln2" ) );
    JSON_ObjectAdd( ln2, (JNode *)JSON_Float( "value", 0.69314 ) );

    constants = JSON_Array( "constants" );
    JSON_ArrayAdd( constants, pi );
    JSON_ArrayAdd( constants, phi );
    JSON_ArrayAdd( constants, e );
    JSON_ArrayAdd( constants, ln2 );

    JSON_ObjectAdd( root, (JNode *)constants );
    JSON_ObjectAdd( root, (JNode *)meta );

    JSON_Print( (JNode *)root, stdout, false );

    printf("\n");
}

/*! @}
 * end of json_test group */
