# ETL M4 Macro
# For GNU Autotools
# $Id$
#
# By Robert B. Quattlebaum Jr. <darco@users.sf.net>
#

AC_DEFUN([ETL_DEPS],
[
	AC_C_BIGENDIAN
	
	AC_CHECK_LIB(user32, main)
	AC_CHECK_LIB([kernel32], [CreateMutex])
	AC_CHECK_LIB([pthread], [pthread_mutex_init])
		
	AC_HEADER_STDC
	
	AC_CHECK_HEADERS(pthread.h)
	AC_CHECK_HEADERS(sched.h)
	AC_CHECK_HEADERS(sys/times.h)
	AC_CHECK_HEADERS(sys/time.h)
	AC_CHECK_HEADERS(unistd.h)
	AC_CHECK_HEADERS(windows.h)
	AC_CHECK_FUNCS([pthread_create])
	AC_CHECK_FUNCS([pthread_rwlock_init])
	AC_CHECK_FUNCS([pthread_yield])
	AC_CHECK_FUNCS([sched_yield])
	AC_CHECK_FUNCS([CreateThread])
	AC_CHECK_FUNCS([__clone])
	AC_CHECK_FUNCS([QueryPerformanceCounter])
	
	AC_CHECK_FUNCS([gettimeofday])
	AC_CHECK_FUNCS([vsscanf])
	AC_CHECK_FUNCS([vsprintf])
	AC_CHECK_FUNCS([vasprintf])
	AC_CHECK_FUNCS([vsnprintf],[],[
		AC_CHECK_FUNC([_vsnprintf],[
			AC_DEFINE(vsnprintf,_vsnprintf,[define if the vsnprintf function is mangled])
			AC_DEFINE(HAVE_VSNPRINTF,1)
		])
	])
	
	$1
])

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

	ETL_DEPS($1,$2)
])


