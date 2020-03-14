#!/bin/sh

set -e

DIR=`dirname "$0"`
cd "$DIR/../Resources"
CWD=`pwd`

#export DYLD_LIBRARY_PATH="$CWD/lib:$CWD/synfig/lib:$CWD/Library/Frameworks/Python.framework/Versions/3.3/:/Volumes/data/SynfigStudio.app/Contents/Resources/Library/Frameworks/Python.framework/Versions/3.3/Resources/Python.app/Contents/MacOS/:$DYLD_LIBRARY_PATH"

export GTK_EXE_PREFIX="$CWD"
export GTK_DATA_PREFIX="$CWD/share"

export GSETTINGS_SCHEMA_DIR="$CWD/share/glib-2.0/schemas/"
export FONTCONFIG_PATH="$CWD/etc/fonts"
export MLT_DATA="$CWD/share/mlt/"
export MLT_REPOSITORY="$CWD/lib/mlt/"

export PATH="$CWD/bin:$CWD/synfig-production/bin:$PATH"
export SYNFIG_ROOT="$CWD/"
export SYNFIG_MODULE_LIST="$CWD/etc/synfig_modules.cfg"

export PYTHON_VERSION=`ls -1 $CWD/Frameworks/Python.framework/Versions/ | head -n 1`
export PYTHONHOME="$CWD/Frameworks/Python.framework/Versions/${PYTHON_VERSION}/"

export MAGICK_DIR=`ls -1 -d ${CWD}/lib/ImageMagick* | head -n 1`
export MAGICK_DIR=`basename $MAGICK_DIR`
export MAGICK_DIR_CONFIG=`ls -1 -d ${CWD}/lib/${MAGICK_DIR}/config-* | head -n 1`
export MAGICK_DIR_CONFIG=`basename $MAGICK_DIR_CONFIG`
export MAGICK_DIR_MODULES=`ls -1 -d ${CWD}/lib/${MAGICK_DIR}/modules-* | head -n 1`
export MAGICK_DIR_MODULES=`basename $MAGICK_DIR_MODULES`
export MAGICK_CONFIGURE_PATH="${CWD}/lib/${MAGICK_DIR}/${MAGICK_DIR_CONFIG}/"
export MAGICK_CODER_MODULE_PATH="${CWD}/lib/${MAGICK_DIR}/${MAGICK_DIR_MODULES}/coders/"
export MAGICK_CODER_FILTER_PATH="${CWD}/lib/${MAGICK_DIR}/${MAGICK_DIR_MODULES}/filters/"

export XDG_DATA_DIRS="$CWD/share/:$XDG_DATA_DIRS"
export GDK_PIXBUF_MODULEDIR="$CWD/lib/gdk-pixbuf-2.0/2.10.0/loaders/"
export GDK_PIXBUF_MODULE_FILE=$HOME/.synfig-gdk-loaders
[ ! -f $HOME/.synfig-gdk-loaders ] || rm -f  $HOME/.synfig-gdk-loaders
"$CWD/bin/gdk-pixbuf-query-loaders" > "$GDK_PIXBUF_MODULE_FILE"

cd "$CWD"
exec "$SYNFIG_ROOT/bin/synfigstudio" "$@"
