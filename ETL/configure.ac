# $Id$

# -- I N I T --------------------------------------------------
AC_PREREQ(2.60)
AC_INIT([Extended Template Library],[1.5.1],[https://github.com/synfig/synfig/issues],[ETL])
AC_REVISION

AM_SILENT_RULES([yes])

AC_CONFIG_AUX_DIR(config)
AC_CONFIG_HEADERS([ETL/etl_profile_.h])
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([ no-define ])
AM_MAINTAINER_MODE

# -- V A R I A B L E S ----------------------------------------

debug="no"

# -- A R G U M E N T S ----------------------------------------


# -- P R O G R A M S ------------------------------------------
AC_LANG([C++])
AC_REQUIRE_CPP
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_INSTALL
AC_PROG_RANLIB
AX_CXX_COMPILE_STDCXX([11], [noext])

# that is not working, btw
test "x$CXX" == "x" && AC_MSG_ERROR([Required program 'c++' not found.])


AC_ARG_WARNINGS
AC_ARG_OPTIMIZATION
AC_ARG_DEBUG
AC_ARG_PROFILING

AC_WIN32_QUIRKS

# -- L I B R A R I E S ----------------------------------------

# -- H E A D E R S --------------------------------------------

AH_TOP([
// We want to be autoconf/autoheader friendly, so
// if the developer has already included an
// autoheader-generated configuration script,
// we can assume that the user knew what they
// were doing and let them use their own config.h.
//#ifndef PACKAGE_TARNAME
])

AH_BOTTOM([
//#endif // ifndef PACKAGE_TARNAME
])


# -- T Y P E S & S T R U C T S --------------------------------

# -- F U N C T I O N S ----------------------------------------

ETL_DEPS

# -- O U T P U T ----------------------------------------------

PKG_CHECK_MODULES(GLIBMM, [glibmm-2.4 >= 2.24.2],[
	CONFIG_DEPS="$CONFIG_DEPS glibmm-2.4"
],[
	AC_MSG_ERROR([ ** You need to install glibmm-2.4 version 2.24.2 or higher.])
])

etlincludedir=$includedir/ETL
AC_SUBST(etlincludedir)

AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)

AC_CONFIG_FILES([ETL-config
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
- - - - - - - - - - - - - - - - - - - - - - - - -

Install Prefix -------------------> $prefix
Target Platform ------------------> $host
Debug Mode -----------------------> $debug

"'$'"CXX ------------------------------> '$CXX'
"'$'"CXXFLAGS -------------------------> '$CXXFLAGS'
"
