
bin_PROGRAMS = s98d s98c

AM_CFLAGS = -Wall
AM_YFLAGS = -d

# working around bugs in Automake Flex integration :(
BUILT_SOURCES = s98ml_lex.h s98ml_parse.h

s98d_SOURCES = s98d.c s98d_device.c s98d_header.c s98d_dump.c

s98c_SOURCES = s98c.c s98c_action.c s98c_write.c s98ml_parse.y s98ml_lex.l 
s98c_LDADD = @LIBICONV@

s98ml_lex.h: s98ml_lex.l
	$(LEX) -o /dev/null $<

