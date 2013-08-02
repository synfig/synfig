#!/bin/sh

#TODO: Replace version numbers in the .nsi file
#TODO: Uninstall properly!
#TODO: 64bit build
#TODO: Magick++

set -e

export SCRIPTPATH=`dirname "$0"`

export BUILDROOT=$HOME/synfig-buildroot
export PREFIX=$BUILDROOT/win32
export DISTPREFIX=$BUILDROOT/tmp/win32
export CACHEDIR=$BUILDROOT/cache
export PKG_CONFIG_PATH=${PREFIX}/lib/pkgconfig:$PKG_CONFIG_PATH
export PKG_CONFIG_LIBDIR=${PREFIX}/lib/pkgconfig:$PKG_CONFIG_LIBDIR
export PATH=${PREFIX}/bin:$PATH
export LD_LIBRARY_PATH=${PREFIX}/lib:$LD_LIBRARY_PATH
export LDFLAGS="-Wl,-rpath -Wl,\\\$\$ORIGIN/lib"

mkprep()
{

if [ -z $NOSU ]; then
	su -c "yum install -y \
		intltool \
		gcc-c++ \
		autoconf \
		automake \
		libtool \
		libtool-ltdl-devel \
		mingw32-gcc-c++ \
		mingw32-libxml++ \
		mingw32-cairo \
		mingw32-pango \
		mingw32-boost \
		mingw32-libjpeg-turbo \
		mingw32-gtkmm24 \
		mingw32-nsis \
		p7zip \
		ImageMagick \
		"
fi

# copy libs
[ -d ${PREFIX}/bin ] || mkdir -p ${PREFIX}/bin
for file in \
   iconv.dll \
   libatk-\*.dll \
   libatkmm-1.6-1.dll \
   libboost_program_options\*.dll \
   libbz2\*.dll \
   libcairo\*.dll \
   libexpat\*.dll \
   libffi\*.dll \
   libfontconfig\*.dll \
   libfreetype\*.dll \
   libgcc_s_\*.dll \
   libgdk\*.dll \
   libgettext\*.dll \
   libgio\*.dll \
   libglib\*.dll \
   libgmodule\*.dll \
   libgobject\*.dll \
   libgthread\*.dll \
   libgtk\*.dll \
   libharfbuzz\*.dll \
   libiconv\*.dll \
   libintl\*.dll \
   libjasper\*.dll \
   libjpeg\*.dll \
   liblzma\*.dll \
   libpango\*.dll \
   libpixman\*.dll \
   libpng\*.dll \
   libsigc\*.dll \
   libstdc++\*.dll \
   libsynfig\*.dll \
   libtiff\*.dll \
   libturbojpeg.dll \
   libxml2\*.dll \
   libxml++\*.dll \
   libz\*.dll \
   pthread\*.dll \
   zlib\*.dll \
   pango-querymodules.exe \
   synfig.exe \
   synfigstudio.exe \
# this extra line is required!
do
	cp /usr/i686-w64-mingw32/sys-root/mingw/bin/$file ${PREFIX}/bin || true
done

[ -d ${PREFIX}/etc ] || mkdir -p ${PREFIX}/etc
for file in \
   fonts \
   gtk-2.0 \
   pango \
# this extra line is required!
do
	cp -rf /usr/i686-w64-mingw32/sys-root/mingw/etc/$file ${PREFIX}/etc
done

[ -d ${PREFIX}/lib ] || mkdir -p ${PREFIX}/lib
for file in \
   gtk-2.0 \
   gdk-pixbuf-2.0 \
   pango \
# this extra line is required!
do
	cp -rf /usr/i686-w64-mingw32/sys-root/mingw/lib/$file ${PREFIX}/lib
done

[ -d ${PREFIX}/share ] || mkdir -p ${PREFIX}/share
for file in \
   fontconfig \
   themes \
   xml \
# this extra line is required!
do
	cp -rf /usr/i686-w64-mingw32/sys-root/mingw/lib/$file ${PREFIX}/lib
done
}

#ETL
mketl()
{
cd $SCRIPTPATH/../ETL
make clean || true
autoreconf --install --force
mingw32-configure --prefix=${PREFIX} --includedir=${PREFIX}/include --libdir=${PREFIX}/lib --bindir=${PREFIX}/bin $DEBUG
make install
}

#TODO: Magick++


#synfig-core
mksynfig()
{
cd $SCRIPTPATH/../synfig-core/
make clean || true
libtoolize --ltdl --copy --force
autoreconf --install --force
cp ./configure ./configure.real
echo -e "#/bin/sh \n export PKG_CONFIG_PATH=${PREFIX}/lib/pkgconfig:$PKG_CONFIG_PATH \n ./configure.real \$@  \n " > ./configure
chmod +x ./configure
mingw32-configure --prefix=${PREFIX} --includedir=${PREFIX}/include --disable-static --enable-shared --with-magickpp --without-libavcodec --libdir=${PREFIX}/lib --bindir=${PREFIX}/bin --sysconfdir=${PREFIX}/etc --with-boost=/usr/i686-w64-mingw32/sys-root/mingw/ --enable-warnings=minimum $DEBUG
make install -j4
}

#synfig-studio
mksynfigstudio()
{
cd $SCRIPTPATH/../synfig-studio/
make clean || true
/bin/sh ./bootstrap.sh
cp ./configure ./configure.real
echo -e "#/bin/sh \n export PKG_CONFIG_PATH=${PREFIX}/lib/pkgconfig:$PKG_CONFIG_PATH \n ./configure.real \$@  \n " > ./configure
chmod +x ./configure
mingw32-configure --prefix=${PREFIX} --includedir=${PREFIX}/include --disable-static --enable-shared --libdir=${PREFIX}/lib --bindir=${PREFIX}/bin --sysconfdir=${PREFIX}/etc --datadir=${PREFIX}/share  $DEBUG
make install -j4
cp -rf ${PREFIX}/share/pixmaps/synfigstudio/* ${PREFIX}/share/pixmaps/ && rm -rf ${PREFIX}/share/pixmaps/synfigstudio
}

mkpackage()
{
#[ ! -d DISTPREFIX ] || rm -rf $DISTPREFIX
#mkdir -p $DISTPREFIX/bin
#cp  ${PREFIX}/bin/*.exe $DISTPREFIX/bin
#cp  ${PREFIX}/bin/*.dll $DISTPREFIX/bin
#cp -rf ${PREFIX}/etc $DISTPREFIX/etc
#mkdir -p $DISTPREFIX/lib
#cp -rf ${PREFIX}/lib/synfig $DISTPREFIX/lib
#mkdir -p $DISTPREFIX/share
#cp -rf ${PREFIX}/share/pixmaps $DISTPREFIX/share
#cp -rf ${PREFIX}/share/synfig $DISTPREFIX/share
cp -rf $SCRIPTPATH/../synfig-core/examples $PREFIX/
mkdir -p $PREFIX/licenses
cp -rf $SCRIPTPATH/../synfig-studio/COPYING $PREFIX/licenses/synfigstudio.txt
cp -rf $SCRIPTPATH/../synfig-studio/images/installer_logo.bmp $PREFIX/

[ -d $CACHEDIR ] || mkdir -p $CACHEDIR
cd $CACHEDIR

[ -e ffmpeg-latest-win32-static.7z ] || wget http://ffmpeg.zeranoe.com/builds/win32/static/ffmpeg-latest-win32-static.7z
[ ! -d ffmpeg ] || rm -rf ffmpeg
mkdir -p ffmpeg
cd ffmpeg
7z e ../ffmpeg-latest-win32-static.7z
cp ffmpeg.exe $PREFIX/bin
[ -d $PREFIX/licenses ] || mkdir -p $PREFIX/licenses
cp *.txt $PREFIX/licenses
cd ..
rm -rf ffmpeg

[ -e portable-python-3.2.5.1.zip ] || wget http://download.tuxfamily.org/synfig/packages/sources/portable-python-3.2.5.1.zip
[ ! -d python ] || rm -rf python
unzip portable-python-3.2.5.1.zip
[ ! -d $PREFIX/python ] || rm -rf $PREFIX/python
mv python $PREFIX

cd $PREFIX
cp -f $SCRIPTPATH/synfigstudio.nsi $PREFIX/synfigstudio.nsi
makensis synfigstudio.nsi
}

mkall()
{
	mkprep
	mketl
	mksynfig
	mksynfigstudio
	mkpackage
}

if [ -z $1 ]; then
	mkall
else
	echo "Executing custom user command..."
	$@
fi

