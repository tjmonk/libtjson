# libtjson
JSON Parser using lex/yacc

## Prerequisites

This JSON parser supports the varserver project ( https://github.com/tjmonk/varserver ) and depends on the VarObject and VAR_HANDLE types defined there.

## Features

- Construct a JSON object and output it as a string to a FILE *

- Parse a JSON string into a JSON object in memory

- Find elements in a JSON object

- Extract elements from a JSON object as primitive data types

## Example: Construct a JSON object

```
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
```

## Example: Parse a JSON object from a string

```
    inbuf = "{\"sensorId\":\"0x000070B3D5750F0B\",\"timestamp\":\"2023-01-27T01:08:25Z\",\"channels\":[{\"type\":\"PHASE_A_CONSUMPTION\",\"ch\":1,\"eImp_Ws\":95060308549,\"eExp_Ws\":2231,\"p_W\":915,\"q_VAR\":-82,\"v_V\":120.398},{\"type\":\"PHASE_B_CONSUMPTION\",\"ch\":2,\"eImp_Ws\":64627172802,\"eExp_Ws\":2671,\"p_W\":275,\"q_VAR\":-56,\"v_V\":121.061},{\"type\":\"CONSUMPTION\",\"ch\":3,\"eImp_Ws\":159687481246,\"eExp_Ws\":4541,\"p_W\":1189,\"q_VAR\":-138,\"v_V\":120.729}],\"cts\":[{\"ct\":1,\"p_W\":915,\"q_VAR\":-82,\"v_V\":120.398},{\"ct\":2,\"p_W\":275,\"q_VAR\":-56,\"v_V\":121.061},{\"ct\":3,\"p_W\":0,\"q_VAR\":0,\"v_V\":0.000},{\"ct\":4,\"p_W\":0,\"q_VAR\":0,\"v_V\":120.399}]}";

    pNode = JSON_ProcessBuffer( inbuf );
    JSON_Print( pNode, stdout, false );
    JSON_Free( pNode );
```
