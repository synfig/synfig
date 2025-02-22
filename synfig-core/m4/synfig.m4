# SYNFIG M4 Macro
# For GNU Autotools
#
# By Robert B. Quattlebaum Jr. <darco@users.sf.net>
#	AM_LIBXMLPP(,$2)


AC_DEFUN([SYNFIG_DEPS],
[
	AM_PATH_XML2(,,$2)
	AC_CHECK_FUNCS([floor pow sqrt],,$2)
	AM_LIBXMLPP(,$2)

	CXXFLAGS="$CXXFLAGS $LIBXMLPP_CFLAGS"
	LIBS="$LIBS $LIBXMLPP_LIBS"

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
			CXXFLAGS="$CXXFLAGS `$SYNFIG_CONFIG --cxxflags`"
			$1
		else
			AC_MSG_RESULT(no)
			no_SYNFIG_config="yes"
			$2
		fi
	fi

	SYNFIG_DEPS($1,$2)
])


