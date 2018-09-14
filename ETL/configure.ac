# $Id$

# -- I N I T --------------------------------------------------
AC_PREREQ(2.59)
AC_INIT([Extended Template Library],[1.2.2],[https://github.com/synfig/synfig/issues],[ETL])
AC_REVISION

AC_CONFIG_AUX_DIR(config)
AC_CONFIG_HEADERS([ETL/etl_profile_.h])
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
AC_LANG([C++])
AC_REQUIRE_CPP
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_INSTALL
AC_PROG_RANLIB

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
- - - - - -

Install Prefix -------------------> $prefix
Target Platform ------------------> $host
Debug Mode -----------------------> $debug

"'$'"CXX ------------------------------> '$CXX'
"'$'"CXXFLAGS -------------------------> '$CXXFLAGS'
"
