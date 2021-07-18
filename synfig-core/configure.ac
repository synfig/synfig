# $Id$

# -- I N I T --------------------------------------------------

AC_PREREQ([2.60])
AC_INIT([Synfig Core],[1.5.0],[https://github.com/synfig/synfig/issues],[synfig])
AC_REVISION()

AM_SILENT_RULES([yes])

AC_CONFIG_AUX_DIR(config)
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE([nostdinc subdir-objects])
AM_MAINTAINER_MODE

LT_CONFIG_LTDL_DIR([libltdl])

LT_INIT([dlopen, win32-dll, disable-static])
AC_SUBST(LIBTOOL_DEPS)

LTDL_INIT([convenience])

AC_DEFINE(LT_SCOPE,[extern],[LibLTDL is linked statically])

# -- V A R I A B L E S ----------------------------------------

API_VERSION=1.0
MODULE_DIR='${libdir}/synfig/modules'
SYNFIGLIB_DIR=${libdir}/synfig

# -- P R O G R A M S ------------------------------------------

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_CXXCPP
AC_PROG_INSTALL
AX_CXX_COMPILE_STDCXX([11], [noext])

AC_LANG([C++])

AC_C_BIGENDIAN


# -- A R G U M E N T S ----------------------------------------

AC_ARG_OPTIMIZATION
AC_ARG_DEBUG
AC_ARG_WARNINGS
AC_ARG_PROFILING
AC_ARG_PROFILE_ARCS
AC_ARG_BRANCH_PROBABILITIES

AC_ARG_ENABLE(g5opt,
	AS_HELP_STRING(--enable-g5opt, [enable optimizations specific to G5 proc]),[
	CXXFLAGS="$CXXFLAGS -mtune=G5 -falign-loops=32"
	CFLAGS="$CFLAGS -mtune=G5"
],
[
	true
])

AC_WIN32_QUIRKS

dnl
dnl dynamic linker
dnl
AC_CHECK_LIB(c, dlopen,
	DYNAMIC_LD_LIBS="",
	AC_CHECK_LIB(
		dl,
		dlopen,
		DYNAMIC_LD_LIBS="-ldl",
		DYNAMIC_LD_LIBS=""
	)
)

AC_SUBST(DYNAMIC_LD_LIBS)

AC_ARG_ENABLE(layer-profiling,
	AS_HELP_STRING(--enable-layer-profiling, [enable layer profiling]),[
	use_layerprofiling=$enableval
],
[
	use_layerprofiling="no"
])
if test $use_layerprofiling = "yes" ; then {
	AC_DEFINE(SYNFIG_PROFILE_LAYERS,[1],[enable layer profiling])
} ; fi




AC_ARG_WITH(imagemagick,
	AS_HELP_STRING(--without-imagemagick, [do not support ImageMagick]),[
],[
	AC_CHECK_PROG([imagemagick_convert],[convert],[yes],[no])
	with_imagemagick=$imagemagick_convert
])
AM_CONDITIONAL(WITH_IMAGEMAGICK, test $with_imagemagick = yes)





AC_ARG_WITH(magickpp,
	AS_HELP_STRING(--without-magickpp, [do not support Magick++]),[
],[
	with_magickpp="yes"
])
AM_CONDITIONAL(HAVE_LIBMAGICKPP,false)
if test $with_magickpp = "yes" ; then {
	PKG_CHECK_MODULES(MAGICKPP, Magick++ >= 6.4.2,[
		with_magickpp="yes"
		AM_CONDITIONAL(HAVE_LIBMAGICKPP,true)
		CONFIG_DEPS="$CONFIG_DEPS Magick++"
	],[
		with_magickpp="no"
		AC_MSG_ERROR([ *** Unable to find Magick++])
	])
} ; fi




AC_ARG_WITH(ffmpeg,
	AS_HELP_STRING(--without-ffmpeg, [do not support FFMPEG]),[
],[
	with_ffmpeg="yes"
])
AM_CONDITIONAL(WITH_FFMPEG, test $with_ffmpeg = yes)


AC_ARG_WITH(vimage,
	AS_HELP_STRING(--with-vimage, [support Apple vImage]),[
],[
	with_vimage="no"
])
if test $with_vimage = "no" ; then {
	AM_CONDITIONAL(WITH_VIMAGE,false)
} else {
	AM_CONDITIONAL(WITH_VIMAGE,true)
	AC_DEFINE(HAS_VIMAGE,[1],[enable apple vImage])
	VIMAGE_LIBS="-Wc,-framework -Wc,Accelerate"

} ; fi



AC_ARG_WITH(libdv,
	AS_HELP_STRING(--without-libdv, [do not support libdv]),[
],[
	with_libdv="yes"
])
AM_CONDITIONAL(WITH_LIBDV, test $with_libdv = yes)




# LIBAVCODEC CHECK--------------------

AC_ARG_WITH(libavcodec,
	AS_HELP_STRING(--without-libavcodec, [do not support libavcodec]),[
],[
	with_libavcodec="yes"
])

if test $with_libavcodec != "no" ; then {
	PKG_CHECK_MODULES(LIBAVCODEC, [libavcodec >= 57.0 libavformat >= 57.0 libavutil >= 55.0 libswscale >= 4.0],
		[CONFIG_DEPS="$CONFIG_DEPS libavcodec libavformat libswscale"],
		[echo no; with_libavcodec="no"])
} ; fi
if test $with_libavcodec = "yes" ; then {
	AC_DEFINE(WITH_LIBAVCODEC,[],[enable libavcodec support])
	AM_CONDITIONAL(WITH_LIBAVCODEC,true)
} else {
	AM_CONDITIONAL(WITH_LIBAVCODEC,false)
} ; fi

if test $with_libavcodec = "yes" ; then {
	# save flags
	ac_save_CPPFLAGS="$CPPFLAGS"
	ac_save_CFLAGS="$CFLAGS"

	# Need to check both the new and old locations for the avformat header
	CPPFLAGS="${ac_save_CPPFLAGS} ${LIBAVCODEC_CFLAGS}"
	CFLAGS="${ac_save_CFLAGS} ${LIBAVCODEC_CFLAGS}"
	AC_CHECK_HEADERS(libavformat/avformat.h avformat.h ffmpeg/avformat.h)

	# Need to check both the new and old locations for the swscale header
	CPPFLAGS="${ac_save_CPPFLAGS} ${LIBSWSCALE_CFLAGS}"
	CFLAGS="${ac_save_CFLAGS} ${LIBSWSCALE_CFLAGS}"
	AC_CHECK_HEADERS(libswscale/swscale.h swscale.h ffmpeg/swscale.h)

	# restore flags
	CPPFLAGS="${ac_save_CPPFLAGS}"
	CFLAGS="${ac_save_CFLAGS}"
} ; fi


# FREETYPE2 CHECK--------------------

AC_ARG_WITH(freetype,
	AS_HELP_STRING(--without-freetype, [do not support FreeType text rendering (default=auto)]),[
],[
	with_freetype="yes"
])

if test $with_freetype != "no" ; then {
	PKG_CHECK_MODULES(FREETYPE, freetype2,[
		with_freetype="yes"
		CONFIG_DEPS="$CONFIG_DEPS freetype2"
	],[
		PKG_CHECK_MODULES(FREETYPE, xft,[
			with_freetype="yes"
		],[
			with_freetype="no"
		])
	])
} ; fi
AM_CONDITIONAL(WITH_FREETYPE, test $with_freetype = yes)


# FONTCONFIG CHECK--------------------

AC_ARG_WITH(fontconfig,
	AS_HELP_STRING(--without-fontconfig, [do not support fontconfig (default=auto)]),[
],[
	with_fontconfig="yes"
])

if test $with_fontconfig != "no" ; then {
	PKG_CHECK_MODULES(FONTCONFIG, fontconfig,[
		with_fontconfig="yes"
		CONFIG_DEPS="$CONFIG_DEPS fontconfig"
	],[
		with_fontconfig="no"
	])
} ; fi

if test $with_fontconfig = "yes" ; then {
	AC_DEFINE(WITH_FONTCONFIG,[],[enable fontconfig support])
	AM_CONDITIONAL(WITH_FONTCONFIG,true)
} else {
	AM_CONDITIONAL(WITH_FONTCONFIG,false)
} ; fi


# FRIBIDI CHECK--------------------

AC_ARG_WITH([fribidi],
	[AS_HELP_STRING([--with-fribidi], [support bidirectional text rendering @<:@default=check@:>@])],
	[],
	[with_fribidi=check]
)
AS_IF([test "x$with_fribidi" = xyes],
	[
		if test "x$with_freetype" != xyes; then
			AC_MSG_FAILURE(
				[--with-fribidi was given, but it only makes sense if --with-freetype too])
		fi
	]
)

AS_IF([test "x$with_fribidi" != xno],
	[PKG_CHECK_MODULES(FRIBIDI, fribidi,
		[
			with_fribidi="yes"
			AC_DEFINE(HAVE_FRIBIDI, [1],[Enable fribidi support])
		],[
			if test "x$with_fribidi" != xcheck; then
				AC_MSG_FAILURE(
					[--with-fribidi was given, but it couldn't be found])
			fi
		]
	)]
)

AM_CONDITIONAL(WITH_FRIBIDI, test $with_fribidi = yes)

# OPENEXR CHECK------------------------
AC_ARG_WITH(openexr,
	AS_HELP_STRING(--without-openexr, [do not support OpenEXR]),[
],[
	with_openexr="yes"
])
if test $with_openexr = "yes" ; then {
	PKG_CHECK_MODULES(OPENEXR, OpenEXR,[
		CONFIG_DEPS="$CONFIG_DEPS OpenEXR"
	],
	[
		with_openexr="no"
	])
} ; fi
AM_CONDITIONAL(WITH_OPENEXR, test $with_openexr = yes)


# JPEG CHECK (commandline switch) ------------------------
AC_ARG_WITH(jpeg,
	AS_HELP_STRING(--without-jpeg, [do not support JPEG]),[
],[
	with_jpeg="yes"
])
AM_CONDITIONAL(WITH_JPEG, test $with_jpeg = yes)





# -- L I B R A R I E S ----------------------------------------

AC_LIB_LTDL

IT_PROG_INTLTOOL([0.35.0])

GETTEXT_PACKAGE=synfig
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"],
                     [The domain to use with gettext])

localedir="$datadir/locale"
AC_SUBST(localedir)

AM_GNU_GETTEXT([external])
# This is here so autoreconf will run autopoint
AM_GNU_GETTEXT_VERSION([0.15])

PKG_CHECK_MODULES(ETL, [ETL >= 1.5.0],,[
	AC_MSG_ERROR([ ** You need to install the ETL (version 1.5.0 or greater).])
])
CONFIG_DEPS="$CONFIG_DEPS ETL"

AC_CHECK_HEADER([zlib.h],[
	LIBZ_LIBS="-lz"
	AC_SUBST(LIBZ_LIBS)
],[
	AC_MSG_ERROR([ ** You need to install zlib])
])


PKG_CHECK_MODULES(GLIBMM, [glibmm-2.4 >= 2.24.2],[
	CONFIG_DEPS="$CONFIG_DEPS glibmm-2.4"
],[
	AC_MSG_ERROR([ ** You need to install glibmm-2.4 version 2.24.2 or higher.])
])

PKG_CHECK_MODULES(GIOMM, [giomm-2.4 >= 2.18.2],[
	CONFIG_DEPS="$CONFIG_DEPS giomm-2.4"
],[
	AC_MSG_ERROR([ ** You need to install giomm-2.4 version 2.18.2 or higher.])
])

PKG_CHECK_MODULES(LIBXMLPP, libxml++-2.6,[
	CONFIG_DEPS="$CONFIG_DEPS libxml++-2.6"
],[
	PKG_CHECK_MODULES(LIBXMLPP, libxml++-1.0,[
		CONFIG_DEPS="$CONFIG_DEPS libxml++-1.0"
	],[
		AC_MSG_ERROR([ ** You need to install libxml++, either version 2.6 or 1.0.])
	])
])

PKG_CHECK_MODULES(LIBSIGC, [sigc++-2.0],,[
	AC_MSG_ERROR([ ** libsigc++-2.0 not found. It is required. You should really install it.])
])
CONFIG_DEPS="$CONFIG_DEPS sigc++-2.0"

AC_MSG_CHECKING([for OpenGL])
AC_ARG_WITH(opengl,
	AS_HELP_STRING(--with-opengl, [enable OpenGL (default=no)]),[
],[
	with_opengl="no"
])
if test $with_opengl != "no" ; then {
	case "$host" in
		*darwin*)
			AC_MSG_RESULT([ ** OpenGL is not available for OSX yet. Disabled.])
			with_opengl="no"
		;;
		*)
			PKG_CHECK_MODULES(LIBGL, gl, [
				CONFIG_DEPS="$CONFIG_DEPS gl"
				LIBGL_CFLAGS="$LIBGL_CFLAGS -DGL_GLEXT_PROTOTYPES"
				CONFIG_CFLAGS="$CONFIG_CFLAGS -DWITH_OPENGL"
				with_opengl="yes"
			],[
				AC_MSG_RESULT([ ** Cannot find gl.pc. OpenGL disabled.])
				with_opengl="no"
			])
		;;
	esac
} ; fi
AM_CONDITIONAL(WITH_OPENGL, test $with_opengl = yes)

AC_ARG_WITH(opencl,
	AS_HELP_STRING(--with-opencl, [enable OpenCL (default=auto)]),[
],[
	with_opencl="no"
])

if test $with_opencl != "no" ; then {
	AC_CHECK_HEADER(CL/opencl.h,,AC_MSG_ERROR([ ** You need to install opencl-headers package.]))
	PKG_CHECK_MODULES(LIBCL, OpenCL,[
		CONFIG_DEPS="$CONFIG_DEPS OpenCL"
		with_opencl="yes"
	],[
		AC_MSG_ERROR([ ** You need to install OpenCL.])
		with_opencl="no"
	])
	
} ; fi
AM_CONDITIONAL(WITH_OPENCL, test $with_opencl = yes)

PKG_CHECK_MODULES(LIBFFTW, fftw3,,[
	AC_MSG_ERROR([ ** You need to install FFTW 3.])
])
CONFIG_DEPS="$CONFIG_DEPS fftw3"

PKG_CHECK_MODULES(MLTPP, mlt++,,[
	AC_MSG_ERROR([ ** You need to install mlt++.])
])
CONFIG_DEPS="$CONFIG_DEPS mlt++"

AX_BOOST_BASE([1.53.0],,AC_MSG_ERROR([ ** You need to install Boost >= 1.53.0]))
#AX_BOOST_SYSTEM
#if test "x$ax_cv_boost_system" != "xyes" ; then
#	AC_MSG_ERROR([ ** You need to install Boost system library])
#fi
# Boost program options and system libraries are just dependencies
# of the synfig cli and therefore
# not required to link against the synfig library.


TARGET_PNG="yes"
TARGET_MNG="yes"

AC_CHECK_LIB(png16, png_write_row,[
	PNG_LIBS="-lpng16"
	AC_DEFINE(HAVE_LIBPNG,[], [ Define if PNG library is available ])
	AC_SUBST(PNG_LIBS)
	AM_CONDITIONAL(HAVE_LIBPNG,true)
],[
	AC_CHECK_LIB(png14, png_write_row,[
		PNG_LIBS="-lpng14"
		AC_DEFINE(HAVE_LIBPNG,[], [ Define if PNG library is available ])
		AC_SUBST(PNG_LIBS)
		AM_CONDITIONAL(HAVE_LIBPNG,true)
	],[
		AC_CHECK_LIB(png12, png_write_row,[
		PNG_LIBS="-lpng12"
		AC_DEFINE(HAVE_LIBPNG,[])
		AC_SUBST(PNG_LIBS)
		AM_CONDITIONAL(HAVE_LIBPNG,true)
		],[
			AC_CHECK_LIB(png, png_write_row,[
			PNG_LIBS="-lpng"
			AC_DEFINE(HAVE_LIBPNG,[])
			AC_SUBST(PNG_LIBS)
			AM_CONDITIONAL(HAVE_LIBPNG,true)
			],[
				AC_MSG_RESULT([ *** PNG Output target disabled])
				TARGET_PNG="no"
				AM_CONDITIONAL(HAVE_LIBPNG,false)
			],[-lz -L${exec_prefix}/lib])
		],[-lz -L${exec_prefix}/lib])
	],[-lz -L${exec_prefix}/lib])
],[-lz -L${exec_prefix}/lib])

AC_CHECK_LIB(mng, mng_initialize,[
	MNG_LIBS="-lmng -lz"
	AC_DEFINE(HAVE_LIBMNG,[], [ Define if MNG library is available ] )
	AC_SUBST(MNG_LIBS)
	AM_CONDITIONAL(HAVE_LIBMNG,true)
],[
	AC_MSG_RESULT([ *** MNG Output target disabled])
	TARGET_MNG="no"
	AM_CONDITIONAL(HAVE_LIBMNG,false)
],[-lz -L${exec_prefix}/lib])

# check for JPEG availability
if test $with_jpeg = "yes" ; then {
	AC_CHECK_LIB(jpeg, jpeg_CreateCompress,[
		JPEG_LIBS="-ljpeg"
		AC_DEFINE(HAVE_LIBJPEG,[], [ Define if JPEG library is available ] )
		AC_SUBST(JPEG_LIBS)
		TARGET_JPEG="yes"
		AM_CONDITIONAL(HAVE_LIBJPEG,true)
	],[
		AC_MSG_RESULT([ *** JPEG Output target disabled])
		TARGET_JPEG="no"
		AM_CONDITIONAL(HAVE_LIBJPEG,false)
	],[
		-L${exec_prefix}/lib
	])
} else {
	AC_MSG_RESULT([ *** JPEG Output target disabled])
	TARGET_JPEG="no"
	AM_CONDITIONAL(HAVE_LIBJPEG,false)
} ; fi


AC_SUBST(MODULE_DIR)
AC_SUBST(SYNFIGLIB_DIR)

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



# -- H E A D E R S --------------------------------------------

AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([signal.h termios.h process.h io.h fcntl.h])

# -- T Y P E S & S T R U C T S --------------------------------

# -- F U N C T I O N S ----------------------------------------

AC_CHECK_FUNCS([fork]) # used by mod_dv, mod_ffmpeg, mod_imagemagick and synfig-osx launcher
AC_CHECK_FUNCS([pipe])
AC_CHECK_FUNCS([waitpid])

# -- O U T P U T ----------------------------------------------

AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)

# AC_CONFIG_SUBDIRS(libltdl) is required for libtool 1.5.26 but must not be present for
# libtool 2.2.4. Problem seems to be that AC_LIB_LTDL (above) now calls AC_CONFIG_SUBDIRS
# itself, through LTDL_INIT, _LTDL_SETUP, _LTDL_MODE_DISPATCH. We need to check the
# condition that libltdl has already been registered with AC_CONFIG_SUBDIRS:
m4_ifdef([_AC_SEEN_TAG(libltdl)], [], [AC_CONFIG_SUBDIRS(libltdl)])

CONFIG_LIBS="-lsynfig"
CONFIG_CFLAGS="$CONFIG_CFLAGS"
AC_SUBST(CONFIG_LIBS)
AC_SUBST(CONFIG_CFLAGS)
AC_SUBST(CONFIG_DEPS)
AC_SUBST(ETL_CFLAGS)

SYNFIG_LIBS="$VIMAGE_LIBS $LIBZ_LIBS $GLIBMM_LIBS $GIOMM_LIBS $LIBXMLPP_LIBS $LIBGL_LIBS $LIBCL_LIBS $LIBFFTW_LIBS $MLTPP_LIBS $ETL_LIBS $LIBSIGC_LIBS $LTLIBINTL"
SYNFIG_CFLAGS="$BOOST_CPPFLAGS $LIBZ_CFLAGS $GLIBMM_CFLAGS $GIOMM_CFLAGS $LIBXMLPP_CFLAGS $LIBGL_CFLAGS $LIBCL_CFLAGS $LIBFFTW_CFLAGS $MLTPP_CFLAGS $ETL_CFLAGS $LIBSIGC_CFLAGS $CONFIG_CFLAGS -DSYNFIG_NO_DEPRECATED -DLOCALEDIR=\\\"$localedir\\\""

CONFIG_CFLAGS="`echo $CONFIG_CFLAGS | sed s/-mno-cygwin//g | sed s/-mwindows//g`"
SYNFIG_CFLAGS="`echo $SYNFIG_CFLAGS | sed s/-mno-cygwin//g | sed s/-mwindows//g`"
SYNFIG_LIBS="`echo $SYNFIG_LIBS | sed s/-mno-cygwin//g | sed s/-mwindows//g`"

AC_SUBST(SYNFIG_LIBS)
AC_SUBST(SYNFIG_CFLAGS)

AC_SUBST(API_VERSION)

synfigincludedir=$includedir/synfig-$API_VERSION
AC_SUBST(synfigincludedir)

AC_CONFIG_FILES([synfig-config
build_tools/Makefile
synfig.pc
doxygen.cfg
Makefile
po/Makefile.in
src/Makefile
src/synfig/Makefile
src/modules/Makefile
src/modules/lyr_freetype/Makefile
src/modules/lyr_std/Makefile
src/modules/mod_bmp/Makefile
src/modules/mod_dv/Makefile
src/modules/mod_ffmpeg/Makefile
src/modules/mod_filter/Makefile
src/modules/mod_geometry/Makefile
src/modules/mod_gif/Makefile
src/modules/mod_gradient/Makefile
src/modules/mod_imagemagick/Makefile
src/modules/mod_jpeg/Makefile
src/modules/mod_libavcodec/Makefile
src/modules/mod_magickpp/Makefile
src/modules/mod_mng/Makefile
src/modules/mod_noise/Makefile
src/modules/mod_openexr/Makefile
src/modules/mod_particle/Makefile
src/modules/mod_png/Makefile
src/modules/mod_ppm/Makefile
src/modules/mod_yuv420p/Makefile
src/modules/mod_svg/Makefile
src/modules/mod_example/Makefile
src/tool/Makefile
src/modules/synfig_modules.cfg
test/Makefile
examples/walk/Makefile
examples/Makefile
pkg-info/macosx/synfig-core.info
])
## src/modules/trgt_mpg/Makefile
## src/modules/mptr_mplayer/Makefile
AC_OUTPUT

# -- S U M M A R Y --------------------------------------------

echo "
$PACKAGE_NAME v.$VERSION
Configuration Summary
- - - - - - - - - - - - - - - - - -

Install Prefix -------------------> $prefix
Module directory -----------------> $MODULE_DIR
Build Platform -------------------> $build
Host Platform --------------------> $host
Arc Profiling --------------------> $profile_arcs
GProf Profiling ------------------> $profiling
Debug Mode -----------------------> $debug ($debug_flags)
Optimization ---------------------> $optimization
PNG output target support --------> $TARGET_PNG
MNG output target support --------> $TARGET_MNG
JPEG output target support -------> $TARGET_JPEG
ETL_CFLAGS -----------------------> $ETL_CFLAGS
FreeType2 ------------------------> $with_freetype
fontconfig -----------------------> $with_fontconfig
libavcodec -----------------------> $with_libavcodec
vImage ---------------------------> $with_vimage
ImageMagick ----------------------> $with_imagemagick
Magick++ -------------------------> $with_magickpp
FFMPEG ---------------------------> $with_ffmpeg
OpenGL ---------------------------> $with_opengl
OpenCL ---------------------------> $with_opencl
libdv ----------------------------> $with_libdv
OpenEXR --------------------------> $with_openexr

"'$'"CXX -----------------------------> '$CXX'
"'$'"CXXFLAGS ------------------------> '$CXXFLAGS'
"'$'"SYNFIG_LIBS ---------------------> '$SYNFIG_LIBS'
"'$'"LIBZ_LIBS -----------------------> '$LIBZ_LIBS'
"'$'"GLIBMM_LIBS ---------------------> '$GLIBMM_LIBS'
"'$'"GIOMM_LIBS ----------------------> '$GIOMM_LIBS'
"'$'"LIBXMLPP_LIBS -------------------> '$LIBXMLPP_LIBS'
"'$'"ETL_LIBS ------------------------> '$ETL_LIBS'
"'$'"LIBSIGC_LIBS --------------------> '$LIBSIGC_LIBS'
"'$'"SYNFIG_CFLAGS -------------------> '$SYNFIG_CFLAGS'
"'$'"BOOST_CPPFLAGS ------------------> '$BOOST_CPPFLAGS'
"'$'"BOOST_LDFLAGS -------------------> '$BOOST_LDFLAGS'
"'$'"BOOST_SYSTEM_LIB ----------------> '$BOOST_SYSTEM_LIB'
"
