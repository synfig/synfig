# SINFG M4 Macro
# For GNU Autotools
# $Id: sinfg.m4,v 1.1.1.1 2005/01/07 03:34:35 darco Exp $
#
# By Robert B. Quattlebaum Jr. <darco@users.sf.net>
#

AC_DEFUN([SINFG_DEPS],
[
	USING_ETL(,$2)
	AM_PATH_XML2(,,$2)
	AC_CHECK_FUNCS([floor pow sqrt],,$2)
	$1
])

AC_DEFUN([USING_SINFG],
[
	AC_ARG_WITH(sinfg-includes,
	[  --with-sinfg-includes    Specify location of sinfg headers],[
	CXXFLAGS="$CXXFLAGS -I$withval"
	])

	AC_PATH_PROG(SINFG_CONFIG,sinfg-config,no)

	if test "$SINFG_CONFIG" = "no"; then
		no_SINFG_config="yes"
		$2
	else
		AC_MSG_CHECKING([if $SINFG_CONFIG works])
		if $SINFG_CONFIG --libs >/dev/null 2>&1; then
			SINFG_VERSION="`$SINFG_CONFIG --version`"
			AC_MSG_RESULT([yes, $SINFG_VERSION])
			SINFG_CXXFLAGS="`$SINFG_CONFIG --cxxflags`"
			SINFG_CFLAGS="`$SINFG_CONFIG --cflags`"
			SINFG_LIBS="`$SINFG_CONFIG --libs`"
			CXXFLAGS="$CXXFLAGS $SINFG_CXXFLAGS"
			AC_SUBST(SINFG_CXXFLAGS)
			AC_SUBST(SINFG_LIBS)
			AC_SUBST(SINFG_CFLAGS)
			$1
		else
			AC_MSG_RESULT(no)
			no_SINFG_config="yes"
			$2
		fi
	fi

	SINFG_DEPS($1,$2)
])


