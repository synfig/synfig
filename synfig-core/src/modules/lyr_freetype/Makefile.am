# $Id$

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/src

AM_CXXFLAGS = \
	@CXXFLAGS@ \
	@FREETYPE_CFLAGS@


moduledir=@MODULE_DIR@

if WITH_FREETYPE

module_LTLIBRARIES = liblyr_freetype.la

liblyr_freetype_la_SOURCES = \
	main.cpp \
	lyr_freetype.cpp \
	lyr_freetype.h

liblyr_freetype_la_LIBADD = \
	../../synfig/libsynfig.la \
	@FREETYPE_LIBS@ \
	@SYNFIG_LIBS@

liblyr_freetype_la_LDFLAGS = \
	-module \
	-no-undefined \
	-avoid-version

liblyr_freetype_la_CXXFLAGS = \
	@SYNFIG_CFLAGS@ \
	@FREETYPE_CFLAGS@

if WITH_FONTCONFIG
liblyr_freetype_la_LIBADD += @FONTCONFIG_LIBS@
liblyr_freetype_la_CXXFLAGS += @FONTCONFIG_CFLAGS@
endif

if WITH_HARFBUZZ
liblyr_freetype_la_LIBADD += @HARFBUZZ_LIBS@
liblyr_freetype_la_CXXFLAGS += @HARFBUZZ_CFLAGS@
endif

if WITH_FRIBIDI
liblyr_freetype_la_LIBADD += @FRIBIDI_LIBS@
liblyr_freetype_la_CXXFLAGS += @FRIBIDI_CFLAGS@
endif

endif


EXTRA_DIST = \
	lyr_freetype.nsh \
	unlyr_freetype.nsh
