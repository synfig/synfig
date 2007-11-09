# SYNFIG M4 Macro
# For GNU Autotools
# $Id$
#
# By Robert B. Quattlebaum Jr. <darco@users.sf.net>
#

AC_DEFUN([SYNFIG_DEPS],
[
	USING_ETL(,$2)
	AM_PATH_XML2(,,$2)
	AC_CHECK_FUNCS([floor pow sqrt],,$2)
	$1
])

AC_DEFUN([USING_SYNFIG],
[
	AC_ARG_WITH(synfig-includes,
	[  --with-synfig-includes    Specify location of synfig headers],[
	CXXFLAGS="$CXXFLAGS -I$withval"
	])

	AC_PATH_PROG(SYNFIG_CONFIG,synfig-config,no)

	if test "$SYNFIG_CONFIG" = "no"; then
		no_SYNFIG_config="yes"
		$2
	else
		AC_MSG_CHECKING([if $SYNFIG_CONFIG works])
		if $SYNFIG_CONFIG --libs >/dev/null 2>&1; then
			SYNFIG_VERSION="`$SYNFIG_CONFIG --version`"
			AC_MSG_RESULT([yes, $SYNFIG_VERSION])
			SYNFIG_CXXFLAGS="`$SYNFIG_CONFIG --cxxflags`"
			SYNFIG_CFLAGS="`$SYNFIG_CONFIG --cflags`"
			SYNFIG_LIBS="`$SYNFIG_CONFIG --libs`"
			CXXFLAGS="$CXXFLAGS $SYNFIG_CXXFLAGS"
			AC_SUBST(SYNFIG_CXXFLAGS)
			AC_SUBST(SYNFIG_LIBS)
			AC_SUBST(SYNFIG_CFLAGS)
			$1
		else
			AC_MSG_RESULT(no)
			no_SYNFIG_config="yes"
			$2
		fi
	fi

	SYNFIG_DEPS($1,$2)
])


