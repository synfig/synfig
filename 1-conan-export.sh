#!/bin/sh

SCRIPT=$(readlink -f "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

conan export $SCRIPTPATH/conan-recipes/gtk/all gtk/3.24.34@
conan export $SCRIPTPATH/conan-recipes/gtkmm/all gtkmm/3.24.6@
conan export $SCRIPTPATH/conan-recipes/autoconf/all autoconf/2.71@
conan export $SCRIPTPATH/conan-recipes/openexr/3.x openexr/3.1.5@
conan export $SCRIPTPATH/conan-recipes/atk/all atk/2.38.0@
conan export $SCRIPTPATH/conan-recipes/glib/all glib/2.73.2@
conan export $SCRIPTPATH/conan-recipes/gdk-pixbuf/all gdk-pixbuf/2.42.8@
conan export $SCRIPTPATH/conan-recipes/cairo/meson cairo/1.17.4@
conan export $SCRIPTPATH/conan-recipes/pango/all pango/1.50.8@
conan export $SCRIPTPATH/conan-recipes/harfbuzz/all harfbuzz/5.1.0@
conan export $SCRIPTPATH/conan-recipes/glibmm/all glibmm/2.66.4@
conan export $SCRIPTPATH/conan-recipes/cairomm/all cairomm/1.14.3@
conan export $SCRIPTPATH/conan-recipes/pangomm/all pangomm/2.46.2@
conan export $SCRIPTPATH/conan-recipes/libsigcpp/2.x.x libsigcpp/2.10.8@
conan export $SCRIPTPATH/conan-recipes/imagemagick/all imagemagick/7.1.0-45@

