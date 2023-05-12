cmake_minimum_required(VERSION 3.12)
project(s98ml C)

include(FindIconv)
include(CheckIncludeFile)
include(CheckStructHasMember)
include(CheckSymbolExists)
find_package(BISON)
find_package(FLEX)

if(Iconv_FOUND)
message("iconv support enabled")
endif()

check_symbol_exists(strcasecmp "strings.h" HAVE_STRCASECMP)
check_symbol_exists(strtok_r "string.h" HAVE_STRTOK_R)
check_struct_has_member("struct timespec" tv_sec "time.h" HAVE_STRUCT_TIMESPEC)

BISON_TARGET(S98ML_PARSER s98ml_parse.y ${CMAKE_CURRENT_BINARY_DIR}/s98ml_parse.c
  DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/s98ml_parse.h
)
FLEX_TARGET(S98ML_SCANNER s98ml_lex.l ${CMAKE_CURRENT_BINARY_DIR}/s98ml_lex.c
  DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/s98ml_lex.h
)
ADD_FLEX_BISON_DEPENDENCY(S98ML_SCANNER S98ML_PARSER)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
add_executable(s98c
  s98c.c
  s98c.h
  s98c_action.c
  s98c_write.c
  s98c_types.h
  ${BISON_S98ML_PARSER_OUTPUTS}
  ${FLEX_S98ML_SCANNER_OUTPUTS}
)
target_compile_definitions(s98c PRIVATE 
	"$<$<BOOL:${Iconv_FOUND}>:HAVE_ICONV>"
	"$<$<BOOL:${WIN32}>:YY_NO_UNISTD_H>"
	"$<$<BOOL:${HAVE_STRCASECMP}>:HAVE_STRCASECMP>"
	"$<$<BOOL:${HAVE_STRTOK_R}>:HAVE_STRTOK_R>"
	"$<$<BOOL:${HAVE_STRUCT_TIMESPEC}>:HAVE_STRUCT_TIMESPEC>"
)
if(Iconv_FOUND)
target_link_libraries(s98c
Iconv::Iconv
)
endif()

add_executable(s98d
  s98d.c
  s98d.h
  s98d_device.c
  s98d_header.c
  s98d_dump.c
  s98d_types.h
)
target_compile_definitions(s98d PRIVATE 
	"$<$<BOOL:${Iconv_FOUND}>:HAVE_ICONV>"
	"$<$<BOOL:${WIN32}>:YY_NO_UNISTD_H>"
	"$<$<BOOL:${HAVE_STRCASECMP}>:HAVE_STRCASECMP>"
	"$<$<BOOL:${HAVE_STRTOK_R}>:HAVE_STRTOK_R>"
	"$<$<BOOL:${HAVE_STRUCT_TIMESPEC}>:HAVE_STRUCT_TIMESPEC>"
)
