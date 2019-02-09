#!/bin/bash
#


#======= HEADER ===========

set -e

export RELEASE=1
export BUILDROOT_VERSION=1

#if [ `whoami` != "root" ]; then
#	echo "Please use sudo to run this script. Aborting."
#	exit 1
#fi

export SCRIPTPATH=$(cd `dirname "$0"`; pwd)
export MACPORTS="/usr/local/opt" # dictated by HomeBrew defaults




#======= HEADER END ===========

smart_find()
{
	if [ -f "$1" ]; then
		#HomeBrew
		echo "$1"
	else
		#MacPorts
		BINARY=`basename "$1"`
		echo "$MACPORTS/bin/$BINARY"
	fi
}


mkapp()
{
	cd ${SCRIPTPATH}
	VERSION=`get_version_release_string`
	echo Now trying to build your new SynfigStudio app ...

	APPDIR="${SCRIPTPATH}/_production/SynfigStudio.app"
	#APPDIR_FINAL="${SCRIPTPATH}/_production/SynfigStudio.app"
	APPCONTENTS="${APPDIR}/Contents/Resources"

	# initial cleanup
	[ ! -d ${APPDIR} ] || rm -rf ${APPDIR}
	[ ! -d ${APPDIR_FINAL} ] || rm -rf ${APPDIR_FINAL}

	cp -Rf "${SCRIPTPATH}/autobuild/osx/app-template" "$APPDIR"
	mv ${APPDIR}/Contents/MacOS/synfigstudio $APPDIR/Contents/MacOS/SynfigStudio || true

	#mkdir -p "$APPCONTENTS/bin"
	#cp -R "$MACPORTS/bin/ffmpeg" "$APPCONTENTS/bin"
	#"$SCRIPTPATH/macos-gather-deps.sh" 	"$APPCONTENTS/bin/ffmpeg"
	#exit 0

	#cp -R "$MACPORTS/synfig/bin/synfig" "$APPCONTENTS/bin"
	mkdir -p "${APPCONTENTS}/etc"
	mkdir -p "${APPCONTENTS}/share"

	# Synfig
	"$SCRIPTPATH/autobuild/osx/relocate-binary.sh" "`smart_find ${SCRIPTPATH}/_production/build/bin/synfig`" "${SCRIPTPATH}/_production/build" "$APPCONTENTS"

	"$SCRIPTPATH/autobuild/osx/relocate-binary.sh" "`smart_find ${SCRIPTPATH}/_production/build/bin/synfigstudio`" "${SCRIPTPATH}/_production/build" "$APPCONTENTS"
	pushd "${SCRIPTPATH}/_production/build/lib/synfig/modules/"
	for FILE in `ls -1 *.so`; do
		"$SCRIPTPATH/autobuild/osx/relocate-binary.sh" "${SCRIPTPATH}/_production/build/lib/synfig/modules/$FILE" "${SCRIPTPATH}/_production/build" "$APPCONTENTS"
	done
	cp -R ${SCRIPTPATH}/_production/build/lib/synfig/modules/*.la  "$APPCONTENTS/lib/synfig/modules/"
	popd
	cp -R "${SCRIPTPATH}/_production/build/etc" "$APPCONTENTS/"
	cp -R "${SCRIPTPATH}/_production/build/share" "$APPCONTENTS/"

	"$SCRIPTPATH/autobuild/osx/relocate-binary.sh" "`smart_find $MACPORTS/ffmpeg/bin/ffmpeg`" "$MACPORTS" "$APPCONTENTS"
	"$SCRIPTPATH/autobuild/osx/relocate-binary.sh" "`smart_find $MACPORTS/ffmpeg/bin/ffprobe`" "$MACPORTS" "$APPCONTENTS"

	"$SCRIPTPATH/autobuild/osx/relocate-binary.sh" "`smart_find $MACPORTS/libdv/bin/encodedv`" "$MACPORTS" "$APPCONTENTS"
	"$SCRIPTPATH/autobuild/osx/relocate-binary.sh" "`smart_find $MACPORTS/sox/bin/sox`" "$MACPORTS" "$APPCONTENTS"

	# Gtk3
	"$SCRIPTPATH/autobuild/osx/relocate-binary.sh" "`smart_find $MACPORTS/gdk-pixbuf/bin/gdk-pixbuf-query-loaders`" "$MACPORTS" "$APPCONTENTS"
	"$SCRIPTPATH/autobuild/osx/relocate-binary.sh" "`smart_find $MACPORTS/gdk-pixbuf/bin/gdk-pixbuf-pixdata`" "$MACPORTS" "$APPCONTENTS"
	"$SCRIPTPATH/autobuild/osx/relocate-binary.sh" "`smart_find $MACPORTS/gtk+3/bin/gtk3-demo`" "$MACPORTS" "$APPCONTENTS"
	
	if [ -d "$MACPORTS/gdk-pixbuf" ]; then
		PKG_PREFIX="/gdk-pixbuf"
	else
		PKG_PREFIX=""
	fi
	pushd "${MACPORTS}${PKG_PREFIX}/lib/gdk-pixbuf-2.0/2.10.0/loaders/"
	for FILE in `ls -1 *.so`; do
		"$SCRIPTPATH/autobuild/osx/relocate-binary.sh" "${MACPORTS}${PKG_PREFIX}/lib/gdk-pixbuf-2.0/2.10.0/loaders/$FILE" "$MACPORTS" "$APPCONTENTS"
	done
	popd
	cp -R "${MACPORTS}${PKG_PREFIX}/share/gir-1.0"  "${APPCONTENTS}/share/"
	cp -R "${MACPORTS}${PKG_PREFIX}/share/locale"  "${APPCONTENTS}/share/"
	
	if [ -d "$MACPORTS/librsvg-2.40.20" ]; then
		PKG_PREFIX="/librsvg-2.40.20"
	elif [ -d "$MACPORTS/librsvg" ]; then
		PKG_PREFIX="/librsvg"
	else
		PKG_PREFIX=""
	fi
	pushd "${MACPORTS}${PKG_PREFIX}/lib/gdk-pixbuf-2.0/2.10.0/loaders/"
	for FILE in `ls -1 *.so`; do
		"$SCRIPTPATH/autobuild/osx/relocate-binary.sh" "${MACPORTS}${PKG_PREFIX}/lib/gdk-pixbuf-2.0/2.10.0/loaders/$FILE" "$MACPORTS" "$APPCONTENTS"
	done
	popd
	
	if [ -d "$MACPORTS/gtk+3" ]; then
		PKG_PREFIX="/gtk+3"
	else
		PKG_PREFIX=""
	fi
	pushd "${MACPORTS}${PKG_PREFIX}/lib/gtk-3.0/3.0.0/immodules/"
	for FILE in `ls -1 *.so`; do
		"$SCRIPTPATH/autobuild/osx/relocate-binary.sh" "${MACPORTS}${PKG_PREFIX}/lib/gtk-3.0/3.0.0/immodules/$FILE" "$MACPORTS" "$APPCONTENTS"
	done
	popd
	pushd "${MACPORTS}${PKG_PREFIX}/lib/gtk-3.0/3.0.0/printbackends/"
	for FILE in `ls -1 *.so`; do
		"$SCRIPTPATH/autobuild/osx/relocate-binary.sh" "${MACPORTS}${PKG_PREFIX}/lib/gtk-3.0/3.0.0/printbackends/$FILE" "$MACPORTS" "$APPCONTENTS"
	done
	popd
	cp -R "${MACPORTS}${PKG_PREFIX}/etc/gtk-3.0"  "${APPCONTENTS}/etc/"
	cp -R "${MACPORTS}${PKG_PREFIX}/lib/girepository-1.0"  "${APPCONTENTS}/lib/"
	cp -R "${MACPORTS}${PKG_PREFIX}/share/icons"  "${APPCONTENTS}/share/"
	cp -R "${MACPORTS}${PKG_PREFIX}/share/gir-1.0"  "${APPCONTENTS}/share/"
	cp -R "${MACPORTS}${PKG_PREFIX}/share/glib-2.0"  "${APPCONTENTS}/share/"
	cp -R "${MACPORTS}${PKG_PREFIX}/share/locale"  "${APPCONTENTS}/share/"
	cp -R "${MACPORTS}${PKG_PREFIX}/share/themes"  "${APPCONTENTS}/share/"
	
	if [ -d "$MACPORTS/gsettings-desktop-schemas" ]; then
		PKG_PREFIX="/gsettings-desktop-schemas"
	else
		PKG_PREFIX=""
	fi
	cp -R "${MACPORTS}${PKG_PREFIX}/share/glib-2.0"  "${APPCONTENTS}/share/"
	glib-compile-schemas "${APPCONTENTS}/share/glib-2.0/schemas/"
	
	if [ -d "$MACPORTS/hicolor-icon-theme" ]; then
		PKG_PREFIX="/hicolor-icon-theme"
	else
		PKG_PREFIX=""
	fi
	cp -R "${MACPORTS}${PKG_PREFIX}/share/icons"  "${APPCONTENTS}/share/"
	
	if [ -d "$MACPORTS/adwaita-icon-theme" ]; then
		PKG_PREFIX="/adwaita-icon-theme"
	else
		PKG_PREFIX=""
	fi
	cp -R "${MACPORTS}${PKG_PREFIX}/share/icons/Adwaita"  "${APPCONTENTS}/share/icons/"
	
	if [ -d "$MACPORTS/cairo" ]; then
		PKG_PREFIX="/cairo"
	else
		PKG_PREFIX=""
	fi
	pushd "${MACPORTS}${PKG_PREFIX}/lib/cairo/"
	for FILE in `ls -1 *.so`; do
		"$SCRIPTPATH/autobuild/osx/relocate-binary.sh" "${MACPORTS}${PKG_PREFIX}/lib/cairo/$FILE" "$MACPORTS" "$APPCONTENTS"
	done

	mkdir -p "${APPCONTENTS}/lib/cairo/" || true
	cp -R ${MACPORTS}${PKG_PREFIX}/lib/cairo/*.la  "${APPCONTENTS}/lib/cairo/" || true
	cp -R ${MACPORTS}${PKG_PREFIX}/lib/cairo/*.a  "${APPCONTENTS}/lib/cairo/" || true
	popd
	#pushd "$MACPORTS/lib/engines/"
	#for FILE in `ls -1 *.so`; do
	#	"$SCRIPTPATH/autobuild/osx/relocate-binary.sh" "`smart_find $MACPORTS/lib/engines/$FILE`" "$MACPORTS" "$APPCONTENTS"
	#done
	#popd
	if [ -d "$MACPORTS/glib" ]; then
		PKG_PREFIX="/glib"
	else
		PKG_PREFIX=""
	fi
	#mkdir -p "${APPCONTENTS}/share/glib-2.0/"
	cp -R "${MACPORTS}${PKG_PREFIX}/share/glib-2.0"  "${APPCONTENTS}/share/"
	cp -R "${MACPORTS}${PKG_PREFIX}/share/locale"  "${APPCONTENTS}/share/"

	# Python 3
	if [ -d "$MACPORTS/python3" ]; then
		PKG_PREFIX="/python3"
	else
		PKG_PREFIX="/Library"
	fi
	pushd "${MACPORTS}${PKG_PREFIX}/Frameworks/Python.framework/Versions/"
	PYTHON_VERSION=`ls -1 | head -n 1`
	popd
	"$SCRIPTPATH/autobuild/osx/relocate-binary.sh" "`smart_find $MACPORTS/python3/bin/python3`" "$MACPORTS" "$APPCONTENTS"
	"$SCRIPTPATH/autobuild/osx/relocate-binary.sh" "${MACPORTS}${PKG_PREFIX}/Frameworks/Python.framework/Versions/${PYTHON_VERSION}/Resources/Python.app/Contents/MacOS/Python" "$MACPORTS" "$APPCONTENTS"
	mkdir -p "${APPCONTENTS}/Frameworks/Python.framework/Versions/${PYTHON_VERSION}/lib/python${PYTHON_VERSION}/"
	rsync -av --exclude "__pycache__" "${MACPORTS}${PKG_PREFIX}/Frameworks/Python.framework/Versions/${PYTHON_VERSION}/lib/python${PYTHON_VERSION}/" "${APPCONTENTS}/Frameworks/Python.framework/Versions/${PYTHON_VERSION}/lib/python${PYTHON_VERSION}/"
	#cp -R "${MACPORTS}${PKG_PREFIX}/Frameworks/Python.framework/Versions/${PYTHON_VERSION}/lib/python3.3" "${APPCONTENTS}/Frameworks/Python.framework/Versions/${PYTHON_VERSION}/lib/"
	#find ${APPCONTENTS}/Frameworks/Python.framework/Versions/${PYTHON_VERSION}/lib -name "__pycache__" -exec rm -rf {} \;

	# MLT
	if [ -d "$MACPORTS/mlt" ]; then
		PKG_PREFIX="/mlt"
	else
		PKG_PREFIX=""
	fi
	"$SCRIPTPATH/autobuild/osx/relocate-binary.sh" "${MACPORTS}${PKG_PREFIX}/bin/melt" "$MACPORTS" "$APPCONTENTS"
	pushd "${MACPORTS}${PKG_PREFIX}/lib/mlt/"
	for FILE in `ls -1 *.dylib`; do
		"$SCRIPTPATH/autobuild/osx/relocate-binary.sh" "${MACPORTS}${PKG_PREFIX}/lib/mlt/$FILE" "$MACPORTS" "$APPCONTENTS"
	done
	popd
	mkdir -p "${APPCONTENTS}/share/"
	cp -R "${MACPORTS}${PKG_PREFIX}/share/mlt" "${APPCONTENTS}/share/"

	# ImageMagick
	"$SCRIPTPATH/autobuild/osx/relocate-binary.sh" "`smart_find $MACPORTS/imagemagick/bin/animate`" "$MACPORTS" "$APPCONTENTS"
	"$SCRIPTPATH/autobuild/osx/relocate-binary.sh" "`smart_find $MACPORTS/imagemagick/bin/composite`" "$MACPORTS" "$APPCONTENTS"
	"$SCRIPTPATH/autobuild/osx/relocate-binary.sh" "`smart_find $MACPORTS/imagemagick/bin/convert`" "$MACPORTS" "$APPCONTENTS"
	if [ -d "$MACPORTS/imagemagick" ]; then
		PKG_PREFIX="/imagemagick"
	else
		PKG_PREFIX=""
	fi
	pushd "${MACPORTS}${PKG_PREFIX}/lib/"
	IMAGEMAGICK_DIR=`ls -1d ImageMagick* |head -n 1`
	popd
	pushd "${MACPORTS}${PKG_PREFIX}/lib/${IMAGEMAGICK_DIR}"
	IMAGEMAGICK_MODULES_DIR=`ls -1d modules-* |head -n 1`
	popd
	pushd "${MACPORTS}${PKG_PREFIX}/lib/${IMAGEMAGICK_DIR}"
	IMAGEMAGICK_CONFIG_DIR=`ls -1d config-* |head -n 1`
	popd
	pushd "${MACPORTS}${PKG_PREFIX}/etc"
	IMAGEMAGICK_ETC_DIR=`ls -1d ImageMagick-* |head -n 1`
	popd
	pushd "${MACPORTS}${PKG_PREFIX}/lib/${IMAGEMAGICK_DIR}/${IMAGEMAGICK_MODULES_DIR}/coders/"
	for FILE in `ls -1 *.so`; do
		"$SCRIPTPATH/autobuild/osx/relocate-binary.sh" "${MACPORTS}${PKG_PREFIX}/lib/${IMAGEMAGICK_DIR}/${IMAGEMAGICK_MODULES_DIR}/coders/$FILE" "$MACPORTS" "$APPCONTENTS"
	done
	cp -R ${MACPORTS}${PKG_PREFIX}/lib/${IMAGEMAGICK_DIR}/${IMAGEMAGICK_MODULES_DIR}/coders/*.la "${APPCONTENTS}/lib/${IMAGEMAGICK_DIR}/${IMAGEMAGICK_MODULES_DIR}/coders/"
	popd
	pushd "${MACPORTS}${PKG_PREFIX}/lib/${IMAGEMAGICK_DIR}/${IMAGEMAGICK_MODULES_DIR}/filters/"
	for FILE in `ls -1 *.so`; do
		"$SCRIPTPATH/autobuild/osx/relocate-binary.sh" "${MACPORTS}${PKG_PREFIX}/lib/${IMAGEMAGICK_DIR}/${IMAGEMAGICK_MODULES_DIR}/filters/$FILE" "$MACPORTS" "$APPCONTENTS"
	done
	cp -R ${MACPORTS}${PKG_PREFIX}/lib/${IMAGEMAGICK_DIR}/${IMAGEMAGICK_MODULES_DIR}/filters/*.la  "${APPCONTENTS}/lib/${IMAGEMAGICK_DIR}/${IMAGEMAGICK_MODULES_DIR}/filters/"
	popd
	cp -R "${MACPORTS}${PKG_PREFIX}/lib/${IMAGEMAGICK_DIR}/${IMAGEMAGICK_CONFIG_DIR}"  "${APPCONTENTS}/lib/${IMAGEMAGICK_DIR}/"
	cp -R "${MACPORTS}${PKG_PREFIX}/etc/${IMAGEMAGICK_ETC_DIR}"  "${APPCONTENTS}/etc/"

	#cp -R "${MACPORTS}/share/icons"  "$APPCONTENTS/share/"
	#cp -R "${MACPORTS}/share/themes"  "$APPCONTENTS/share/"
	#cp -R "${MACPORTS}/share/mime"  "$APPCONTENTS/share/"

	# app bundle files
	echo "*** Please do _NOT_ delete this file. The file script depends on it. ***" > "$APPCONTENTS/v$VERSION"
	sed -i "" -e "s/_VERSION_/$VERSION/g" "$APPDIR/Contents/MacOS/SynfigStudio"
	sed -i "" -e "s/_VERSION_/$VERSION/g" "$APPDIR/Contents/Info.plist"

	# save information about the ports which make up this build
	echo "Synfig Studio $VERSION for Mac OS X" > "$APPCONTENTS/build-info.txt"
	date >> "$APPCONTENTS/build-info.txt"
	#port installed >> "$APPCONTENTS/build-info.txt"
	#sed -i "" -e "s/are currently installed:/were used to build this package:/g" "$APPCONTENTS/build-info.txt"

	#mv "${APPDIR}" "${APPDIR_FINAL}"

	echo
	echo "Your new Synfig Studio app bundle should now be ready to run."
	echo
}

mkdmg()
{
	cd "${SCRIPTPATH}/_production/"

	# get OS major version
	OSXVER=`uname -r | cut -f 1 -d '.'`

	#VERSION=`synfig --version 2>&1 | cut -d " " -f 2`
	VERSION=`get_version_release_string`
	#echo Synfig version is: $VERSION

	ARCH=`uname -m`
	export FINAL_FILENAME=SynfigStudio-"$VERSION"


	VOLNAME="SynfigStudio"
	TRANSITORY_FILENAME="synfig-wla.sparseimage"
	[ ! -f "$TRANSITORY_FILENAME" ] || rm -f "$TRANSITORY_FILENAME"

	APPDIR="${SCRIPTPATH}/_production/SynfigStudio.app"

	/usr/bin/hdiutil detach /Volumes/"$VOLNAME" || true

	echo "Creating and attaching disk image..."
	[ ! -e "$TRANSITORY_FILENAME" ] || rm -rf "$TRANSITORY_FILENAME"
	/usr/bin/hdiutil create -type SPARSE -size 700m -fs "HFS+" -volname "$VOLNAME" -attach "$TRANSITORY_FILENAME"

	echo "Copying files to disk image..."
	cp -R $APPDIR /Volumes/"$VOLNAME"/SynfigStudio.app
	cp -R ${SCRIPTPATH}/synfig-studio/COPYING /Volumes/"$VOLNAME"/LICENSE.txt
	cp -R ${SCRIPTPATH}/autobuild/osx/README_FIRST.pdf /Volumes/"$VOLNAME"/README_FIRST.pdf
	mv /Volumes/"$VOLNAME"/SynfigStudio.app/Contents/MacOS/synfigstudio /Volumes/"$VOLNAME"/SynfigStudio.app/Contents/MacOS/SynfigStudio || true

	# open the window so that the icon database is generated
	open /Volumes/"$VOLNAME" || true
	sleep 3

	echo "Detaching disk image..."
	/usr/bin/hdiutil detach /Volumes/"$VOLNAME"

	echo "Compressing disk image..."
	[ ! -e "./${FINAL_FILENAME}.dmg" ] || rm -rf "./${FINAL_FILENAME}.dmg"
	/usr/bin/hdiutil convert -imagekey zlib-level=9 -format UDBZ "$TRANSITORY_FILENAME" -o ./"${FINAL_FILENAME}"

	echo "Removing uncompressed transitory dmg..."
	/bin/rm -f "$TRANSITORY_FILENAME"

	echo "Done!"
}

get_version_release_string()
{
	pushd "$SCRIPTPATH" > /dev/null
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



###=================================== MAIN ======================================

main()  # dummy for navigation
{
	true
}

mkapp
mkdmg
