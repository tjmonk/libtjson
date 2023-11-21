cmake_minimum_required(VERSION 3.10)

set(CMAKE_BUILD_TYPE Debug)

project(tjson
	VERSION 0.3
	DESCRIPTION "Tiny JSON Parser"
)

include(GNUInstallDirs)

find_package(BISON)
find_package(FLEX)

FLEX_TARGET( TJSON_Scanner src/lexan.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c )
BISON_TARGET( TJSON_Parser src/json_parser.y ${CMAKE_CURRENT_BINARY_DIR}/y.c )
ADD_FLEX_BISON_DEPENDENCY(TJSON_Scanner TJSON_Parser)

add_executable( jsontest
    src/jsontest.c
)

target_link_libraries( jsontest
    tjson
)

add_library( ${PROJECT_NAME} SHARED
    src/json.c
    ${BISON_TJSON_Parser_OUTPUTS}
    ${FLEX_TJSON_Scanner_OUTPUTS}
)

set_target_properties( ${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    POSITION_INDEPENDENT_CODE ON
)

set(TJSON_HEADERS inc/tjson/json.h)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    PUBLIC_HEADER "${TJSON_HEADERS}"
)

target_include_directories( ${PROJECT_NAME} PRIVATE . )

target_include_directories( ${PROJECT_NAME} PUBLIC inc )

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tjson )

install(TARGETS jsontest
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

