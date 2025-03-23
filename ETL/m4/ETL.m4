# ETL M4 Macro
# For GNU Autotools
#
# By Robert B. Quattlebaum Jr. <darco@users.sf.net>
#

AC_DEFUN([USING_ETL],
[
	AC_ARG_WITH(ETL-includes,
	[  --with-ETL-includes    Specify location of ETL headers],[
	CXXFLAGS="$CXXFLAGS -I$withval"
	])

	AC_PATH_PROG(ETL_CONFIG,ETL-config,no)

	if test "$ETL_CONFIG" = "no"; then
		no_ETL_config="yes"
		$2
	else
		AC_MSG_CHECKING([if $ETL_CONFIG works])
		if $ETL_CONFIG --libs >/dev/null 2>&1; then
			ETL_VERSION="`$ETL_CONFIG --version`"
			AC_MSG_RESULT([yes, $ETL_VERSION])
			CXXFLAGS="$CXXFLAGS `$ETL_CONFIG --cxxflags`"
			$1
		else
			AC_MSG_RESULT(no)
			no_ETL_config="yes"
			$2
		fi
	fi
])


