# $Id$

# -- I N I T --------------------------------------------------

AC_INIT([Extended Template Library],[0.04.12],[http://synfig.org/Bugs],[ETL])
AC_REVISION

AC_CONFIG_AUX_DIR(config)
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([ no-define ])
AM_MAINTAINER_MODE

# -- V A R I A B L E S ----------------------------------------

debug="no"

SVN_REPOSITORY=https://synfig.svn.sourceforge.net/svnroot/synfig/ETL
AC_SUBST(SVN_REPOSITORY)

# -- A R G U M E N T S ----------------------------------------


# -- P R O G R A M S ------------------------------------------

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_RANLIB

AC_LANG([C++])

AC_ARG_WARNINGS
AC_ARG_DEBUG
AC_ARG_OPTIMIZATION
AC_ARG_PROFILING

AC_WIN32_QUIRKS

# -- L I B R A R I E S ----------------------------------------

AC_CHECK_LIB([user32], [main])
AC_CHECK_LIB([kernel32], [CreateMutex])
AC_CHECK_LIB([pthread], [pthread_mutex_init], , [HAVE_LIBPTHREAD=no])

if test x$HAVE_LIBPTHREAD = xno ; then
        D_LIBPTHREAD="#undef"
else
        D_LIBPTHREAD="#define"
fi

# -- H E A D E R S --------------------------------------------

AC_HEADER_STDC
AC_CHECK_HEADERS([pthread.h], [D_PTHREAD_H="#define"], [D_PTHREAD_H="#undef"])
AC_CHECK_HEADERS([sched.h], [D_SCHED_H="#define"], [D_SCHED_H="#undef"])
AC_CHECK_HEADERS([sys/time.h])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([windows.h])

# -- T Y P E S & S T R U C T S --------------------------------

# -- F U N C T I O N S ----------------------------------------

AC_CHECK_FUNCS([pthread_create], [D_PTHREAD_CREATE="#define"], [D_PTHREAD_CREATE="#undef"])
AC_CHECK_FUNCS([pthread_rwlock_init], [D_PTHREAD_RWLOCK_INIT="#define"], [D_PTHREAD_RWLOCK_INIT="#undef"])
AC_CHECK_FUNCS([sched_yield], [D_SCHED_YIELD="#define"], [D_SCHED_YIELD="#undef"])
AC_CHECK_FUNCS([CreateThread], [D_CREATETHREAD="#define"], [D_CREATETHREAD="#undef"])
AC_CHECK_FUNCS([QueryPerformanceCounter], [D_QUERYPERFORMANCECOUNTER="#define"], [D_QUERYPERFORMANCECOUNTER="#undef"])
AC_CHECK_FUNCS([gettimeofday], [D_GETTIMEOFDAY="#define"], [D_GETTIMEOFDAY="#undef"])
AC_CHECK_FUNCS([sscanf], [D_SSCANF="#define"], [D_SSCANF="#undef"])
AC_CHECK_FUNCS([vsscanf], [D_VSSCANF="#define"], [D_VSSCANF="#undef"])
AC_CHECK_FUNCS([vsprintf], [D_VSPRINTF="#define"], [D_VSPRINTF="#undef"])
AC_CHECK_FUNCS([vasprintf], [D_VASPRINTF="#define"], [D_VASPRINTF="#undef"])
AC_CHECK_FUNCS([vsnprintf], [D_VSNPRINTF="#define"], [D_VSNPRINTF="#undef"])
AC_CHECK_FUNCS([_vsnprintf], [D__VSNPRINTF="#define"], [D__VSNPRINTF="#undef"])

# -- O U T P U T ----------------------------------------------

AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)

# Libraries
AC_SUBST(D_LIBPTHREAD)

# Headers
AC_SUBST(D_PTHREAD_H)
AC_SUBST(D_SCHED_H)

# Functions
AC_SUBST(D_PTHREAD_CREATE)
AC_SUBST(D_PTHREAD_RWLOCK_INIT)
AC_SUBST(D_SCHED_YIELD)
AC_SUBST(D_CREATETHREAD)
AC_SUBST(D_QUERYPERFORMANCECOUNTER)
AC_SUBST(D_GETTIMEOFDAY)
AC_SUBST(D_SSCANF)
AC_SUBST(D_VSSCANF)
AC_SUBST(D_VSPRINTF)
AC_SUBST(D_VASPRINTF)
AC_SUBST(D_VSNPRINTF)
AC_SUBST(D__VSNPRINTF)

AC_CONFIG_FILES([ETL-config
ETL/etl_profile.h
Makefile
ETL/Makefile
test/Makefile
ETL.pc
doxygen.cfg
])
AC_OUTPUT

# -- S U M M A R Y --------------------------------------------

echo "
$PACKAGE_NAME v.$VERSION
Configuration Summary
- - - - - -

Install Prefix -------------------> $prefix
Target Platform ------------------> $host
Debug Mode -----------------------> $debug

"'$'"CXX ------------------------------> '$CXX'
"'$'"CXXFLAGS -------------------------> '$CXXFLAGS'
"
