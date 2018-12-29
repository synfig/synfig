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

SYNFIG_REPO_DIR="$SCRIPTPATH"




#======= HEADER END ===========


readlink_f() {
	TARGET_FILE=$1

	cd `dirname $TARGET_FILE`
	TARGET_FILE=`basename $TARGET_FILE`

	# Iterate down a (possible) chain of symlinks
	while [ -L "$TARGET_FILE" ]
	do
		TARGET_FILE=`readlink $TARGET_FILE`
		cd `dirname $TARGET_FILE`
		TARGET_FILE=`basename $TARGET_FILE`
	done

	# Compute the canonicalized name by finding the physical path 
	# for the directory we're in and appending the target file.
	PHYS_DIR=`pwd -P`
	RESULT=$PHYS_DIR/$TARGET_FILE
	echo $RESULT
}

mkapp()
{
	cd ${SCRIPTPATH}
	VERSION=`get_version_release_string`
	echo Now trying to build your new SynfigStudio app ...

	APPDIR="${SCRIPTPATH}/_production/SynfigStudio-app"
	APPDIR_FINAL="${SCRIPTPATH}/_production/SynfigStudio.app"
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

if false; then
	# Synfig
	"${SCRIPTPATH}/autobuild/osx-relocate-binary.sh" "${SCRIPTPATH}/_production/build/bin/synfig" "$APPCONTENTS"
	"${SCRIPTPATH}/autobuild/osx-relocate-binary.sh" "${SCRIPTPATH}/_production/build/bin/synfigstudio" "$APPCONTENTS"
	pushd "${SCRIPTPATH}/_production/build/lib/synfig/modules/"
	for FILE in `ls -1 *.so`; do
		"${SCRIPTPATH}/autobuild/osx-relocate-binary.sh" "${SCRIPTPATH}/_production/build/lib/synfig/modules/$FILE" "$APPCONTENTS" "${SCRIPTPATH}/_production/build/"
	done
	cp -R ${SCRIPTPATH}/_production/build/lib/synfig/modules/*.la  "$APPCONTENTS/lib/synfig/modules/"
	popd
	cp -R "${SCRIPTPATH}/_production/build/etc" "$APPCONTENTS/"
	cp -R "${SCRIPTPATH}/_production/build/share" "$APPCONTENTS/"

	"${SCRIPTPATH}/autobuild/osx-relocate-binary.sh" "`which ffmpeg`" "$APPCONTENTS"
	"${SCRIPTPATH}/autobuild/osx-relocate-binary.sh" "`which ffprobe`" "$APPCONTENTS"

	"${SCRIPTPATH}/autobuild/osx-relocate-binary.sh" "`which encodedv`" "$APPCONTENTS"
	"${SCRIPTPATH}/autobuild/osx-relocate-binary.sh" "`which sox`" "$APPCONTENTS"

	# Gtk3
	"${SCRIPTPATH}/autobuild/osx-relocate-binary.sh" "`which gdk-pixbuf-query-loaders`" "$APPCONTENTS"
	"${SCRIPTPATH}/autobuild/osx-relocate-binary.sh" "`which gdk-pixbuf-pixdata`" "$APPCONTENTS"
	
	PKG_BINARY=`which gdk-pixbuf-query-loaders`
	PKG_PATH=`readlink_f ${PKG_BINARY}`
	PKG_PATH=`dirname ${PKG_PATH}`
	PKG_PATH=`dirname ${PKG_PATH}`
	#!!!
	pushd "${PKG_PATH}/lib/gdk-pixbuf-2.0/2.10.0/loaders/"
	for FILE in `ls -1 *.so`; do
		"${SCRIPTPATH}/autobuild/osx-relocate-binary.sh" "$PKG_PATH/lib/gdk-pixbuf-2.0/2.10.0/loaders/$FILE" "$APPCONTENTS" "${PKG_PATH}"
	done
	popd
	
	"${SCRIPTPATH}/autobuild/osx-relocate-binary.sh" "`which gtk3-demo`" "$APPCONTENTS"
	
	PKG_BINARY=`which gtk3-demo`
	PKG_PATH=`readlink_f ${PKG_BINARY}`
	PKG_PATH=`dirname ${PKG_PATH}`
	PKG_PATH=`dirname ${PKG_PATH}`
	pushd "${PKG_PATH}/lib/gtk-3.0/3.0.0/immodules/"
	for FILE in `ls -1 *.so`; do
		"${SCRIPTPATH}/autobuild/osx-relocate-binary.sh" "$PKG_PATH/lib/gtk-3.0/3.0.0/immodules/$FILE" "$APPCONTENTS" "${PKG_PATH}"
	done
	popd
	pushd "${PKG_PATH}/lib/gtk-3.0/3.0.0/printbackends/"
	for FILE in `ls -1 *.so`; do
		"${SCRIPTPATH}/autobuild/osx-relocate-binary.sh" "$PKG_PATH/lib/gtk-3.0/3.0.0/printbackends/$FILE" "$APPCONTENTS" "${PKG_PATH}"
	done
	popd
	cp -R "$PKG_PATH/etc/gtk-3.0"  "$APPCONTENTS/etc/"
	cp -R "$PKG_PATH/lib/girepository-1.0"  "$APPCONTENTS/lib/"
	
	PKG_BINARY=`which cairo-trace`
	PKG_PATH=`readlink_f ${PKG_BINARY}`
	PKG_PATH=`dirname ${PKG_PATH}`
	PKG_PATH=`dirname ${PKG_PATH}`
	pushd "${PKG_PATH}/lib/cairo/"
	for FILE in `ls -1 *.so`; do
		"${SCRIPTPATH}/autobuild/osx-relocate-binary.sh" "$PKG_PATH/lib/cairo/$FILE" "$APPCONTENTS" "${PKG_PATH}"
	done
	popd
	cp -R "${PKG_PATH}/lib/cairo/*.la"  "$APPCONTENTS/lib/cairo/" || true
	
	#pushd "$MACPORTS/lib/engines/"
	#for FILE in `ls -1 *.so`; do
	#	"${SCRIPTPATH}/autobuild/osx-relocate-binary.sh" "$MACPORTS/lib/engines/$FILE" "$MACPORTS" "$APPCONTENTS"
	#done
	#popd
	
	PKG_BINARY=`which gio`
	PKG_PATH=`readlink_f ${PKG_BINARY}`
	PKG_PATH=`dirname ${PKG_PATH}`
	PKG_PATH=`dirname ${PKG_PATH}`
	mkdir -p "$APPCONTENTS/share/glib-2.0/"
	cp -R "$PKG_PATH/share/glib-2.0/schemas"  "$APPCONTENTS/share/glib-2.0"

	# Python 3
	PKG_BINARY=`which python3`
	PKG_PATH=`readlink_f ${PKG_BINARY}`
	PKG_PATH=`dirname ${PKG_PATH}`
	PKG_PATH=`dirname ${PKG_PATH}`
	PKG_PATH=`dirname ${PKG_PATH}`
	PKG_PATH=`dirname ${PKG_PATH}`
	PKG_PATH=`dirname ${PKG_PATH}`
	PKG_PATH=`dirname ${PKG_PATH}`
	#"${SCRIPTPATH}/autobuild/osx-relocate-binary.sh" "${PKG_PATH}/bin/python3" "$APPCONTENTS"
	pushd "${PKG_PATH}/Frameworks/Python.framework/Versions/"
	PYTHON_VERSION=`ls -1 | head -n 1`
	popd
	"${SCRIPTPATH}/autobuild/osx-relocate-binary.sh" "${PKG_PATH}/Frameworks/Python.framework/Versions/${PYTHON_VERSION}/Resources/Python.app/Contents/MacOS/Python" "$APPCONTENTS" "${PKG_PATH}"
	mkdir -p "$APPCONTENTS/Frameworks/Python.framework/Versions/${PYTHON_VERSION}/lib/python${PYTHON_VERSION}/"
	rsync -av --exclude "__pycache__" "${PKG_PATH}/Frameworks/Python.framework/Versions/${PYTHON_VERSION}/lib/python${PYTHON_VERSION}/" "$APPCONTENTS/Frameworks/Python.framework/Versions/${PYTHON_VERSION}/lib/python${PYTHON_VERSION}/"
	#cp -R "$MACPORTS/Library/Frameworks/Python.framework/Versions/3.3/lib/python3.3" "$APPCONTENTS/Library/Frameworks/Python.framework/Versions/3.3/lib/"
	#find $APPCONTENTS/Library/Frameworks/Python.framework/Versions/3.3/lib -name "__pycache__" -exec rm -rf {} \;

	# MLT
	PKG_BINARY=`which melt`
	PKG_PATH=`readlink_f ${PKG_BINARY}`
	PKG_PATH=`dirname ${PKG_PATH}`
	PKG_PATH=`dirname ${PKG_PATH}`
	"${SCRIPTPATH}/autobuild/osx-relocate-binary.sh" "$PKG_PATH/bin/melt" "$APPCONTENTS"
	pushd "${PKG_PATH}/lib/mlt/"
	for FILE in `ls -1 *.dylib`; do
		"${SCRIPTPATH}/autobuild/osx-relocate-binary.sh" "${PKG_PATH}/lib/mlt/$FILE" "$APPCONTENTS" "${PKG_PATH}"
	done
	popd
	cp -R "$PKG_PATH/share/mlt"  "$APPCONTENTS/share/"
fi
	# ImageMagick
	PKG_BINARY=`which convert`
	PKG_PATH=`readlink_f ${PKG_BINARY}`
	PKG_PATH=`dirname ${PKG_PATH}`
	PKG_PATH=`dirname ${PKG_PATH}`
	"${SCRIPTPATH}/autobuild/osx-relocate-binary.sh" "$PKG_PATH/bin/animate" "$APPCONTENTS"
	"${SCRIPTPATH}/autobuild/osx-relocate-binary.sh" "$PKG_PATH/bin/composite" "$APPCONTENTS"
	"${SCRIPTPATH}/autobuild/osx-relocate-binary.sh" "$PKG_PATH/bin/convert" "$APPCONTENTS"
	pushd "$PKG_PATH/lib/ImageMagick-6.9.2/modules-Q16/coders/"
	for FILE in `ls -1 *.so`; do
		"${SCRIPTPATH}/autobuild/osx-relocate-binary.sh" "$PKG_PATH/lib/ImageMagick-6.9.2/modules-Q16/coders/$FILE" "$APPCONTENTS" "$PKG_PATH"
	done
	cp -R $PKG_PATH/lib/ImageMagick-6.9.2/modules-Q16/coders/*.la  "$APPCONTENTS/lib/ImageMagick-6.9.2/modules-Q16/coders/"
	popd
	pushd "$PKG_PATH/lib/ImageMagick-6.9.2/modules-Q16/filters/"
	for FILE in `ls -1 *.so`; do
		"${SCRIPTPATH}/autobuild/osx-relocate-binary.sh" "$PKG_PATH/lib/ImageMagick-6.9.2/modules-Q16/filters/$FILE" "$APPCONTENTS" "$PKG_PATH"
	done
	cp -R $PKG_PATH/lib/ImageMagick-6.9.2/modules-Q16/filters/*.la  "$APPCONTENTS/lib/ImageMagick-6.9.2/modules-Q16/filters/"
	popd
	cp -R "$PKG_PATH/lib/ImageMagick-6.9.2/config-Q16"  "$APPCONTENTS/lib/ImageMagick-6.9.2/"
	cp -R "$PKG_PATH/etc/ImageMagick-6"  "$APPCONTENTS/etc/"

	cp -R "$PKG_PATH/share/icons"  "$APPCONTENTS/share/"
	cp -R "$PKG_PATH/share/themes"  "$APPCONTENTS/share/"
	cp -R "$PKG_PATH/share/mime"  "$APPCONTENTS/share/"

	# app bundle files
	echo "*** Please do _NOT_ delete this file. The file script depends on it. ***" > "$APPCONTENTS/v$VERSION"
	sed -i "" -e "s/_VERSION_/$VERSION/g" "$APPDIR/Contents/MacOS/SynfigStudio"
	sed -i "" -e "s/_VERSION_/$VERSION/g" "$APPDIR/Contents/Info.plist"

	# save information about the ports which make up this build
	echo "Synfig Studio $VERSION for Mac OS X" > "$APPCONTENTS/build-info.txt"
	date >> "$APPCONTENTS/build-info.txt"
	#port installed >> "$APPCONTENTS/build-info.txt"
	#sed -i "" -e "s/are currently installed:/were used to build this package:/g" "$APPCONTENTS/build-info.txt"

	mv "$APPDIR" "$APPDIR_FINAL"

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
	export FINAL_FILENAME=SynfigStudio-"$VERSION"


	VOLNAME="SynfigStudio"
	TRANSITORY_FILENAME="synfig-wla.sparseimage"

	APPDIR="${SCRIPTPATH}/_production/SynfigStudio.app"

	/usr/bin/hdiutil detach /Volumes/"$VOLNAME" || true

	echo "Creating and attaching disk image..."
	[ ! -e "$TRANSITORY_FILENAME" ] || rm -rf "$TRANSITORY_FILENAME"
	/usr/bin/hdiutil create -type SPARSE -size 700m -fs "HFS+" -volname "$VOLNAME" -attach "$TRANSITORY_FILENAME"

	echo "Copying files to disk image..."
	cp -R $APPDIR /Volumes/"$VOLNAME"/SynfigStudio.app
	cp -R ${SCRIPTPATH}/synfig-studio/COPYING /Volumes/"$VOLNAME"/LICENSE.txt
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



###=================================== MAIN ======================================

main()  # dummy for navigation
{
	true
}

mkapp
mkdmg
