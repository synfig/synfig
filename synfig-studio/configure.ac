# $Id$

# -- I N I T --------------------------------------------------

AC_PREREQ([2.60])
AC_INIT([Synfig Studio],[1.3.11],[https://github.com/synfig/synfig/issues],[synfigstudio])
AC_REVISION()

AM_CONDITIONAL(DEVELOPMENT_SNAPSHOT, true)

AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE([subdir-objects])
AM_MAINTAINER_MODE

LT_INIT([dlopen, win32-dll, disable-static])
AC_SUBST(LIBTOOL_DEPS)

AC_CONFIG_MACRO_DIRS([m4])

# -- V A R I A B L E S ----------------------------------------

API_VERSION=1.0

imageext=png
imagedir="${datadir}/pixmaps/synfigstudio"
plugindir="${datadir}/synfig/plugins"
sounddir="${datadir}/synfig/sounds"
uidir="${datadir}/synfig/ui"

GETTEXT_PACKAGE=synfigstudio
LOCALEDIR=[${prefix}/${DATADIRNAME}/locale]

# -- P R O G R A M S ------------------------------------------

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_CXXCPP
AC_PROG_INSTALL
AC_LANG([C++])
AX_CXX_COMPILE_STDCXX([11], [noext])

AC_ARG_OPTIMIZATION
AC_ARG_DEBUG

if test "x$debug" == "xyes"; then
    AC_DEFINE(SHOW_EXTRA_INFO,[],[Show extra info in the about dialog])
fi

AC_ARG_WARNINGS
AC_ARG_PROFILE_ARCS
AC_ARG_BRANCH_PROBABILITIES
AC_ARG_PROFILING

AC_ARG_ENABLE(g5opt,[
  --enable-g5opt           enable optimizations specific to G5 proc],[
    G5OPTFLAGS="-mpowerpc-gpopt -mno-multiple -mno-update -mcpu=970  -mtune=970 -falign-loops=16 -falign-functions=16"
],
[
    G5OPTFLAGS=
])

AC_WIN32_QUIRKS

## AM_GLIB_GNU_GETTEXT([external])
IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"],
                     [The domain to use with gettext])
AC_SUBST(LOCALEDIR)
AM_GNU_GETTEXT([external])
# This is here so autoreconf will run autopoint
AM_GNU_GETTEXT_VERSION([0.15])

case "$host" in
    *mingw* | *cygwin*)
        AM_CONDITIONAL(WIN32_PKG, true)
        AM_CONDITIONAL(MACOSX_PKG, false)
    ;;
    *darwin*)
        AM_CONDITIONAL(WIN32_PKG, false)
        AM_CONDITIONAL(MACOSX_PKG, true)
    ;;
    *)
        AM_CONDITIONAL(WIN32_PKG, false)
        AM_CONDITIONAL(MACOSX_PKG, false)
    ;;
esac

# -- L I B R A R I E S ----------------------------------------

# Gtk compatibility hacks:
# Since 3.14 some our hacks for GUI decorations works in other way
# Since 3.14 for checkboxes STATE_FLAG_ACTIVE replaced with STATE_FLAG_CHECKED
# Since 3.18 changed bahavor of Gtk::Adjustment (in new version signals emits automatically)
# Since 3.20 Gdk::Seat has been introduced that supersedes Gdk::DeviceManager
# Since 3.22 To fix artefacts with widgets placement we use new CSS fields: min-width, min-height
PKG_CHECK_MODULES(GTKMM, gtkmm-3.0,,[
    AC_MSG_ERROR([ ** You need to install Gtkmm-3.0 ])
])
AC_SUBST(GTKMM_CFLAGS)
AC_SUBST(GTKMM_LIBS)

PKG_CHECK_MODULES(SYNFIG, [synfig >= 1.3.11] [ETL >= 1.3.11] sigc++-2.0,,[
    AC_MSG_ERROR([ ** Unable to set up dependent libraries (synfig >= 1.3.11, ETL >= 1.3.11, sigc++-2.0) ])
])
AC_SUBST(SYNFIG_CFLAGS)
AC_SUBST(SYNFIG_LIBS)

AC_ARG_ENABLE([jack],
    AS_HELP_STRING([--enable-jack],
           [ Enable experimental JACK transport support experimental ]),
    [],
    [enable_jack=auto]
)

if test "x$enable_jack" != "xno"; then
    PKG_CHECK_MODULES(JACK,
        [jack],[
            AC_SUBST(JACK_CFLAGS)
            AC_SUBST(JACK_LIBS)
            AC_DEFINE(WITH_JACK,[],[JACK support enabled.])
            AC_MSG_RESULT([yes])
            AC_MSG_RESULT([ ** JACK support enabled.])
            enable_jack=yes
        ],[
            if test "x$enable_jack" == "xyes"; then
                AC_MSG_RESULT([no])
                AC_MSG_ERROR([ ** Please install JACK to enable JACK support ])
            else
                AC_MSG_RESULT([no])
                enable_jack=no
            fi
        ]
    )
fi

AC_ARG_WITH(userdir,[
  --with-userdir=(dirname)         Relative path to user directory],[
    AC_DEFINE_UNQUOTED(SYNFIG_USER_APP_DIR,"$with_userdir",[Synfig User Data Directory])
],[])

# -- H E A D E R S --------------------------------------------

# Headers for gui/ipc.cpp and gui/app.cpp
AC_CHECK_HEADERS([unistd.h fcntl.h])
AC_CHECK_HEADERS([sys/types.h sys/stat.h sys/errno.h])

# -- T Y P E S & S T R U C T S --------------------------------

# -- F U N C T I O N S ----------------------------------------

# -- A R G U M E N T S ----------------------------------------

# -- O U T P U T ----------------------------------------------

AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(LDFLAGS)

AC_SUBST(API_VERSION)

AC_SUBST(imageext)
AC_SUBST(imagedir)
AC_SUBST(plugindir)
AC_SUBST(sounddir)
AC_SUBST(uidir)

AC_PATH_PROG(UPDATE_MIME_DATABASE, update-mime-database, no)
AC_ARG_ENABLE(update-mimedb,
   AS_HELP_STRING([--disable-update-mimedb],
                   [disable the update-mime-database after install [default=no]]),,
    enable_update_mimedb=yes)
AM_CONDITIONAL(ENABLE_UPDATE_MIMEDB, (test x$enable_update_mimedb = xyes) && (test x$UPDATE_MIME_DATABASE != xno))

AC_ARG_WITH([images],
    AS_HELP_STRING([--without-images], [skip building images]),
    [],
    [with_images=yes]
)

AM_CONDITIONAL([WITH_IMAGES], [test "x$with_images" != "xno"])

AC_CONFIG_FILES([
    Makefile
    build_tools/Makefile
    doxygen.cfg
    po/Makefile.in
    synfigstudio.mime
    synfigstudio.keys
    org.synfig.SynfigStudio.xml
    synfigstudio-thumbnailer.schemas
    brushes/Makefile
    brushes/CD_concept/Makefile
    src/Makefile
    src/gui/Makefile
    src/synfigapp/Makefile
    images/Makefile
    pkg-info/macosx/synfig-studio.info
    plugins/Makefile
    plugins/add-skeleton-simple/Makefile
    plugins/view-unhide-all-layers/Makefile
    plugins/lottie-exporter/Makefile
    plugins/lottie-exporter/common/Makefile
    plugins/lottie-exporter/effects/Makefile
    plugins/lottie-exporter/helpers/Makefile
    plugins/lottie-exporter/layers/Makefile
    plugins/lottie-exporter/properties/Makefile
    plugins/lottie-exporter/properties/shapePropKeyframe/Makefile
    plugins/lottie-exporter/shapes/Makefile
    plugins/lottie-exporter/sources/Makefile
    plugins/lottie-exporter/synfig/Makefile
    sounds/Makefile
    ui/Makefile
])
AC_OUTPUT

# -- S U M M A R Y --------------------------------------------

echo "
$PACKAGE_NAME v.$VERSION
Configuration Summary
- - - - - - - - - - - - - - - - - -

Install Prefix -------------------> $prefix
Host Platform --------------------> $host
Icon Format ----------------------> $imageext
Debug Mode -----------------------> $debug
Profiling Mode -------------------> $profiling
Optimizations --------------------> $optimization
JACK Enabled ---------------------> $enable_jack
Build images ---------------------> $with_images

"'$'"CXX ------------------------------> '$CXX'
"'$'"CXXFLAGS -------------------------> '$CXXFLAGS'
"'$'"LDFLAGS --------------------------> '$LDFLAGS'
"'$'"SYNFIG_CFLAGS --------------------> '$SYNFIG_CFLAGS'
"'$'"SYNFIG_LIBS ----------------------> '$SYNFIG_LIBS'
"'$'"GTKMM_CFLAGS ---------------------> '$GTKMM_CFLAGS'
"'$'"GTKMM_LIBS -----------------------> '$GTKMM_LIBS'
"'$'"JACK_CFLAGS ----------------------> '$JACK_CFLAGS'
"'$'"JACK_LIBS ------------------------> '$JACK_LIBS'
"
