cmake_minimum_required(VERSION 3.12)
project(s98ml C)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})
include(FindIconv)
include(CheckIncludeFile)
include(CheckStructHasMember)
find_package(BISON)
find_package(FLEX)

if(Iconv_FOUND)
message("iconv support enabled")
add_compile_definitions(HAVE_ICONV)
endif()

check_struct_has_member("struct timespec" tv_sec "time.h" HAVE_STRUCT_TIMESPEC)

BISON_TARGET(S98ML_PARSER s98ml_parse.y ${CMAKE_CURRENT_BINARY_DIR}/s98ml_parse.c
  DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/s98ml_parse.h
)
FLEX_TARGET(S98ML_SCANNER s98ml_lex.l ${CMAKE_CURRENT_BINARY_DIR}/s98ml_lex.c
  DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/s98ml_lex.h
)
ADD_FLEX_BISON_DEPENDENCY(S98ML_SCANNER S98ML_PARSER)

if (HAVE_STRUCT_TIMESPEC)
add_definitions(-DHAVE_STRUCT_TIMESPEC)
endif (HAVE_STRUCT_TIMESPEC)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
add_executable(s98c
  s98c.c
  s98c_action.c
  s98c_write.c
  ${BISON_S98ML_PARSER_OUTPUTS}
  ${FLEX_S98ML_SCANNER_OUTPUTS}
)
target_link_libraries(s98c
Iconv::Iconv
)

add_executable(s98d
  s98d.c
  s98d_device.c
  s98d_header.c
  s98d_dump.c
)

