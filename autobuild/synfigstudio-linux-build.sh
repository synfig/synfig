#!/bin/bash
#
# SynfigStudio build script
# Copyright (c) 2008-2010 Konstantin Dmitriev
#
# This package is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This package is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.

# = Usage: =
#    ./synfigstudio-linux-build.sh [mode] [revision]
#
# where:
#   - [mode] is full|quick|package
#   - [revision] - commit id, branch or tag (package mode only)
#
# To build packages it's required to run this script as root.
#
# = Examples: =
#
# == Standart mode ==
# Configure and (re)build synfigstudio into your homedir:
#    ./synfigstudio-linux-build.sh
# Configure and make clean build synfigstudio into your homedir:
#    ./synfigstudio-linux-build.sh full
# Quick rebuild of synfig (without configure) into your homedir:
#    ./synfigstudio-linux-build.sh quick
#
# == Package mode ==
# Build package from "master" branch:
#    ./synfigstudio-linux-build.sh package
# Build package from "genete_setup_dialog" branch
#    ./synfigstudio-linux-build.sh package origin/genete_setup_dialog
# Build package from commit with "synfigstudio-0.62.00" tag
#    ./synfigstudio-linux-build.sh package synfigstudio-0.62.00
#
# Note: Make sure to run "git clean -f -x -d" after you switch branches.
#
# = TODO =
# - debuginfo packages

RELEASE=8
if [ -z "$PREFIX" ]; then
PREFIX=$HOME/synfig/
fi

PACKAGES_PATH=$HOME/synfig-packages     	# path where to write packages files

if [ -z $BUILDROOT ]; then
PACKAGES_BUILDROOT=$HOME/synfig-buildroot	# path of for build infrastructure
else
PACKAGES_BUILDROOT=$BUILDROOT/synfig-buildroot
fi
if [ -d "$PACKAGES_BUILDROOT" ]; then
PACKAGES_BUILDROOT=`cd $PACKAGES_BUILDROOT; pwd`	# canonify buildroot path
fi

BUILDROOT_VERSION=9
BUILDROOT_LIBRARY_SET_ID=4
MAKE_THREADS=2					#count of threads for make

# full = clean, configure, make
# standart = configure, make
# quick = make
# package = chroot, clean, configure, make
MODE='standart'
OPENGL=0
BREED=
if [ -z $DEBUG ]; then
	export DEBUG=0
fi

export EMAIL='root@synfig.org'

# Bundled libraries
LIBSIGCPP=2.2.10
GLEW=1.5.1
CAIROMM=1.8.0
IMAGEMAGICK=6.8.6
PANGOMM=2.26.3		# required by GTKMM 2.20.3
GTKMM=3.0.0 		# !!! we need Notebook.set_action_widget()
FTGL=2.1.2
FREEGLUT=2.4.0
GTKGLEXT=1.2.0
GTKGLEXTMM=1.2.0
LIBXMLPP=2.22.0
GLIBMM=2.24.2		# required by GTKMM 2.20.3
CAIRO=1.12.0		# required by the cairo render engine 2013-04-01
BOOST=1_53_0

# System libraries
ATK=1.29.4			# required by GTK 2.20.1
GLIB=2.24.2			# required by GLIBMM 2.24.2
GTK=3.0.0			# !!! we need Notebook.set_action_widget()
PIXMAN=0.22.0		# required by CAIRO 1.12.0
PANGO=1.24.5
FONTCONFIG=2.5.0
JACK=0.124.1


GITVERSION=1.7.0   # git version for chroot environment

SYNFIG_REPO_DIR=''
WORKDIR_IS_REPO=0

# Allow overriding PREFIX and/or BREED
if [ -f "./synfigstudio-build.conf" ] ; then
	. "./synfigstudio-build.conf"
fi

set -e

if (test "$2"); then
	SELECTEDREVISION=$2
else
	SELECTEDREVISION=origin/master
fi

mklibsigcpp()
{
if ! pkg-config sigc\+\+-2.0 --exact-version=${LIBSIGCPP}  --print-errors; then
	pushd /source
	wget -c --no-check-certificate http://ftp.gnome.org/pub/GNOME/sources/libsigc++/${LIBSIGCPP%.*}/libsigc++-${LIBSIGCPP}.tar.bz2
	[ ! -d libsigc++-${LIBSIGCPP} ] && tar -xjf libsigc++-${LIBSIGCPP}.tar.bz2 #&& cd libsigc++-${LIBSIGCPP} && patch -p1 < ../libsigc++-2.0_2.0.18-2.diff && cd ..
	cd libsigc++-${LIBSIGCPP}
	#make clean || true
	./configure --prefix=${PREFIX}/ --includedir=${PREFIX}/include --disable-static --enable-shared
	make -j$MAKE_THREADS
	make install
	cd ..
	popd
fi
}

mkglib()
{
if ! pkg-config glib-2.0 --exact-version=${GLIB}  --print-errors; then
	pushd /source
	[ ! -d glib-${GLIB} ] && tar -xjf glib-${GLIB}.tar.bz2
	cd glib-${GLIB}
	#[[ $DOCLEAN == 1 ]] && make clean || true
	./configure --disable-static --enable-shared
	make -j$MAKE_THREADS
	make install
	cd ..
	popd
fi
}

mkjack()
{
if ! pkg-config jack --exact-version=${JACK}  --print-errors; then
	pushd /source
	apt-get install -y libdb-dev uuid-dev
	[ ! -d jack-audio-connection-kit-${JACK} ] && tar -xzf jack-audio-connection-kit-${JACK}.tar.gz
	cd jack-audio-connection-kit-${JACK}
	#[[ $DOCLEAN == 1 ]] && make clean || true
	./configure --disable-static --enable-shared \
		--libdir=/usr/local/lib
	make -j$MAKE_THREADS
	make install
	cd ..
	popd
fi
}


mkatk()
{
if ! pkg-config atk --exact-version=${ATK}  --print-errors; then
	pushd /source
	[ ! -d atk-${ATK} ] && tar -xjf atk-${ATK}.tar.bz2
	cd atk-${ATK}
	#[[ $DOCLEAN == 1 ]] && make clean || true
	./configure --disable-static --enable-shared
	make -j$MAKE_THREADS
	make install
	cd ..
	popd
fi
}

mkglibmm()
{
if ! pkg-config glibmm-2.4 --exact-version=${GLIBMM}  --print-errors; then
	pushd /source
	[ ! -d glibmm-${GLIBMM} ] && tar -xjf glibmm-${GLIBMM}.tar.bz2
	cd glibmm-${GLIBMM}
	#[[ $DOCLEAN == 1 ]] && make clean || true
	./configure --prefix=${PREFIX} --includedir=${PREFIX}/include --disable-static --enable-shared --disable-fulldocs
	make -j$MAKE_THREADS
	make install
	cd ..
	popd
fi
}

mklibxmlpp()
{
if ! pkg-config libxml\+\+-2.6 --exact-version=${LIBXMLPP}  --print-errors; then
	pushd /source
	[ ! -d libxml++-${LIBXMLPP} ] && tar -xjf libxml++-${LIBXMLPP}.tar.bz2
	cd libxml++-${LIBXMLPP}
	#[[ $DOCLEAN == 1 ]] && make clean || true
	./configure --prefix=${PREFIX} --includedir=${PREFIX}/include --disable-static --enable-shared
	make -j$MAKE_THREADS
	make install
	cd ..
	popd
fi
}

mkimagemagick()
{
PKG_NAME=ImageMagick
PKG_VERSION="${IMAGEMAGICK}-10"
TAREXT=bz2
if ! pkg-config ImageMagick --exact-version=${IMAGEMAGICK}  --print-errors; then
	pushd /source
	[ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://www.imagemagick.org/download/legacy/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xjf ${PKG_NAME}-${PKG_VERSION}.tar.bz2 # && cd ${PKG_NAME}-${PKG_VERSION} && patch -p1 < ../ImageMagick-6.4.0-multilib.patch && cd ..
	cd ${PKG_NAME}-${PKG_VERSION}
	#[[ $DOCLEAN == 1 ]] && make clean || true
	./configure --prefix=${PREFIX} --includedir=${PREFIX}/include --disable-static --enable-shared \
		--with-modules \
		--without-perl \
		--without-x \
		--with-threads \
		--with-magick_plus_plus
	sed -i 's|^hardcode_libdir_flag_spec=.*|hardcode_libdir_flag_spec=""|g' libtool
	sed -i 's|^runpath_var=LD_RUN_PATH|runpath_var=DIE_RPATH_DIE|g' libtool
	make -j$MAKE_THREADS
	make install
	cd ..
	popd
fi
}

mkglew()
{
[ ! -d glew-${GLEW} ] && tar -xzf glew-${GLEW}.tar.gz && cd glew && patch -p1 < glew-${GLEW}-makefile.patch && cd ..
	pushd /source
	cd glew
	#[[ $DOCLEAN == 1 ]] && make clean || true
	sed -i -e 's/\r//g' config/config.guess
	make -j$MAKE_THREADS
	make install GLEW_DEST=${PREFIX} libdir=/lib bindir=/bin  includedir=/include
	cd ..
	popd
}

mkfontconfig()
{
if ! pkg-config fontconfig --exact-version=${FONTCONFIG}  --print-errors; then
	pushd /source
	[ ! -d fontconfig-${FONTCONFIG} ] && tar -xzf fontconfig-${FONTCONFIG}.tar.gz
	cd fontconfig-${FONTCONFIG}
	#[[ $DOCLEAN == 1 ]] && make clean || true
	./configure --disable-static --enable-shared
	make -j$MAKE_THREADS
	make install
	cd ..
fi
}

mkpixman()
{
if ! pkg-config pixman-1 --exact-version=${PIXMAN}  --print-errors; then
	pushd /source
	[ ! -d pixman-${PIXMAN} ] && tar -xzf pixman-${PIXMAN}.tar.gz
	cd pixman-${PIXMAN}
	#[[ $DOCLEAN == 1 ]] && make clean || true
	./configure --disable-static --enable-shared
	make -j$MAKE_THREADS
	make install
	cd ..
fi
}

mkcairo()
{
if ! pkg-config cairo --exact-version=${CAIRO}  --print-errors; then
	pushd /source
	[ ! -d cairo-${CAIRO} ] && tar -xzf cairo-${CAIRO}.tar.gz
	cd cairo-${CAIRO}
	#[[ $DOCLEAN == 1 ]] && make clean || true
	./configure --prefix=${PREFIX} \
		--disable-static 	\
		--enable-warnings 	\
		--enable-xlib 		\
		--enable-freetype 	\
	    --enable-gobject    \
		--disable-gtk-doc
	make -j$MAKE_THREADS
	make install
	cd ..
fi
}

mkcairomm()
{
if ! pkg-config cairomm-1.0 --exact-version=${CAIROMM}  --print-errors; then
	pushd /source
	[ ! -d cairomm-${CAIROMM} ] && tar -xzf cairomm-${CAIROMM}.tar.gz
	cd cairomm-${CAIROMM}
	#[[ $DOCLEAN == 1 ]] && make clean || true
	./configure --prefix=${PREFIX} --includedir=${PREFIX}/include --disable-static --enable-shared --enable-docs=no
	make -j$MAKE_THREADS
	make install
	cd ..
fi
}

mkpango()
{
if ! pkg-config pango --exact-version=${PANGO}  --print-errors; then
	pushd /source
	[ ! -d pango-${PANGO} ] && tar -xjf pango-${PANGO}.tar.bz2
	cd pango-${PANGO}
	#[[ $DOCLEAN == 1 ]] && make clean || true
	./configure  --disable-static --enable-shared
	make -j$MAKE_THREADS
	make install
	cd ..
	popd
fi
}

mkpangomm()
{
if ! pkg-config pangomm-1.4 --exact-version=${PANGOMM}  --print-errors; then
	pushd /source
	[ ! -d pangomm-${PANGOMM} ] && tar -xjf pangomm-${PANGOMM}.tar.bz2
	cd pangomm-${PANGOMM}
	#[[ $DOCLEAN == 1 ]] && make clean || true
	./configure --prefix=${PREFIX} --includedir=${PREFIX}/include --disable-static --enable-shared --disable-docs
	make -j$MAKE_THREADS
	make install
	cd ..
	popd
fi
}

mkgtk()
{
if ! pkg-config gtk\+-3.0 --exact-version=${GTK}  --print-errors; then
	pushd /source
	[ ! -d gtk\+-${GTK} ] && tar -xjf gtk\+-${GTK}.tar.bz2
	cd gtk\+-${GTK}
	#[[ $DOCLEAN == 1 ]] && make clean || true
	./configure --disable-static --enable-shared --disable-examples --disable-demos --disable-docs
	make -j$MAKE_THREADS
	make install
	cd ..
	popd
fi
}

mkgtkmm()
{
if ! pkg-config gtkmm-3.0 --exact-version=${GTKMM}  --print-errors; then
	pushd /source
	[ ! -d gtkmm-${GTKMM} ] && tar -xjf gtkmm-${GTKMM}.tar.bz2
	cd gtkmm-${GTKMM}
	#[[ $DOCLEAN == 1 ]] && make clean || true
	./configure --prefix=${PREFIX} --includedir=${PREFIX}/include --disable-static --enable-shared --disable-examples --disable-demos --disable-docs
	make -j$MAKE_THREADS
	make install
	cd ..
	popd
fi
}

mkfreeglut()
{
	pushd /source
	[ ! -d freeglut-${FREEGLUT} ] && tar -xzf freeglut-${FREEGLUT}.tar.gz
	cd freeglut-${FREEGLUT}
	#[[ $DOCLEAN == 1 ]] && make clean || true
	./configure --prefix=${PREFIX} --includedir=${PREFIX}/include --disable-static  --disable-warnings --enable-shared
	make -j$MAKE_THREADS
	make install
	cd ..
	popd
}

mkftgl()
{
	pushd /source
	if [ ! -d ftgl-${FTGL} ]; then
		tar -xjf ftgl-${FTGL}.tar.bz2
		cd FTGL
		patch -p1 -b < ftgl-2.1.2-destdir.patch
		patch -p1 -b < ftgl-2.1.2-Glyph-g++_41.patch
		patch -p1 -b < ftgl-2.1.2-pc_req.patch
		patch -p1 -b < ftgl-2.1.2-rpath_FTGLDemo.patch
		patch -p1 -b < ftgl-2.1.2-ttf_font.patch
		sed -i.fixed_version -e 's|2.0.5|%{version}|g' unix/configure.ac unix/configure
		cd ..
	fi
	cd FTGL
	cd unix
	#[[ $DOCLEAN == 1 ]] && make clean || true
	./configure --prefix=${PREFIX} --includedir=${PREFIX}/include --disable-static --enable-shared --with-gl-inc=${PREFIX}/include  --with-gl-lib=${PREFIX}/lib  --with-glut-inc=${PREFIX}/include  --with-glut-lib=${PREFIX}/lib  --with-x
	make all -j$MAKE_THREADS
	make install
	cd ..
	popd
}

mkgtkglext()
{
	pushd /source
	[ ! -d gtkglext-${GTKGLEXT} ] && tar -xjf gtkglext-${GTKGLEXT}.tar.bz2
	cd gtkglext-${GTKGLEXT}
	#[[ $DOCLEAN == 1 ]] && make clean || true
	./configure --prefix=${PREFIX} --includedir=${PREFIX}/include  --disable-gtk-doc --disable-static  --enable-shared
	make -j$MAKE_THREADS
	make install
	cd ..
	popd
}

mkgtkglextmm()
{
	pushd /source
	[ ! -d gtkglextmm-${GTKGLEXTMM} ] && tar -xjf gtkglextmm-${GTKGLEXTMM}.tar.bz2 && cd gtkglextmm-${GTKGLEXTMM} && patch -p1 < gtkglextmm-1.2.0-aclocal.diff && cd ..
	cd gtkglextmm-${GTKGLEXTMM}
	#[[ $DOCLEAN == 1 ]] && make clean || true
	./configure --prefix=${PREFIX} --includedir=${PREFIX}/include  --disable-dependency-tracking --disable-static  --enable-shared
	make -j$MAKE_THREADS
	make install
	popd
cd ..
}

mkgit()
{
	pushd /source
	[ ! -d git-${GITVERSION} ] && tar -xjf git-${GITVERSION}.tar.bz2
	cd git-${GITVERSION}
	./configure
	make -j$MAKE_THREADS
	make install
	cd ..
	popd
}

mkboost()
{
if ! cat /usr/local/include/boost/version.hpp |egrep "BOOST_LIB_VERSION \"${BOOST%_*}\""; then
	pushd /source
	[ ! -d boost-${BOOST} ] && tar -xjf boost_${BOOST}.tar.bz2
	cd boost_${BOOST}
	./bootstrap.sh
	./b2 || true
	./b2 install || true
	cd ..
	popd
fi
cp /usr/local/lib/libboost_program_options.so.1.53.0 $PREFIX/lib/
}

mkETL()
{
if [ -f ${SYNFIG_REPO_DIR}/ETL/trunk/configure.ac ]; then
	pushd ${SYNFIG_REPO_DIR}/ETL/trunk
else
	pushd ${SYNFIG_REPO_DIR}/ETL
fi


if ( [[ $MODE == 'package' ]] || [[ $MODE == 'full' ]] ); then
	echo "Cleaning source tree..."
	make clean || true
fi


if [[ $MODE != 'quick' ]]; then
	echo "Going to configure..."
	rm -f aclocal.m4
	autoreconf --install --force
	/bin/sh ./configure --prefix=${PREFIX} --includedir=${PREFIX}/include $DEBUG
fi

make -j$MAKE_THREADS
make install

popd
}

mksynfig()
{
if [ -d ${SYNFIG_REPO_DIR}/synfig-core/trunk/configure.ac ]; then
	pushd ${SYNFIG_REPO_DIR}/synfig-core/trunk
else
	pushd ${SYNFIG_REPO_DIR}/synfig-core
fi

( [[ $MODE == 'package' ]] || [[ $MODE == 'full' ]] ) && make clean || true

if [[ $MODE != 'quick' ]]; then
	libtoolize --ltdl --copy --force
	sed -i 's/^AC_CONFIG_SUBDIRS(libltdl)$/m4_ifdef([_AC_SEEN_TAG(libltdl)], [], [AC_CONFIG_SUBDIRS(libltdl)])/' configure.ac || true
	sed -i 's/^# AC_CONFIG_SUBDIRS(libltdl)$/m4_ifdef([_AC_SEEN_TAG(libltdl)], [], [AC_CONFIG_SUBDIRS(libltdl)])/' configure.ac || true
	autoreconf --install --force
	if [ -e /etc/debian_version ] && [ -z $BOOST_CONFIGURE_OPTIONS ]; then
		# Debian/Ubuntu multiarch
		MULTIARCH_LIBDIR="/usr/lib/`uname -m`-linux-gnu/"
		if [ -e "${MULTIARCH_LIBDIR}/libboost_program_options.so" ]; then
			export BOOST_CONFIGURE_OPTIONS="--with-boost-libdir=$MULTIARCH_LIBDIR"
		fi
	fi
	export CONFIG_SHELL=/bin/bash
	/bin/bash ./configure --prefix=${PREFIX} \
		--includedir=${PREFIX}/include \
		--disable-static --enable-shared \
		--with-magickpp \
		--without-libavcodec \
		--without-included-ltdl \
		$BOOST_CONFIGURE_OPTIONS \
		$DEBUG
fi

#It looks like mod_libavcodec causes segfault on synfig-core when rendering to png.
#Tested on: ffmpeg-0.4.9-0.52.20080908.fc10.x86_64.

#if [[ $GIT == 1 ]]; then
#	export CFLAGS=-I/opt/synfig/include/GL/
#	export LDFLAGS=-L/opt/synfig/lib/
#else
#	export CFLAGS=''
#	export LDFLAGS=''
#fi

make -j$MAKE_THREADS
make install

sed -i "s|^includedir=.*$|includedir=$SYNFIG_REPO_DIR\/synfig-core\/src|" ${PREFIX}/lib/pkgconfig/synfig.pc

popd
}

mksynfigstudio()
{
if [ -d ${SYNFIG_REPO_DIR}/synfig-studio/trunk/configure.ac ]; then
	pushd ${SYNFIG_REPO_DIR}/synfig-studio/trunk
else
	pushd ${SYNFIG_REPO_DIR}/synfig-studio
fi

( [[ $MODE == 'package' ]] || [[ $MODE == 'full' ]] ) && make clean || true

if [[ $MODE == 'package' ]]; then
	CONFIGURE_PACKAGE_OPTIONS='--disable-update-mimedb'
fi

if [[ $MODE != 'quick' ]]; then
	/bin/sh ./bootstrap.sh
	/bin/sh ./configure --prefix=${PREFIX} --includedir=${PREFIX}/include --disable-static --enable-shared --enable-jack --enable-warnings=max $DEBUG $CONFIGURE_PACKAGE_OPTIONS
fi

make -j$MAKE_THREADS
make install

for n in AUTHORS COPYING NEWS README
do
  	cp -f $n ${PREFIX}
done

#if [ -e synfigstudio-cph-monitor ]; then
#	cp -f synfigstudio-cph-monitor ${PREFIX}/bin/
#	chmod a+x ${PREFIX}/bin/synfigstudio-cph-monitor
#fi

popd
}

mkpack()
{
	[ -d /packages ] || mkdir /packages

	# bundle libpng
	rm -f ${PREFIX}/lib/libpng* || true
	cp -av /usr/lib/libpng*.so* ${PREFIX}/lib
	# bundle libjasper
	rm -f ${PREFIX}/lib/libjasper* || true
	cp -av /usr/lib/libjasper*.so* ${PREFIX}/lib
	# bundle libltdl
	rm -f ${PREFIX}/lib/libltdl* || true
	cp -av /usr/lib/libltdl*.so* ${PREFIX}/lib

	# A place for optional libs
	[ -e ${PREFIX}/lib.extra ] || mkdir -p ${PREFIX}/lib.extra
	# bundle optional libjack
	rm -f ${PREFIX}/lib.extra/libjack* || true
	cp -av /usr/local/lib/libjack.so* ${PREFIX}/lib.extra
	rm -f ${PREFIX}/lib.extra/libdb-4* || true
	cp -av /usr/lib/libdb-4*.so ${PREFIX}/lib.extra
	
	cat > $PREFIX/synfig <<EOF
#!/bin/sh

PREFIX="/opt/synfig"

export LD_LIBRARY_PATH=\${PREFIX}/lib:\$LD_LIBRARY_PATH
export SYNFIG_ROOT=\${PREFIX}/
export SYNFIG_MODULE_LIST=\${PREFIX}/etc/synfig_modules.cfg

\$PREFIX/bin/synfig "\$@"
EOF
	chmod a+x $PREFIX/synfig
	
	cat > $PREFIX/synfigstudio <<EOF
#!/bin/sh

PREFIX="/opt/synfig"

# Check if this system have JACK installed
if ( ! ldconfig -p | grep libjack.so >/dev/null ) || ( ! which jackd >/dev/null ) ; then
	# No JACK, so disable this functionality.
	# (The bundled libjack won't work correctly anyway).
	export SYNFIG_DISABLE_JACK=1
	export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:\${PREFIX}/lib.extra
fi

export LD_LIBRARY_PATH=\${PREFIX}/lib:\$LD_LIBRARY_PATH
export SYNFIG_ROOT=\${PREFIX}/
export SYNFIG_MODULE_LIST=\${PREFIX}/etc/synfig_modules.cfg

\$PREFIX/bin/synfigstudio "\$@"
EOF
	chmod a+x $PREFIX/synfigstudio
	
	#== tar.bz2 ==
	TBZPREFIX=/tmp/synfigstudio-${VERSION}-${REVISION}.$BREED.$RELEASE.${ARCH}
	rm -rf $TBZPREFIX
	mkdir -p $TBZPREFIX
	cp -r  ${PREFIX}/* $TBZPREFIX

	if [[ $DEBUG != '' ]]; then
		GDB="which gdb && xterm -e  gdb -ex run -ex quit \$PREFIX/bin/synfig \"\$@\" || "
	else
		GDB=''
	fi

	#binaries
	cat > $TBZPREFIX/synfig <<EOF
#!/bin/sh

PREFIX=\`dirname \$0\`
PREFIX=\`cd \$PREFIX; pwd\`
export LD_LIBRARY_PATH=\${PREFIX}/lib:\$LD_LIBRARY_PATH
export SYNFIG_ROOT=\${PREFIX}/
export SYNFIG_MODULE_LIST=\${PREFIX}/etc/synfig_modules.cfg

$GDB\$PREFIX/bin/synfig "\$@"
EOF
	if [[ $DEBUG != '' ]]; then
		GDB="which gdb && xterm -e  gdb -ex run -ex quit \$PREFIX/bin/synfigstudio \$@ || "
	else
		GDB=''
	fi
	cat > $TBZPREFIX/synfigstudio <<EOF
#!/bin/sh

PREFIX=\`dirname \$0\`
PREFIX=\`cd \$PREFIX; pwd\`

# Check if this system have JACK installed
if ( ! ldconfig -p | grep libjack.so >/dev/null ) || ( ! which jackd >/dev/null ) ; then
	# No JACK, so disable this functionality.
	# (The bundled libjack won't work correctly anyway).
	export SYNFIG_DISABLE_JACK=1
	export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:\${PREFIX}/lib.extra
fi

export LD_LIBRARY_PATH=\${PREFIX}/lib:\$LD_LIBRARY_PATH
export SYNFIG_ROOT=\${PREFIX}/
export SYNFIG_MODULE_LIST=\${PREFIX}/etc/synfig_modules.cfg

$GDB\$PREFIX/bin/synfigstudio "\$@"
EOF
	chmod a+x $TBZPREFIX/synfig
	chmod a+x $TBZPREFIX/synfigstudio

	#cleaning devel stuff
	mkdir $TBZPREFIX/bin.tmp
	mv $TBZPREFIX/bin/synfig $TBZPREFIX/bin.tmp/synfig
	mv $TBZPREFIX/bin/synfigstudio $TBZPREFIX/bin.tmp/synfigstudio
	rm -rf $TBZPREFIX/bin
	mv $TBZPREFIX/bin.tmp $TBZPREFIX/bin

	rm -f $TBZPREFIX/lib/*.la
	rm -f $TBZPREFIX/lib/*.a
	rm -f $TBZPREFIX/lib/cairo/*.la
	rm -rf $TBZPREFIX/include
	rm -rf $TBZPREFIX/lib/gdkmm-3.0
	rm -rf $TBZPREFIX/lib/libxml++-2.6
	rm -rf $TBZPREFIX/lib/glibmm-2.4
	rm -rf $TBZPREFIX/lib/pangomm-1.4
	rm -rf $TBZPREFIX/lib/gtkmm-3.0
	rm -rf $TBZPREFIX/lib/pkgconfig
	rm -rf $TBZPREFIX/lib/sigc++-2.0
	rm -rf $TBZPREFIX/share/doc
	rm -rf $TBZPREFIX/share/devhelp
	rm -rf $TBZPREFIX/share/gtk-doc
	rm -rf $TBZPREFIX/share/aclocal
	rm -rf $TBZPREFIX/share/ImageMagick-6.4.0
	rm -rf $TBZPREFIX/share/man

	rm -f /packages/synfigstudio-${VERSION}-${REVISION}.$BREED.$RELEASE.${ARCH}.tar.bz2
	pushd $TBZPREFIX/../
	tar cjf /packages/synfigstudio-${VERSION}-${REVISION}.$BREED.$RELEASE.${ARCH}.tar.bz2 synfigstudio-${VERSION}-${REVISION}.$BREED.$RELEASE.${ARCH}
	popd
	rm -rf $TBZPREFIX

	#== rpm ==
    cat > synfigstudio.spec << EOF
%define __spec_install_post /bin/true

Name:           synfigstudio
Version:        ${VERSION}
Release:        ${REVISION}.${BREED}.${RELEASE}
Summary:        Film-Quality 2D Vector Animation package
Group:          Applications/Graphics
License:        GPL
URL:            http://www.synfig.org/
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Obsoletes:       synfig ETL
AutoReqProv: no


%description
Synfig Animation Studio is a powerful, industrial-strength vector-based
2D animation software, designed from the ground-up for producing
feature-film quality animation with fewer people and resources.
It eliminates the need for tweening, preventing the need to hand-draw
each frame. Synfig features spatial and temporal resolution independence
(sharp and smooth at any resolution or framerate), high dynamic range
images, and a flexible plugin system.


%prep


%build

%install
rm -rf \$RPM_BUILD_ROOT
mkdir -p \$RPM_BUILD_ROOT/${PREFIX}
cp -r  ${PREFIX}/* \$RPM_BUILD_ROOT/${PREFIX}
mkdir -p \$RPM_BUILD_ROOT/usr/share
mv \$RPM_BUILD_ROOT/${PREFIX}/share/applications \$RPM_BUILD_ROOT/usr/share
mv \$RPM_BUILD_ROOT/${PREFIX}/share/icons \$RPM_BUILD_ROOT/usr/share
mv \$RPM_BUILD_ROOT/${PREFIX}/share/mime \$RPM_BUILD_ROOT/usr/share
mkdir -p \$RPM_BUILD_ROOT/usr/share/mime-info
ln -sf ${PREFIX}/share/mime-info/synfigstudio.keys \$RPM_BUILD_ROOT/usr/share/mime-info/synfigstudio.keys
ln -sf ${PREFIX}/share/mime-info/synfigstudio.mime \$RPM_BUILD_ROOT/usr/share/mime-info/synfigstudio.mime
mkdir -p \$RPM_BUILD_ROOT/usr/share/pixmaps
ln -sf ${PREFIX}/share/pixmaps/sif_icon.png \$RPM_BUILD_ROOT/usr/share/pixmaps/sif_icon.png
ln -sf ${PREFIX}/share/pixmaps/synfig_icon.png \$RPM_BUILD_ROOT/usr/share/pixmaps/synfig_icon.png
mkdir -p \$RPM_BUILD_ROOT/usr/bin
cp \$RPM_BUILD_ROOT/${PREFIX}/synfig \$RPM_BUILD_ROOT/usr/bin/
cp \$RPM_BUILD_ROOT/${PREFIX}/synfigstudio \$RPM_BUILD_ROOT/usr/bin/

#if [ -e \$RPM_BUILD_ROOT/${PREFIX}/bin/synfigstudio-cph-monitor ]; then
#mv \$RPM_BUILD_ROOT/${PREFIX}/bin/synfigstudio-cph-monitor \$RPM_BUILD_ROOT/usr/bin/
#cat > \$RPM_BUILD_ROOT/usr/share/applications/synfigstudio-cph-monitor.desktop << EOD
#[Desktop Entry]
#Encoding=UTF-8
#Name=Synfig Studio CPH monitor
#Comment=This application collecting statistics about synfig crashes
#Exec=synfigstudio-cph-monitor
#Icon=terminal.png
#Terminal=true
#Type=Application
#Categories=Graphics;Application;
#X-Desktop-File-Install-Version=0.15
#EOD
#fi

#cleaning devel stuff
rm -f \$RPM_BUILD_ROOT/${PREFIX}/lib/*.la
rm -f \$RPM_BUILD_ROOT/${PREFIX}/lib/*.a
rm -f \$RPM_BUILD_ROOT/${PREFIX}/lib/cairo/*.la
#rm -rf \$RPM_BUILD_ROOT/${PREFIX}/bin
rm -rf \$RPM_BUILD_ROOT/${PREFIX}/include
rm -rf \$RPM_BUILD_ROOT/${PREFIX}/lib/gdkmm-3.0
rm -rf \$RPM_BUILD_ROOT/${PREFIX}/lib/libxml++-2.6
rm -rf \$RPM_BUILD_ROOT/${PREFIX}/lib/glibmm-2.4
rm -rf \$RPM_BUILD_ROOT/${PREFIX}/lib/gtkmm-3.0
rm -rf \$RPM_BUILD_ROOT/${PREFIX}/lib/pangomm-1.4
rm -rf \$RPM_BUILD_ROOT/${PREFIX}/lib/pkgconfig
rm -rf \$RPM_BUILD_ROOT/${PREFIX}/lib/sigc++-2.0
rm -rf \$RPM_BUILD_ROOT/${PREFIX}/share/doc
rm -rf \$RPM_BUILD_ROOT/${PREFIX}/share/devhelp
rm -rf \$RPM_BUILD_ROOT/${PREFIX}/share/gtk-doc
rm -rf \$RPM_BUILD_ROOT/${PREFIX}/share/aclocal
rm -rf \$RPM_BUILD_ROOT/${PREFIX}/share/ImageMagick-6.4.0
rm -rf \$RPM_BUILD_ROOT/${PREFIX}/share/man


%clean
rm -rf \$RPM_BUILD_ROOT

%post
if [ -x /usr/bin/update-mime-database ]; then
  update-mime-database /usr/share/mime
fi
if [ -x /usr/bin/update-desktop-database ]; then
  update-desktop-database
fi

%postun
if [ -x /usr/bin/update-mime-database ]; then
  update-mime-database /usr/share/mime
fi
if [ -x /usr/bin/update-desktop-database ]; then
  update-desktop-database
fi

%files
%defattr(-,root,root,-)
$PREFIX
/usr/share/*
/usr/bin/*

%changelog
* Sat Mar 21 2009 Konstantin Dmitriev <ksee.zelgadis@gmail.com> - 0.61.09-2354.morevnapackage.1
- Update to SVN2354
- Include ImageMagick-c++

* Wed Jan 14 2009 Konstantin Dmitriev <ksee.zelgadis@gmail.com> - 0.61.09-2316.morevnapackage.1
- First release

EOF
    rpmbuild -bb synfigstudio.spec

    #cp /usr/src/redhat/RPMS/$ARCH/synfigstudio-${VERSION}-${REVISION}.${BREED}.$RELEASE.${ARCH}.rpm ../
    cp /usr/src/rpm/RPMS/$ARCH/synfigstudio-${VERSION}-${REVISION}.${BREED}.$RELEASE.${ARCH}.rpm /packages/
    pushd /packages/
    alien -k --scripts synfigstudio-${VERSION}-${REVISION}.${BREED}.$RELEASE.${ARCH}.rpm
    rm -rf synfigstudio-${VERSION}
    popd
}

initialize()
{
	# Make sure we have all dependencies installed
	echo "Checking dependencies..."
	DEB_LIST_MINIMAL="\
		build-essential \
		autoconf automake \
		shared-mime-info \
		libltdl3-dev \
		libtool \
		intltool \
		gettext \
		libpng12-dev \
		fontconfig \
		libfreetype6-dev \
		libfontconfig1-dev \
		libxml2-dev \
		libtiff-dev \
		libmlt-dev libmlt++-dev \
		libjasper-dev \
		x11proto-xext-dev libdirectfb-dev libxfixes-dev libxinerama-dev libxdamage-dev libxcomposite-dev libxcursor-dev libxft-dev libxrender-dev libxt-dev libxrandr-dev libxi-dev libxext-dev libx11-dev \
		libatk1.0-dev \
		libgl1-mesa-dev \
		imagemagick \
		bzip2"
	if which yum >/dev/null; then
		#
		#  Fedora
		#
		PKG_LIST="git"
		if [[ $MODE == 'package' ]]; then
			PKG_LIST="${PKG_LIST} \
				debootstrap \
				rsync"
		else
			PKG_LIST="${PKG_LIST} \
				intltool \
				libpng-devel \
				libjpeg-devel \
				freetype-devel \
				fontconfig-devel \
				atk-devel \
				pango-devel \
				cairo-devel \
				gtk3-devel \
				gettext-devel \
				libxml2-devel \
				libxml++-devel \
				gcc-c++ \
				autoconf \
				automake \
				libtool \
				libtool-ltdl-devel \
				boost-devel \
				boost-program-options \
				shared-mime-info \
				OpenEXR-devel \
				libmng-devel \
				ImageMagick-c++-devel \
				jack-audio-connection-kit-devel \
				mlt-devel \
				ocl-icd-devel \
				opencl-headers \
				gtkmm30-devel \
				glibmm24-devel"
		fi
		if ! ( rpm -qv $PKG_LIST ); then
			echo "Running yum (you need root privelegies to do that)..."
			su -c "yum install $PKG_LIST" || true
		fi
	elif which zypper >/dev/null; then
		#
		#  OpenSUSE
		#
		PKG_LIST="git"
		if [[ $MODE == 'package' ]]; then
			PKG_LIST="${PKG_LIST} \
				debootstrap \
				rsync"
		else
			PKG_LIST="${PKG_LIST} libpng-devel libjpeg-devel freetype-devel fontconfig-devel atk-devel pango-devel cairo-devel gtk3-devel gettext-devel libxml2-devel libxml++-devel gcc-c++ autoconf automake libtool libtool-ltdl-devel boost-devel boost-program-options shared-mime-info"
			PKG_LIST="${PKG_LIST} OpenEXR-devel libmng-devel ImageMagick-c++-devel gtkmm3-devel glibmm2-devel"
		fi
		if ! ( rpm -qv $PKG_LIST ); then
			echo "Running zypper (you need root privelegies to do that)..."
			su -c "zypper install $PKG_LIST" || true
		fi
	elif which apt-get >/dev/null; then
		if [[ $MODE == 'package' ]]; then
			if [[ `cat /etc/chroot.id` == "Synfig Packages Buildroot v${BUILDROOT_VERSION}" ]]; then
				#we are inside of chroot
				PKG_LIST="$DEB_LIST_MINIMAL rpm alien xsltproc wget python"
			else
				#we have to prepare chroot
				PKG_LIST="git-core debootstrap rsync"
			fi
		else
			if ( cat /etc/altlinux-release | egrep "ALT Linux" ); then
				#
				#  ALT Linux case
				#
				PKG_LIST=" \
					rpm-build \
					boost-program_options-devel \
					git-core \
					shared-mime-info \
					libltdl3-devel \
					intltool \
					gettext \
					libpng12-devel \
					libjpeg-devel \
					fontconfig \
					libfreetype-devel \
					fontconfig-devel \
					libxml2-devel \
					libtiff-devel \
					libjasper-devel \
					libdirectfb-devel \
					libXfixes-devel \
					libXinerama-devel \
					libXdamage-devel \
					libXcomposite-devel \
					libXcursor-devel \
					libXft-devel \
					libXrender-devel \
					libXt-devel \
					libXrandr-devel \
					libXi-devel \
					libXext-devel \
					libX11-devel \
					libatk-devel \
					bzip2 \
					libmng-devel \
					libgtkmm3-devel \
					libglibmm-devel \
					libsigc++2-devel \
					libxml++2-devel \
				"
			else
				#
				#  Ubuntu/Debian case
				#
				PKG_LIST=" \
					${DEB_LIST_MINIMAL} \
					git-core \
					libmng-dev \
					libjack-jackd2-dev \
					libgtkmm-3.0-dev \
					libglibmm-2.4-dev \
					libsigc++-2.0-dev \
					libxml++2.6-dev \
					libboost-program-options-dev \
					libboost-system-dev \
					libboost-filesystem-dev \
					libboost-chrono-dev \
				"
			fi
		fi
		echo "Running apt-get (you need root privelegies to do that)..."
		echo
		sudo apt-get update || true
		sudo apt-get install -y $PKG_LIST
		sudo apt-get install -y autopoint || true # Ubuntu special case
	else
		if [[ $MODE == 'package' ]]; then
			if ! ( which git && which debootstrap ) ; then
				echo "ERROR: Please install 'git' and 'debootstrap'."
				exit;
			fi
		else
			echo "WARNING: This build script does not works with package mangement systems other than yum, zypper or apt! You should install dependent packages manually."
			echo "REQUIRED PACKAGES: libpng-devel libjpeg-devel freetype-devel fontconfig-devel atk-devel pango-devel cairo-devel gtk3-devel gettext-devel libxml2-devel libxml++-devel gcc-c++ autoconf automake libtool libtool-ltdl-devel shared-mime-info OpenEXR-devel libmng-devel ImageMagick-c++-devel gtkmm30-devel glibmm24-devel"
			echo ""
			read
		fi
	fi
	echo "Done."

	if [[ $DEBUG == 1 ]]; then
		DEBUG='--enable-debug --enable-optimization=0'
	else
		DEBUG=''
	fi

	if [[ $MODE == 'package' ]]; then
		PREFIX="/opt/synfig"
	fi

	if [[ $MODE == 'package' ]] && [[ `cat /etc/chroot.id` == "Synfig Packages Buildroot v${BUILDROOT_VERSION}" ]]; then
		SYNFIG_REPO_DIR="/source/synfig.git"

		which git || mkgit

		pushd $SYNFIG_REPO_DIR
		git fetch
		git reset --hard HEAD
		#we can't switch directly to specified commit wit old git, so let's create temporary branch:
		#git checkout master || ( git branch -f master && git checkout master )
		#git branch -f __build $SELECTEDREVISION
		#git checkout __build
		git checkout $SELECTEDREVISION
		VERSION=`cat synfig-core/configure.ac |egrep "AC_INIT\(\[Synfig Core\],"| sed "s|.*Core\],\[||" | sed "s|\],\[.*||"`
		if [ -z $BREED ]; then
			BREED="`git branch -a --no-color --contains HEAD | sed -e s/\*\ // | sed -e s/\(no\ branch\)// | tr '\n' ' ' | tr -s ' ' | sed s/^' '//`"
			if ( echo $BREED | egrep origin/master > /dev/null ); then
				#give a priority to master branch
				BREED='master'
			else
				BREED=`echo $BREED | cut -d ' ' -f 1`
				BREED=${BREED##*/}
			fi
			BREED=${BREED%_master}
		fi
		if [[ ${VERSION##*-RC} != ${VERSION} ]]; then
			#if [[ $BREED == 'master' ]]; then
				BREED=rc${VERSION##*-RC}
			#else
			#	BREED=rc${VERSION##*-RC}.$BREED
			#fi
			VERSION=${VERSION%%-*}
		fi
		[[ $DEBUG == 1 ]] && BREED=${BREED}.dbg
		BREED=`echo $BREED | tr _ . | tr - .`	# No "-" or "_" characters, becuse RPM and DEB complain
		REVISION=`git show --pretty=format:%ci HEAD |  head -c 10 | tr -d '-'`
		echo
		echo
		echo "BUILDING synfigstudio-$VERSION-$REVISION.$BREED.$RELEASE"
		echo
		echo
		sleep 5
		popd

		#Trick to detect arch under chroot
		#ARCH=`rpm -q --queryformat='%{arch}\n' e2fsprogs`
		MACHINE_TYPE=`uname -m`
		case ${MACHINE_TYPE} in
			i586)
			ARCH=i386;;
			i686)
			ARCH=i386;;
			*)
			ARCH=${MACHINE_TYPE};;
		esac
	else
		#detecting repo
		SCRIPTPATH=`dirname "$0"`
		pushd "$SCRIPTPATH"
		if git rev-parse --git-dir >/dev/null; then
			SYNFIG_REPO_DIR=$(dirname `git rev-parse --git-dir`)
			pushd "$SYNFIG_REPO_DIR" > /dev/null
			SYNFIG_REPO_DIR=`pwd`
			popd  > /dev/null
			WORKDIR_IS_REPO=1
		fi
		popd > /dev/null
	fi

	#export PREFIX=/opt/synfig
	export PKG_CONFIG_PATH=${PREFIX}/lib/pkgconfig:${PREFIX}/lib64/pkgconfig:/usr/local/lib/pkgconfig:/usr/lib/`uname -i`-linux-gnu/pkgconfig/:$PKG_CONFIG_PATH
	export PATH=${PREFIX}/bin:$PATH
	export LD_LIBRARY_PATH=${PREFIX}/lib:${PREFIX}/lib64:/usr/local/lib:$LD_LIBRARY_PATH
	export LDFLAGS="-Wl,-rpath -Wl,\\\$\$ORIGIN/lib"
}

mk()
{
	if [[ WORKDIR_IS_REPO == 0 ]]; then
		SYNFIG_REPO_DIR=`pwd`/synfig.git/
		git clone git://github.com/synfig/synfig.git ${SYNFIG_REPO_DIR}
	fi

	mkETL
	mksynfig
	mksynfigstudio
}

mkpackage()
{
	#check if we already in chroot
	if [[ `cat /etc/chroot.id` == "Synfig Packages Buildroot v${BUILDROOT_VERSION}" ]]; then
		echo "We are in chroot now."

		echo "[user]"  > /root/.gitconfig
		echo "email = packages@synfig.org"  >> /root/.gitconfig
		echo "name = Synfig Packager" >> /root/.gitconfig
		
		# we need newer intltool
		dpkg -i /source/intltool_0.41.1-1_all.deb

		#system libs
		mkglib
		mkfontconfig
		mkatk
		mkpixman
		mkcairo
		mkpango
		mkgtk
		mkjack


		#synfig-core deps
		mklibsigcpp
		mkglibmm
		mklibxmlpp
		if [[ $OPENGL == 1 ]]; then
			mkglew
		fi
		mkimagemagick
		mkboost

		#synfig-studio deps
		mkcairomm
		mkpangomm
		mkgtkmm
		if [[ $OPENGL == 1 ]]; then
			mkfreeglut
			mkftgl
			mkgtkglext
			mkgtkglextmm
		fi

		mkETL
		mksynfig
		mksynfigstudio

		mkpack
	else
		[ -d $HOME/synfig-packages ] || mkdir -p $HOME/synfig-packages
		#DEB_LIST="build-essential,autoconf,automake,libltdl3-dev,libtool,gettext,libpng12-dev,libjpeg62-dev,libfreetype6-dev,libfontconfig1-dev,libgtk3.0-dev,libxml2-dev,bzip2,rpm,alien,xsltproc"
		for ARCH in i386 amd64; do
		if [[ $ARCH == 'i386' ]];then
			SETARCH='linux32'
		else
			SETARCH='linux64'
		fi
		
		# If chroot version changed -> reset existing buildroot
		if [[ `cat $PACKAGES_BUILDROOT.$ARCH/etc/chroot.id` != "Synfig Packages Buildroot v${BUILDROOT_VERSION}" ]]; then
			echo "======================= !!! ======================"
			echo "   Buildroot version changed. Force update..."
			echo "======================= !!! ======================"
			if [ -e $PACKAGES_BUILDROOT.$ARCH/ ]; then
				rm -rf $PACKAGES_BUILDROOT.$ARCH/
			fi
			debootstrap --arch=$ARCH --variant=buildd  --include=sudo lenny $PACKAGES_BUILDROOT.$ARCH http://archive.debian.org/debian
			#debootstrap --arch=$ARCH --variant=buildd  --include=sudo squeeze $PACKAGES_BUILDROOT.$ARCH http://ftp.de.debian.org/debian
		fi
		#set chroot ID
		echo "Synfig Packages Buildroot v${BUILDROOT_VERSION}" > $PACKAGES_BUILDROOT.$ARCH/etc/chroot.id
		
		# If library set changed -> remove all existing libraries to force rebuild
		if [[ `cat $PACKAGES_BUILDROOT.$ARCH/etc/chroot_libset.id` != "${BUILDROOT_LIBRARY_SET_ID}" ]]; then
			echo "======================= !!! ======================"
			echo "   Library set is changed. Force cleanup..."
			echo "======================= !!! ======================"
			sleep 5
			echo "Cleaning $PACKAGES_BUILDROOT.$ARCH/usr/local ..."
			rm -rf $PACKAGES_BUILDROOT.$ARCH/usr/local || true
			echo "Cleaning $PACKAGES_BUILDROOT.$ARCH/$PREFIX ..."
			rm -rf $PACKAGES_BUILDROOT.$ARCH/$PREFIX || true
			echo
		fi
		#set library set ID
		echo "${BUILDROOT_LIBRARY_SET_ID}" > $PACKAGES_BUILDROOT.$ARCH/etc/chroot_libset.id
		
		cp -f $0 $PACKAGES_BUILDROOT.$ARCH/build.sh
		
		#resolv.conf
		cp -f /etc/resolv.conf $PACKAGES_BUILDROOT.$ARCH/etc/resolv.conf
		#keep proxy settings
		if ! [ -z $http_proxy ]; then
			#echo "export http_proxy=\"$http_proxy\";" >> $PACKAGES_BUILDROOT.$ARCH/root/.bashrc
			#echo "echo 'proxy export done';" >> $PACKAGES_BUILDROOT.$ARCH/root/.bashrc
			echo "Acquire::http::Proxy \"$http_proxy\";" > $PACKAGES_BUILDROOT.$ARCH/etc/apt/apt.conf
		fi
		#fetch sources to cache
		[ -d $PACKAGES_BUILDROOT.$ARCH/source/synfig.git ] && rm -rf $PACKAGES_BUILDROOT.$ARCH/source/synfig.git || true
		if [ -d $PACKAGES_BUILDROOT/synfig.git ]; then
			if [[ $WORKDIR_IS_REPO == 1 ]]; then
				rm -rf "$PACKAGES_BUILDROOT/synfig.git"
			fi
		fi
		if ! [ -d $PACKAGES_BUILDROOT/synfig.git ]; then
			if [[ $WORKDIR_IS_REPO == 1 ]]; then
				git clone $SYNFIG_REPO_DIR $PACKAGES_BUILDROOT/synfig.git
				sed -i 's|url = .*|url = git://github.com/synfig/synfig.git|' $PACKAGES_BUILDROOT/synfig.git/.git/config
			else
				git clone git://github.com/synfig/synfig.git $PACKAGES_BUILDROOT/synfig.git
			fi
		fi
		pushd $PACKAGES_BUILDROOT
		cd synfig.git && git fetch && cd ..
		#[ ! -e git-$GITVERSION.tar.bz2 ] && wget -c http://kernel.org/pub/software/scm/git/git-$GITVERSION.tar.bz2
		rsync -av rsync://download.tuxfamily.org/pub/synfig/packages/sources/base/ ./
		if [[ $OPENGL == 1 ]]; then
			rsync -av rsync://download.tuxfamily.org/pub/synfig/packages/sources/opengl/ ./
		fi
		popd
		#copy sources
		[ -d $PACKAGES_BUILDROOT.$ARCH/source ] || mkdir -p $PACKAGES_BUILDROOT.$ARCH/source
		cp -rf $PACKAGES_BUILDROOT/* $PACKAGES_BUILDROOT.$ARCH/source/

		#set up the /proc link
		echo "Mounting proc..."
		if ! ( mount | egrep "proc on $PACKAGES_BUILDROOT.${ARCH}/proc" ); then
			mount -o bind /proc $PACKAGES_BUILDROOT.$ARCH/proc
			echo "   Done."
		else
			echo "   Already mounted. Skipping."
		fi

		#go to chroot
		$SETARCH chroot $PACKAGES_BUILDROOT.$ARCH env http_proxy=$http_proxy bash /build.sh package $SELECTEDREVISION

		umount $PACKAGES_BUILDROOT.$ARCH/proc || true

		mv -f $PACKAGES_BUILDROOT.$ARCH/packages/* $PACKAGES_PATH
		done
		echo
		echo
		find $PACKAGES_PATH/synfigstudio* -maxdepth 1
		echo "     DONE BUILDING PACKAGES INTO $HOME/synfig-packages"
		echo
	fi
}

###=================================== MAIN ======================================

if [ -z $1 ]; then
	ARG='standart'
else
	ARG=$1
fi

case $ARG in
	full)
		MODE='full'
		initialize
		mk
		exit;;
	standart)
		MODE='standart'
		initialize
		mk
		exit;;
	quick)
		MODE='quick'
		initialize
		mk
		exit;;
	package)
		MODE='package'
		initialize
		mk$ARG
		exit;;
	*)
		if [ -e /etc/chroot.id ]; then
			MODE='package'
		else
			MODE='standart'
		fi
		initialize
		mk$ARG
		exit;;
esac
