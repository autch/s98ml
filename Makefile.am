
bin_PROGRAMS = s98d lex_test s98c

AM_CFLAGS = -Wall
AM_YFLAGS = -d

# working around bugs in Automake Flex integration :(
BUILT_SOURCES = s98ml_parse.h s98ml_lex.h

s98d_SOURCES = s98d.c s98d_device.c s98d_header.c s98d_dump.c
lex_test_SOURCES = s98ml_parse.y s98ml_lex.l lex_test.c
lex_test_LDADD = -lfl

s98c_SOURCES = s98ml_parse.y s98ml_lex.l 
s98c_LDADD = -lfl

s98ml_lex.h: s98ml_lex.l
	$(LEX) -o /dev/null $<

