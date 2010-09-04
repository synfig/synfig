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

RELEASE=1
PREFIX=$HOME/synfig/

PACKAGES_PATH=$HOME/synfig-packages     	# path where to write packages files
PACKAGES_BUILDROOT=/tmp/synfig-buildroot	# path of for build infrastructure
MAKE_THREADS=2					#count of threads for make

# full = clean, configure, make
# standart = configure, make
# quick = make
# package = chroot, clean, configure, make
MODE='standart'
OPENGL=0
DEBUG=0
BREED=

LIBSIGCPP=2.0.17
GLEW=1.5.1
CAIROMM=1.2.4
IMAGEMAGICK=6.4.0
#GTKMM=2.10.9
GTKMM=2.8.12
FTGL=2.1.2
FREEGLUT=2.4.0
GTKGLEXT=1.2.0
GTKGLEXTMM=1.2.0
#LIBXMLPP=2.14.0
LIBXMLPP=2.20.0
#LIBXMLPP=2.23.2
GLIBMM=2.12.8
#GLIBMM=2.18.1
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
if [ ! -d ${PREFIX}/include/sigc++-2.0 ]; then
	pushd /source
	[ ! -d libsigc++-${LIBSIGCPP} ] && tar -xjf libsigc++-${LIBSIGCPP}.tar.bz2
	cd libsigc++-${LIBSIGCPP}
	#make clean || true
	./configure --prefix=${PREFIX}/ --includedir=${PREFIX}/include --disable-static --enable-shared 
	make -j$MAKE_THREADS 
	make install
	cd ..
	popd
fi
}

mkglibmm()
{
if [ ! -d ${PREFIX}/include/glibmm-2.4 ]; then
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
if [ ! -d ${PREFIX}/include/libxml++-2.6 ]; then
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
if [ ! -d ${PREFIX}/include/ImageMagick ]; then
	pushd /source
	[ ! -d ImageMagick-${IMAGEMAGICK} ] && tar -xjf ImageMagick-${IMAGEMAGICK}-10.tar.bz2 && cd ImageMagick-${IMAGEMAGICK} && patch -p1 < ../ImageMagick-6.4.0-multilib.patch && cd ..
	cd ImageMagick-${IMAGEMAGICK}
	#[[ $DOCLEAN == 1 ]] && make clean || true
	./configure --prefix=${PREFIX} --includedir=${PREFIX}/include --disable-static --enable-shared \
		--with-modules \
		--without-perl \
		--without-x \
		--with-threads \
		--with-magick_plus_plus \
		--without-gslib \
		--without-wmf \
		--without-lcms \
		--without-rsvg \
		--without-xml \
		--without-windows-font-dir \
		--without-dps \
		--without-djvu \
		--without-fpx \
		--without-jbig \
		--without-jp2 \
		--without-tiff \
		--without-fontconfig \
		--without-freetype \
		--without-png \
		--without-gvc \
		--without-openexr
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

mkcairomm()
{
if [ ! -d ${PREFIX}/include/cairomm-1.0 ]; then
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

mkgtkmm()
{
if [ ! -d ${PREFIX}/include/gtkmm-2.4 ]; then
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

export CXXFLAGS=-I${PREFIX}/include/ImageMagick

( [[ $MODE == 'package' ]] || [[ $MODE == 'full' ]] ) && make clean || true

if [[ $MODE != 'quick' ]]; then
	libtoolize --ltdl --copy --force
	sed -i 's/^AC_CONFIG_SUBDIRS(libltdl)$/m4_ifdef([_AC_SEEN_TAG(libltdl)], [], [AC_CONFIG_SUBDIRS(libltdl)])/' configure.ac || true
	sed -i 's/^# AC_CONFIG_SUBDIRS(libltdl)$/m4_ifdef([_AC_SEEN_TAG(libltdl)], [], [AC_CONFIG_SUBDIRS(libltdl)])/' configure.ac || true
	autoreconf --install --force
	/bin/sh ./configure --prefix=${PREFIX} --includedir=${PREFIX}/include --disable-static --enable-shared --with-magickpp --without-libavcodec $DEBUG  
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

if [[ $MODE != 'quick' ]]; then
	autoreconf --install --force
	/bin/sh ./configure --prefix=${PREFIX} --includedir=${PREFIX}/include --disable-static --enable-shared $DEBUG
fi

make -j$MAKE_THREADS
make install

for n in AUTHORS COPYING NEWS README
do
  	cp -f $n ${PREFIX}
done

if [ -e synfigstudio-cph-monitor ]; then
	cp -f synfigstudio-cph-monitor ${PREFIX}/bin/
	chmod a+x ${PREFIX}/bin/synfigstudio-cph-monitor
fi

popd
}

mkpack()
{
	[ -d /packages ] || mkdir /packages
	
	#== tar.bz2 ==
	TBZPREFIX=/tmp/synfigstudio-${VERSION}-${REVISION}.$BREED.$RELEASE.${ARCH}
	rm -rf $TBZPREFIX
	mkdir -p $TBZPREFIX
	cp -r  ${PREFIX}/* $TBZPREFIX
	
	if [[ $DEBUG == '--enable-debug' ]]; then
		GDB="which gdb && xterm -e  gdb -ex run -ex quit \$PREFIX/bin/synfig \$@ || "
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

$GDB\$PREFIX/bin/synfig \$@
EOF
	if [[ $DEBUG == '--enable-debug' ]]; then
		GDB="which gdb && xterm -e  gdb -ex run -ex quit \$PREFIX/bin/synfigstudio \$@ || "
	else
		GDB=''
	fi
	cat > $TBZPREFIX/synfigstudio <<EOF
#!/bin/sh

PREFIX=\`dirname \$0\`
PREFIX=\`cd \$PREFIX; pwd\`
export LD_LIBRARY_PATH=\${PREFIX}/lib:\$LD_LIBRARY_PATH
export SYNFIG_ROOT=\${PREFIX}/
export SYNFIG_MODULE_LIST=\${PREFIX}/etc/synfig_modules.cfg

$GDB\$PREFIX/bin/synfigstudio \$@
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
	rm -rf $TBZPREFIX/include
	rm -rf $TBZPREFIX/lib/gdkmm-2.4
	rm -rf $TBZPREFIX/lib/libxml++-2.6
	rm -rf $TBZPREFIX/lib/glibmm-2.4
	rm -rf $TBZPREFIX/lib/gtkmm-2.4
	rm -rf $TBZPREFIX/lib/pkgconfig
	rm -rf $TBZPREFIX/lib/sigc++-2.0
	rm -rf $TBZPREFIX/share/doc
	rm -rf $TBZPREFIX/share/aclocal
	rm -rf $TBZPREFIX/share/ImageMagick-6.4.0
	rm -rf $TBZPREFIX/share/man
	
	rm -f /packages/synfigstudio-${VERSION}-${REVISION}.$BREED.$RELEASE.${ARCH}.tar.bz2
	pushd $TBZPREFIX/../
	tar cjf /packages/synfigstudio-${VERSION}-${REVISION}.$BREED.$RELEASE.${ARCH}.tar.bz2 synfigstudio-${VERSION}-${REVISION}.$BREED.$RELEASE.${ARCH}
	popd
	rm -rf $TBZPREFIX
	
	#== rpm ==
	RPMREVISION=`echo $REVISION.$BREED | sed s/-/_/`
    cat > synfigstudio.spec << EOF
%define __spec_install_post /bin/true

Name:           synfigstudio
Version:        ${VERSION}
Release:        ${RPMREVISION}.${RELEASE}
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
It is designed to be capable of producing feature-film quality
animation. It eliminates the need for tweening, preventing the
need to hand-draw each frame. Synfig features spatial and temporal
resolution independence (sharp and smoothat any resolution or framerate),
high dynamic range images, and a flexible plugin system.


%prep


%build

%install
rm -rf \$RPM_BUILD_ROOT
mkdir -p \$RPM_BUILD_ROOT/${PREFIX}
cp -r  ${PREFIX}/* \$RPM_BUILD_ROOT/${PREFIX}
mkdir -p \$RPM_BUILD_ROOT/usr/share
mv \$RPM_BUILD_ROOT/${PREFIX}/share/applications \$RPM_BUILD_ROOT/usr/share
mv \$RPM_BUILD_ROOT/${PREFIX}/share/icons \$RPM_BUILD_ROOT/usr/share
mkdir -p \$RPM_BUILD_ROOT/usr/share/mime-info
ln -sf ${PREFIX}/share/mime-info/synfigstudio.keys \$RPM_BUILD_ROOT/usr/share/mime-info/synfigstudio.keys
ln -sf ${PREFIX}/share/mime-info/synfigstudio.mime \$RPM_BUILD_ROOT/usr/share/mime-info/synfigstudio.mime
mkdir -p \$RPM_BUILD_ROOT/usr/share/pixmaps
ln -sf ${PREFIX}/share/pixmaps/sif_icon.png \$RPM_BUILD_ROOT/usr/share/pixmaps/sif_icon.png
ln -sf ${PREFIX}/share/pixmaps/synfig_icon.png \$RPM_BUILD_ROOT/usr/share/pixmaps/synfig_icon.png
mkdir -p \$RPM_BUILD_ROOT/usr/bin
mv \$RPM_BUILD_ROOT/${PREFIX}/bin/synfig \$RPM_BUILD_ROOT/usr/bin/
mv \$RPM_BUILD_ROOT/${PREFIX}/bin/synfigstudio \$RPM_BUILD_ROOT/usr/bin/

if [ -e \$RPM_BUILD_ROOT/${PREFIX}/bin/synfigstudio-cph-monitor ]; then
mv \$RPM_BUILD_ROOT/${PREFIX}/bin/synfigstudio-cph-monitor \$RPM_BUILD_ROOT/usr/bin/
cat > \$RPM_BUILD_ROOT/usr/share/applications/synfigstudio-cph-monitor.desktop << EOD
[Desktop Entry]
Encoding=UTF-8
Name=Synfig Studio CPH monitor
Comment=This application collecting statistics about synfig crashes
Exec=synfigstudio-cph-monitor
Icon=terminal.png
Terminal=true
Type=Application
Categories=Graphics;Application;
X-Desktop-File-Install-Version=0.15
EOD
fi

#cleaning devel stuff
rm -f \$RPM_BUILD_ROOT/${PREFIX}/lib/*.la
rm -f \$RPM_BUILD_ROOT/${PREFIX}/lib/*.a
rm -rf \$RPM_BUILD_ROOT/${PREFIX}/bin
rm -rf \$RPM_BUILD_ROOT/${PREFIX}/include
rm -rf \$RPM_BUILD_ROOT/${PREFIX}/lib/gdkmm-2.4
rm -rf \$RPM_BUILD_ROOT/${PREFIX}/lib/libxml++-2.6
rm -rf \$RPM_BUILD_ROOT/${PREFIX}/lib/glibmm-2.4
rm -rf \$RPM_BUILD_ROOT/${PREFIX}/lib/gtkmm-2.4
rm -rf \$RPM_BUILD_ROOT/${PREFIX}/lib/pkgconfig
rm -rf \$RPM_BUILD_ROOT/${PREFIX}/lib/sigc++-2.0
rm -rf \$RPM_BUILD_ROOT/${PREFIX}/share/doc
rm -rf \$RPM_BUILD_ROOT/${PREFIX}/share/aclocal
rm -rf \$RPM_BUILD_ROOT/${PREFIX}/share/ImageMagick-6.4.0
rm -rf \$RPM_BUILD_ROOT/${PREFIX}/share/man


%clean
rm -rf \$RPM_BUILD_ROOT

%post

%postun

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
    
    #cp /usr/src/redhat/RPMS/$ARCH/synfigstudio-${VERSION}-${RPMREVISION}.$RELEASE.${ARCH}.rpm ../
    cp /usr/src/rpm/RPMS/$ARCH/synfigstudio-${VERSION}-${RPMREVISION}.$RELEASE.${ARCH}.rpm /packages/
    pushd /packages/
    alien -k synfigstudio-${VERSION}-${RPMREVISION}.$RELEASE.${ARCH}.rpm
    rm -rf synfigstudio-${VERSION}
    popd
}

initialize()
{
	# Make sure we have all dependencies installed
	echo "Checking dependencies..."
	DEB_LIST_MINIMAL="build-essential autoconf automake libltdl3-dev libtool gettext cvs libpng12-dev libjpeg62-dev libfreetype6-dev libfontconfig1-dev libgtk2.0-dev libxml2-dev bzip2"
	if which yum >/dev/null; then
		PKG_LIST="git"
		if [[ $MODE == 'package' ]]; then
			PKG_LIST="${PKG_LIST} debootstrap rsync"
		else
			PKG_LIST="${PKG_LIST} libpng-devel libjpeg-devel freetype-devel fontconfig-devel atk-devel pango-devel cairo-devel gtk2-devel gettext-devel libxml2-devel libxml++-devel gcc-c++ autoconf automake libtool libtool-ltdl-devel cvs"
			PKG_LIST="${PKG_LIST} OpenEXR-devel libmng-devel ImageMagick-c++-devel gtkmm24-devel glibmm24-devel"
		fi
		if ! ( rpm -qv $PKG_LIST ); then
			echo "Running yum (you need root privelegies to do that)..."
			su -c "yum install $PKG_LIST"
		fi
	elif which apt-get >/dev/null; then
		PKG_LIST="git-core"
		if [[ $MODE == 'package' ]]; then
			if [[ `cat /etc/chroot.id` == "Synfig Packages Buildroot" ]]; then
				#we are inside of chroot
				PKG_LIST="$DEB_LIST_MINIMAL rpm alien xsltproc wget python"
			else
				#we have to prepare chroot
				PKG_LIST="${PKG_LIST} debootstrap rsync"
			fi
		else
			PKG_LIST="${PKG_LIST} ${DEB_LIST_MINIMAL} libmng-dev libgtkmm-2.4-dev libglibmm-2.4-dev libsigc++-2.0-dev"
		fi
		if ! ( dpkg -s $PKG_LIST >/dev/null ); then
			echo "Running apt-get (you need root privelegies to do that)..."
			echo 
			#echo "http_proxy =====" $http_proxy
			#env
			sudo apt-get update || true
			sudo apt-get install -y $PKG_LIST
		fi
	else
		if [[ $MODE == 'package' ]]; then
			if ! ( which git && which debootstrap ) ; then
				echo "ERROR: Please install 'git' and 'debootstrap'."
				exit;
			fi
		else
			echo "WARNING: This build script does not works with package mangement systems other than rpm/yum or apt/dpkg! You should install dependent packages manually."
			read
		fi
	fi
	echo "Done."
	
	if [[ $DEBUG == 1 ]]; then
		DEBUG='--enable-debug'
	else
		DEBUG=''
	fi
	
	
	
	if [[ $MODE == 'package' ]] && [[ `cat /etc/chroot.id` == "Synfig Packages Buildroot" ]]; then
		SYNFIG_REPO_DIR="/source/synfig.git"
		PREFIX="/opt/synfig"
		
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
		[[ $DEBUG == 1 ]] && BREED=${BREED}_debug
		REVISION=`git show --pretty=medium $SELECTEDREVISION | head -n 3 |tail -n 1 | sed 's/Date: *//' | sed 's/ +.*//'`
		REVISION=`date --date="${REVISION}" +%Y%m%d`
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
		if git rev-parse --git-dir >/dev/null; then
			SYNFIG_REPO_DIR=$(dirname `git rev-parse --git-dir`)
			WORKDIR_IS_REPO=1
		fi
	fi
	
	export PKG_CONFIG_PATH=${PREFIX}/lib/pkgconfig
	export PATH=${PREFIX}/bin:$PATH
	export LD_LIBRARY_PATH=${PREFIX}/lib:$LD_LIBRARY_PATH
	export LDFLAGS="-Wl,-rpath -Wl,\\\$\$ORIGIN/lib"
}

mk()
{
	if [[ WORKDIR_IS_REPO == 0 ]]; then
		SYNFIG_REPO_DIR=`pwd`/synfig.git/
		git clone git://synfig.git.sourceforge.net/gitroot/synfig/synfig ${SYNFIG_REPO_DIR}
	fi
	
	mkETL
	mksynfig
	mksynfigstudio
}

mkpackage()
{
	#check if we already in chroot
	if [[ `cat /etc/chroot.id` == "Synfig Packages Buildroot" ]]; then
		echo "We are in chroot now."
		
		echo "[user]"  > /root/.gitconfig
		echo "email = packages@synfig.org"  >> /root/.gitconfig
		echo "name = Synfig Packager" >> /root/.gitconfig
		
		
		
		#synfig-core deps
		mklibsigcpp
		mkglibmm
		mklibxmlpp
		if [[ $OPENGL == 1 ]]; then
			mkglew
		fi
		mkimagemagick
		#synfig-studio deps
		mkcairomm
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
		#DEB_LIST="build-essential,autoconf,automake,libltdl3-dev,libtool,gettext,cvs,libpng12-dev,libjpeg62-dev,libfreetype6-dev,libfontconfig1-dev,libgtk2.0-dev,libxml2-dev,bzip2,rpm,alien,xsltproc"
		for ARCH in amd64 i386; do
		if [[ $ARCH == 'i386' ]];then
			SETARCH='linux32'
		else
			SETARCH='linux64'
		fi
		if ! [ -e $PACKAGES_BUILDROOT.$ARCH/etc/chroot.id ]; then
			debootstrap --arch=$ARCH --variant=buildd  --include=sudo etch $PACKAGES_BUILDROOT.$ARCH http://archive.debian.org/debian
		fi
		#set chroot ID
		echo "Synfig Packages Buildroot" > $PACKAGES_BUILDROOT.$ARCH/etc/chroot.id
		cp -f $0 $PACKAGES_BUILDROOT.$ARCH/build.sh
		#keep proxy settings
		if ! [ -z $http_proxy ]; then 
			#echo "export http_proxy=\"$http_proxy\";" >> $PACKAGES_BUILDROOT.$ARCH/root/.bashrc
			#echo "echo 'proxy export done';" >> $PACKAGES_BUILDROOT.$ARCH/root/.bashrc
			echo "Acquire::http::Proxy \"$http_proxy\";" > $PACKAGES_BUILDROOT.$ARCH/etc/apt/apt.conf
		fi
		#fetch sources to cache
		[ -d $PACKAGES_BUILDROOT.$ARCH/source ] && rm -rf $PACKAGES_BUILDROOT.$ARCH/source || true
		if ! [ -d $PACKAGES_BUILDROOT/synfig.git ]; then
			if [[ $WORKDIR_IS_REPO == 1 ]]; then
				git clone $SYNFIG_REPO_DIR $PACKAGES_BUILDROOT/synfig.git
				sed -i 's|url = .*|url = git://synfig.git.sourceforge.net/gitroot/synfig/synfig|' $PACKAGES_BUILDROOT/synfig.git/.git/config
			else
				git clone git://synfig.git.sourceforge.net/gitroot/synfig/synfig $PACKAGES_BUILDROOT/synfig.git
			fi
		fi
		pushd $PACKAGES_BUILDROOT
		cd synfig.git && git fetch && cd ..
		[ ! -e git-$GITVERSION.tar.bz2 ] && wget -c http://kernel.org/pub/software/scm/git/git-$GITVERSION.tar.bz2
		for FILE in \
			cairomm-${CAIROMM}.tar.gz \
			glibmm-${GLIBMM}.tar.bz2 \
			gtkmm-${GTKMM}.tar.bz2 \
			libsigc++-${LIBSIGCPP}.tar.bz2 \
			libxml++-${LIBXMLPP}.tar.bz2 \
			ImageMagick-6.3.8-invalid-gerror-use.patch \
			ImageMagick-${IMAGEMAGICK}-10.tar.bz2 \
			ImageMagick-${IMAGEMAGICK}-multilib.patch \
			ImageMagick-${IMAGEMAGICK}-xdg-open.patch
		do
			#[ ! -e $FILE ] && wget -c http://download.tuxfamily.org/morevna/morevnapackage/sources/$FILE
			rsync -av rsync://download.tuxfamily.org/pub/synfig/packages/sources/$FILE ./
		done
		if [[ $OPENGL == 1 ]]; then
		for FILE in \
			glew-${GLEW}.tar.gz\
			glew-${GLEW}-makefile.patch \
			freeglut-${FREEGLUT}.tar.gz \
			ftgl-${FTGL}.tar.gz \
			ftgl-${FTGL}-destdir.patch \
			ftgl-${FTGL}-Glyph-g++_41.patch \
			ftgl-${FTGL}-pc_req.patch \
			ftgl-${FTGL}-rpath_FTGLDemo.patch \
			ftgl-${FTGL}-ttf_font.patch \
			gtkglext-${GTKGLEXT}.tar.bz2 \
			gtkglextmm-${GTKGLEXTMM}.tar.bz2 \
			gtkglextmm-${GTKGLEXTMM}-aclocal.diff
		do
			#[ ! -e $FILE ] && wget -c http://download.tuxfamily.org/morevna/morevnapackage/sources/$FILE
			rsync -av rsync://download.tuxfamily.org/pub/synfig/packages/sources/$FILE ./
		done
		fi
		popd
		#copy sources
		[ -d $PACKAGES_BUILDROOT.$ARCH/source ] || mkdir -p $PACKAGES_BUILDROOT.$ARCH/source
		cp -rf $PACKAGES_BUILDROOT/* $PACKAGES_BUILDROOT.$ARCH/source/
		#go to chroot
		$SETARCH chroot $PACKAGES_BUILDROOT.$ARCH env http_proxy=$http_proxy bash /build.sh package $SELECTEDREVISION
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
		MODE='package'
		initialize
		mk$ARG
		exit;;
esac
