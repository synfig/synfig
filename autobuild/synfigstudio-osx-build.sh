#!/bin/bash
#
# SynfigStudio mac package build script
# Copyright (c) 2008-2009 Simone Karin Lehmann (GimpOSX parts)
# Copyright (c) 2012-2013 Konstantin Dmitriev
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
#
#
# = Usage notes =
# 
# * You need to have XCode and git installed
# * Builds from current repository, current revision. So you should manually checkout the desired revision to build
# * If no repository found - then  sources fetched into ~/src/synfig and the latest master branch is built
# * Executing script without arguments makes a full clean build and produces dmg package
# * You can pass arguments to the script to invoke particular stage. 
#	Available stages: mkmacports, mkdeps, mketl, mksynfig, mksynfigstudio, mkapp, mkdmg
#	Example: 
#		synfigstudio-osx-build.sh mkdeps
# * You can pass a custom command to be invoked in the build environment.
#	Example (uninstalls glibmm package from the buildroot):
#		synfigstudio-osx-build.sh port uninstall glibmm
# * If you suspect something is wrong with your macports installation you can always start from scratch by removing ~/src/macports dir



#======= HEADER ===========

set -e

export RELEASE=1
export BUILDROOT_VERSION=1

if [ `whoami` != "root" ]; then
	echo "Please use sudo to run this script. Aborting."
	exit 1
fi

export SCRIPTPATH=$(cd `dirname "$0"`; pwd)

SYNFIG_REPO_DIR=$(dirname "$SCRIPTPATH")
SCRIPTDIR_IS_REPO=0

if [ -e "$SYNFIG_REPO_DIR/build.conf" ]; then
	source "$SYNFIG_REPO_DIR/build.conf"
fi

if [ ! -z $UNIVERSAL ] && [[ ! $UNIVERSAL == 0 ]]; then
export BUILDDIR=~/synfig-buildroot/build.u
else
export BUILDDIR=~/synfig-buildroot/build
export UNIVERSAL=0
fi

if [ ! -z $X11 ] && [[ ! $X11 == 0 ]]; then
	export X11=1
	export BUILDDIR=${BUILDDIR}.x11
fi

if [ ! -z $DEBUG ] && [[ ! $DEBUG == 0 ]]; then
	echo
	echo "Debug mode: enabled"
	echo
	DEBUG='--enable-debug --enable-optimization=0'
	export BUILDDIR=${BUILDDIR}.debug
else
	DEBUG=''
fi

LNKDIR=/tmp/skl/SynfigStudio
MACPORTS=$LNKDIR/Contents/Resources
MPSRC=MacPorts-2.3.3


export SYNFIG_PREFIX=${MACPORTS}/synfig/
export PATH="$MACPORTS/bin:${SYNFIG_PREFIX}/bin:$MACPORTS/sbin:$PATH"
export SYNFIG_MODULE_LIST=${SYNFIG_PREFIX}/etc/synfig_modules.cfg
export PKG_CONFIG_PATH=${MACPORTS}/lib/pkgconfig:${SYNFIG_PREFIX}/lib/pkgconfig$PKG_CONFIG_PATH
export LD_LIBRARY_PATH=${MACPORTS}/lib:${SYNFIG_PREFIX}/lib:${SYNFIG_PREFIX}/lib64:$LD_LIBRARY_PATH

#export ACLOCAL_FLAGS="-I ${SYNFIG_PREFIX}/share/aclocal -I ${MACPORTS}/share/aclocal"
#export CPPFLAGS="-fpermissive -I${MACPORTS}/include -I${SYNFIG_PREFIX}/include"
export CPPFLAGS="-I${MACPORTS}/include -I${SYNFIG_PREFIX}/include"
export LDFLAGS="-L${MACPORTS}/lib -L${SYNFIG_PREFIX}/lib"
if [[ ! $UNIVERSAL == 0 ]]; then
export CFLAGS="-arch i386 -arch x86_64"
export CXXFLAGS="-arch i386 -arch x86_64"
export LDFLAGS="$LDFLAGS -arch i386 -arch x86_64"
fi

#======= HEADER END ===========



prepare()
{
	# == workarounds ==
	
	#We should do that, otherwise python won't build:
	if [ -e $MACPORTS/tmp/app ]; then
		[ ! -e $MACPORTS/tmp/app.bak ] || rm -rf $MACPORTS/tmp/app.bak
		echo "Backing up custom /Applications/MacPorts dir..."
		mv $MACPORTS/tmp/app $MACPORTS/tmp/app.bak || true
	fi
	
	# Cleanup some stuff, remaining from older installations
	rm /Library/LaunchDaemons/org.macports.rsyncd.plist || true
	
	# == symlinks ==
	
	if [ ! -e "$BUILDDIR" ]; then
	  mkdir -p "$BUILDDIR"
	fi

	echo -n setting symlink to build directory...
	test -d /tmp/skl || mkdir -p /tmp/skl
	chmod a+w /tmp/skl
	test -L $LNKDIR && rm $LNKDIR
	test -L $MACPORTS && rm $MACPORTS
	mkdir -p `dirname $MACPORTS`
	ln -s "$BUILDDIR" $MACPORTS
	chmod a+w $MACPORTS
	echo
}

mkmacports()
{

# cleanup previous installation
if [ -e "$BUILDDIR/" ]; then
  rm -rf "$BUILDDIR/"
fi

mkdir -p "$BUILDDIR/"
mkdir -p ~/src
cd ~/src

# compile MacPorts and do a selfupdate
if [ ! -d "$MPSRC" ]; then
  #curl -LO http://svn.macports.org/repository/macports/downloads/$MPSRC/$MPSRC.tar.bz2
  curl -LO http://distfiles.macports.org/MacPorts/$MPSRC.tar.bz2
  echo -n extracting MacPorts sources ...
  bunzip2 $MPSRC.tar.bz2
  tar -xf $MPSRC.tar
  rm $MPSRC.tar
  echo done.
fi

# this is the main part on which this building and packaging process relies on
echo Compiling and updating MacPorts...
cd $MPSRC
./configure --prefix $MACPORTS --with-tclpackage=$MACPORTS/share/macports/Tcl \
	--with-install-user=`id -un` \
	--with-install-group=`id -gn`
make clean
make -j$JOBS
#sudo make install
make install
cd - > /dev/null
export PATH="$MACPORTS/bin:$MACPORTS/sbin:$PATH"

# give our build directory user perms so we don't need sudo in further steps. Avoid sudo whenever it's possible!
#sudo chown -R $UID "$BUILDDIR"

# do a selfupdate
echo Now selfupdating MacPorts. Please wait ...
port -d selfupdate

# now give some hints
echo
echo "Yor MacPorts installation is now ready to build SynfigStudio for Mac OS X."
}

mkdeps()
{
	# test if we have MacPorts  installed
	if [ ! -e "$MACPORTS/bin/port" ]; then
		mkmacports
	fi

	if [ -e "$MACPORTS/bin/synfig" ]; then
		echo "=========================== !!! =========================="
		echo "   Macports installation is outdated. Force rebuild..."
		echo "=========================== !!! =========================="
		sleep 10
		rm -rf "$MACPORTS"
		mkmacports
	fi

	echo $BUILDROOT_VERSION > "$MACPORTS/etc/buildroot-id"
	if [[ `cat "$MACPORTS/etc/buildroot-id"` != "${BUILDROOT_VERSION}" ]]; then
		echo "======================= !!! ======================"
		echo "   Buildroot version changed. Force rebuild..."
		echo "======================= !!! ======================"
		sleep 10
		rm -rf "$MACPORTS"
		mkmacports
	fi
	
	# Don't write into /Applications/MacPorts
	[ -d $MACPORTS/tmp/app ] || mkdir -p $MACPORTS/tmp/app
	sed -i "" -e "s|/Applications/MacPorts|$MACPORTS/tmp/app|g" "$MACPORTS/etc/macports/macports.conf" || true
	
	if [[ ! $UNIVERSAL == 0 ]]; then
		if [[ $X11 == 1 ]]; then
			echo "+universal +x11 +nonfree" > $MACPORTS/etc/macports/variants.conf
		else
			echo "+universal +no_x11 +quartz -x11 +nonfree" > $MACPORTS/etc/macports/variants.conf
		fi
	else
		if [[ $X11 == 1 ]]; then
			echo "+x11 +nonfree" > $MACPORTS/etc/macports/variants.conf
		else
			echo "+no_x11 +quartz -x11 +nonfree" > $MACPORTS/etc/macports/variants.conf
		fi
	fi
	
	pushd ${SCRIPTPATH}/macports
	portindex
	popd
	echo "file://${SCRIPTPATH}/macports [nosync]" > $MACPORTS/etc/macports/sources.conf
	echo "rsync://rsync.macports.org/release/tarballs/ports.tar [default]" >> $MACPORTS/etc/macports/sources.conf
	
	# workaround the bug introduced in MacPorts 2.2.0 - https://trac.macports.org/ticket/39850
	cp -rf $MACPORTS/etc/macports/macports.conf $MACPORTS/etc/macports/macports.conf.bak
	sed '/sandbox_enable/d' $MACPORTS/etc/macports/macports.conf.bak > $MACPORTS/etc/macports/macports.conf
	echo "sandbox_enable no" >> $MACPORTS/etc/macports/macports.conf

	port selfupdate
	
	# We have to make sure python 2 is default, because some packages won't build with python 3
	port select --set python python27 || true
	
	port upgrade outdated || true

	CORE_DEPS=" \
		fontconfig \
		freetype \
		gettext \
		ImageMagick \
		libmng \
		libpng \
		libsigcxx2 \
		libtool \
		libxmlxx2 \
		mlt \
		openexr \
		pkgconfig \
		ffmpeg \
		boost \
		cairo \
		libtool"
	STUDIO_DEPS=" \
		gtkmm3 \
		python33 \
		gnome-themes-standard \
		intltool"
	port install -f $CORE_DEPS $STUDIO_DEPS
	
	# We have to make sure python 2 is default, because some packages won't build with python 3
	port select --set python python27

	# ...but we still need python3 binary available
	pushd $MACPORTS/bin/ > /dev/null
	ln -sf python3.3 python3
	popd > /dev/null
	
	cp ${SCRIPTPATH}/gtk-3.0/settings.ini ${MACPORTS}/etc/gtk-3.0/

}

mketl()
{
	# building ETL
	pushd ${SYNFIG_REPO_DIR}/ETL
	rm -f aclocal.m4
	autoreconf --install --force
	make clean || true
	./configure --prefix=${SYNFIG_PREFIX}  --includedir=${SYNFIG_PREFIX}/include
	make -j$JOBS install
	popd
}

libtoolize()
{
	glibtoolize $@
}

mksynfig()
{
	# building synfig-core
	pushd ${SYNFIG_REPO_DIR}/synfig-core
	export CXXFLAGS="$CXXFLAGS -I${SYNFIG_PREFIX}/include/ImageMagick"
	make clean || true
	libtoolize --ltdl --copy --force
	sed -i 's/^AC_CONFIG_SUBDIRS(libltdl)$/m4_ifdef([_AC_SEEN_TAG(libltdl)], [], [AC_CONFIG_SUBDIRS(libltdl)])/' configure.ac || true
	sed -i 's/^# AC_CONFIG_SUBDIRS(libltdl)$/m4_ifdef([_AC_SEEN_TAG(libltdl)], [], [AC_CONFIG_SUBDIRS(libltdl)])/' configure.ac || true
	autoreconf --install --force
	if [[ ! $UNIVERSAL == 0 ]]; then
	export DEPTRACK="--disable-dependency-tracking"
	fi
	/bin/sh ./configure ${DEPTRACK} --prefix=${SYNFIG_PREFIX} --includedir=${SYNFIG_PREFIX}/include --disable-static --enable-shared --without-opengl --with-magickpp --without-libavcodec --with-boost=${MACPORTS} ${DEBUG}
	make -j$JOBS install
	popd
}

mksynfigstudio()
{
	# Copy launch script, so we can test synfigstudio without building an app package
	[ ! -e "${MACPORTS}/../MacOS" ] || rm -rf "${MACPORTS}/../MacOS"
	cp -rf $SCRIPTPATH/app-template/Contents/MacOS ${MACPORTS}/../MacOS
	
	# building synfig-studio
	pushd ${SYNFIG_REPO_DIR}/synfig-studio

	make clean || true
	CONFIGURE_PACKAGE_OPTIONS='--disable-update-mimedb'
	/bin/sh ./bootstrap.sh
	if [[ ! $UNIVERSAL == 0 ]]; then
	export DEPTRACK="--disable-dependency-tracking"
	fi
	/bin/sh ./configure ${DEPTRACK}  --prefix=${SYNFIG_PREFIX} --includedir=${SYNFIG_PREFIX}/include --disable-static --enable-shared $DEBUG $CONFIGURE_PACKAGE_OPTIONS
	make -j$JOBS install

	for n in AUTHORS COPYING NEWS README
	do
	  	cp -f $n ${MACPORTS}
	done
	
	popd
}

mkapp()
{
	#VERSION=`eval "synfig" --version 2>&1 | cut -d " " -f 2`
	VERSION=`get_version_release_string`
	echo Now trying to build your new SynfigStudio app ...

	DIR=`dirname "$BUILDDIR"`
	SYNFIGAPP="$DIR/SynfigStudio-new-app/Contents/Resources"

	# initial cleanup
	[ ! -e $DIR/SynfigStudio-new-app ] || rm -rf $DIR/SynfigStudio-new-app
	[ ! -e $DIR/SynfigStudio.app ] || rm -rf $DIR/SynfigStudio.app

	cp -R "$SCRIPTPATH/app-template" "$DIR/SynfigStudio-new-app"
	mv $DIR/SynfigStudio-new-app/Contents/MacOS/synfigstudio $DIR/SynfigStudio-new-app/Contents/MacOS/SynfigStudio || true

	#cd "$SCRIPTPATH"/LauncherCode
	#xcodebuild -configuration Deployment
	#cd -
	#cp -R "$SCRIPTPATH/LauncherCode/build/Deployment/ScriptExec.app/Contents/Resources"  "$SYNFIGAPP"
	#mkdir -p "$DIR/SynfigStudio-new-app/Contents/MacOS"
	#cp -R "$SCRIPTPATH/LauncherCode/build/Deployment/ScriptExec.app/Contents/MacOS/ScriptExec "$SYNFIGAPP/../MacOS/SynfigStudio"

	# copy binaries from gimp-launcher 
	# In MacPorts 1.7 there will be a config option
	#cp "$MP/Applications/ScriptExec.app/Contents/MacOS/ScriptExec" "$GIMPAPP/../MacOS/Gimp"
	#cp -R "$MP/Applications/ScriptExec.app/Contents/Resources/MenuBar.nib" "$GIMPAPP"

	# copy all data
	echo copying data ...
	cp -R $MACPORTS/bin "$SYNFIGAPP/bin"
	cp -R $MACPORTS/etc "$SYNFIGAPP/etc"
	cp -R $MACPORTS/lib "$SYNFIGAPP/lib"
	cp -R $MACPORTS/Library "$SYNFIGAPP/Library"
	cp -R $MACPORTS/synfig "$SYNFIGAPP/synfig"
	cp -R $MACPORTS/share "$SYNFIGAPP/share"

	# Cleaning

	# remove svn meta data
	cd "$DIR"
	find "SynfigStudio-new-app" -name ".svn" -print0 | xargs -0 rm -rf
	find "SynfigStudio-new-app" -name ".DS_Store" -print0 | xargs -0 rm -rf

	#echo cleaning up some stuff in lib directories ...
	cd "$SYNFIGAPP"
	rm -rf lib/gtk-2.0/include
	rm -rf lib/python2.7/test
	rm -rf lib/python2.7/*/test
	rm -rf Library/Frameworks/Python.framework/Versions/2.*
	find lib \( -name "*.la" -or -name "*.a" \) -not -path "lib/ImageMagick*"  -delete
	find . -name "*.pyo" -delete

	echo cleaning up some stuff in share ...
	cd "$SYNFIGAPP/share"
	rm -rf gutenprint/doc
	rm -rf gutenprint/samples
	rm -rf ghostscript/fonts
	rm -rf ghostscript/*/doc
	rm -rf ghostscript/*/examples

	echo cleaning up locales ...
	find locale \( \! -name "gtk*" -and \! -name "synfig*" \! -name "gutenprint*" \) -delete


	# app bundle files
	echo "*** Please do _NOT_ delete this file. The file script depends on it. ***" > "$SYNFIGAPP/v$VERSION"
	sed -i "" -e "s/_VERSION_/$VERSION/g" "$SYNFIGAPP/../MacOS/SynfigStudio" 
	sed -i "" -e "s/_VERSION_/$VERSION/g" "$SYNFIGAPP/../Info.plist" 

	# save information about the ports which make up this build
	echo "Synfig Studio $VERSION for Mac OS X $OSNAME" > "$SYNFIGAPP/build-info.txt"
	date >> "$SYNFIGAPP/build-info.txt"
	port installed >> "$SYNFIGAPP/build-info.txt"
	sed -i "" -e "s/are currently installed:/were used to build this package:/g" "$SYNFIGAPP/build-info.txt"

	mv "$DIR/SynfigStudio-new-app" "$DIR/SynfigStudio.app"

	echo
	echo "Your new Synfig Studio app bundle should now be ready to run."
	echo
}

mkdmg()
{
	cd ~
	
	# get OS major version
	OSXVER=`uname -r | cut -f 1 -d '.'`

	#VERSION=`synfig --version 2>&1 | cut -d " " -f 2`
	VERSION=`get_version_release_string`
	#echo Synfig version is: $VERSION

	ARCH=`uname -m`
	if [[ ! $UNIVERSAL == 0 ]]; then
	export FINAL_FILENAME=synfigstudio-"$VERSION"
	else
	export FINAL_FILENAME=synfigstudio-"$VERSION"."$ARCH"
	fi
	

	VOLNAME="SynfigStudio"
	TRANSITORY_FILENAME="synfig-wla.sparseimage"
	
	APPDIR=`dirname "$BUILDDIR"`/SynfigStudio.app
	
	/usr/bin/hdiutil detach /Volumes/"$VOLNAME" || true

	echo "Creating and attaching disk image..."
	[ ! -e "$TRANSITORY_FILENAME" ] || rm -rf "$TRANSITORY_FILENAME"
	/usr/bin/hdiutil create -type SPARSE -size 3072m -fs "HFS+" -volname "$VOLNAME" -attach "$TRANSITORY_FILENAME"

	echo "Copying files to disk image..."
	cp -R $APPDIR /Volumes/"$VOLNAME"/SynfigStudio.app
	cp -R ${SYNFIG_REPO_DIR}/synfig-studio/COPYING /Volumes/"$VOLNAME"/LICENSE.txt
	mv /Volumes/"$VOLNAME"/SynfigStudio.app/Contents/MacOS/synfigstudio /Volumes/"$VOLNAME"/SynfigStudio.app/Contents/MacOS/SynfigStudio || true

	# open the window so that the icon database is generated
	open /Volumes/"$VOLNAME" || true
	sleep 3

	echo "Detaching disk image..."
	/usr/bin/hdiutil detach /Volumes/"$VOLNAME"

	echo "Compressing disk image..."
	[ ! -e "$FINAL_FILENAME" ] || rm -rf "$FINAL_FILENAME"
	/usr/bin/hdiutil convert -imagekey zlib-level=9 -format UDBZ "$TRANSITORY_FILENAME" -o ./"$FINAL_FILENAME"

	echo "Removing uncompressed transitory dmg..."
	/bin/rm -f "$TRANSITORY_FILENAME"

	echo "Done!"
}

get_version_release_string()
{
	pushd "$SYNFIG_REPO_DIR" > /dev/null
	VERSION=`cat synfig-core/configure.ac |egrep "AC_INIT\(\[Synfig Core\],"| sed "s|.*Core\],\[||" | sed "s|\],\[.*||"`
	#if [ -z $BREED ]; then
	#	BREED="`git branch -a --no-color --contains HEAD | sed -e s/\*\ // | sed -e s/\(no\ branch\)// | tr '\n' ' ' | tr -s ' ' | sed s/^' '//`"
	#	if ( echo $BREED | egrep origin/master > /dev/null ); then
	#		#give a priority to master branch
	#		BREED='master'
	#	else
	#		BREED=`echo $BREED | cut -d ' ' -f 1`
	#		BREED=${BREED##*/}
	#	fi
	#	BREED=${BREED%_master}
	#fi
	if [[ ${VERSION##*-RC} != ${VERSION} ]]; then
		#if [[ $BREED == 'master' ]]; then
			BREED=rc${VERSION##*-RC}
		#else
		#	BREED=rc${VERSION##*-RC}.$BREED
		#fi
		VERSION=${VERSION%%-*}
	fi
	if [ ! -z $BREED ]; then 
		BREED=`echo $BREED | tr _ . | tr - .`	# No "-" or "_" characters, becuse RPM and DEB complain
		BREED=.$BREED
	fi
	REVISION=`git show --pretty=format:%ci HEAD |  head -c 10 | tr -d '-'`
	echo "$VERSION-$REVISION$BREED"
	#echo "$VERSION-$REVISION$BREED.$RELEASE"
	popd >/dev/null
}

mkall()
{
	mkdeps

	# cleanup previous synfig installation if any
	[ ! -d "$SYNFIG_PREFIX" ] || rm -rf "$SYNFIG_PREFIX"

	mketl
	mksynfig
	mksynfigstudio

	#port install synfigstudio || true

	#synfig -q installer_logo.sif -o installer_logo.png --time 0
	#synfig(294) [23:51:01] info: Loading modules from /tmp/skl/#SynfigStudio.app/Contents/Resources/etc/synfig_modules.cfg
	#synfig: warning: Unable to find module "mod_magickpp" (file not found)
	#synfig: warning: Unable to find module "mod_libavcodec" (file not found)
	#synfig(294) [23:51:01] error: Standard Exception: basic_string::_S_construct NULL not valid
	#Unable to load 'installer_logo.sif'.
	#Throwing out job...
	#Nothing to do!

	#Workaround:
	#[ ! -e ~/src/macports/synfig-build ] || rm -rf ~/src/macports/synfig-build
	#cd $MACPORTS/var/macports/build/
	#DIRPATH=`ls`
	#cd -
	#VERSION=`synfig --version 2>&1 | cut -d " " -f 2`
	#cp -R "$MACPORTS/var/macports/build/$DIRPATH/synfigstudio/work/synfigstudio-$VERSION" ~/src/macports/synfig-build
	#cd ~/src/macports/synfig-build/images
	#make
	#cd -
	#cp -R ~/src/macports/synfig-build/images/* #"$MACPORTS/var/macports/build/$DIRPATH/synfigstudio/work/synfigstudio-$VERSION/images"
	#Workaround end

	#port install synfigstudio

	mkapp
	mkdmg

}

do_cleanup()
{
	#restore Applications/MacPorts dir
	if [ -e $MACPORTS/tmp/app.bak ]; then
		echo "Restoring custom /Applications/MacPorts dir."
		rm -rf $MACPORTS/tmp/app || true
		mv $MACPORTS/tmp/app.bak $MACPORTS/tmp/app
	fi
	rm -rf $LNKDIR || true
}

###=================================== MAIN ======================================

main()  # dummy for navigation
{ 
	true
}

#Init traps
trap do_cleanup INT

# number of jobs
export JOBS=`sysctl hw.ncpu | cut -f 2 -d " "`
echo "Detected processors count: $JOBS"

# get OS X version. 8=Tiger, 9=Leopard, 10=Snowleopard
export OS=`uname -r | cut -d "." -f1`
if [ $OS -eq 9 -o $OS -eq 10 ]; then
	GTKFONT=9.8
	THEME=Leopard
else
	GTKFONT=12.5
	THEME=Tiger
fi

# set OS code name
OSNAME=$THEME
if [ $OS -eq 10 ]; then
	OSNAME=Snowleopard
fi

if [ ! $OS -eq 13 ]; then # if not Mountain Lion then set compatibility mode
export MACOSX_DEPLOYMENT_TARGET=10.5
echo "Enabling binary compatibility mode: MACOSX_DEPLOYMENT_TARGET=10.5"
fi

prepare

# Fetch sources
#port install python33
#port select --set python python33
#port install git-core
if ! ( which git ); then
	echo "ERROR: No git found. Please install git from http://code.google.com/p/git-osx-installer/"
	exit 1
fi

#detecting repo
pushd $SCRIPTPATH >/dev/null
if git rev-parse --git-dir >/dev/null; then
	SYNFIG_REPO_DIR=$(dirname `git rev-parse --git-dir`)
	SCRIPTDIR_IS_REPO=1
fi
popd >/dev/null

if [ ! -e ${SYNFIG_REPO_DIR} ]; then
	pushd `dirname "${SYNFIG_REPO_DIR}"`
	git clone git://github.com/synfig/synfig.git
	popd
fi

VERSION=`get_version_release_string`

pushd ${SYNFIG_REPO_DIR} >/dev/null
if [[ WORKDIR_IS_REPO == 0 ]]; then
	git fetch
	git reset --hard HEAD
	SELECTEDREVISION=origin/master
	git checkout $SELECTEDREVISION
fi


echo
echo
echo "BUILDING synfigstudio-$VERSION"
echo
echo
sleep 5

popd > /dev/null

if [ -z $1 ]; then
	mkall
else
	echo "Executing custom user command..."
	$@
fi

do_cleanup
