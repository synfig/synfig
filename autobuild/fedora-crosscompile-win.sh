#!/bin/sh

export SCRIPTPATH=`dirname "$0"`

yum install -y mingw32-gcc-c++ mingw32-libxml++ mingw32-cairo mingw32-pango mingw32-boost mingw32-libjpeg-turbo

export PREFIX=~/synfig-win
export PKG_CONFIG_PATH=${PREFIX}/lib/pkgconfig:$PKG_CONFIG_PATH
export PKG_CONFIG_LIBDIR=${PREFIX}/lib/pkgconfig:$PKG_CONFIG_LIBDIR
export PATH=${PREFIX}/bin:$PATH
export LD_LIBRARY_PATH=${PREFIX}/lib:$LD_LIBRARY_PATH
export LDFLAGS="-Wl,-rpath -Wl,\\\$\$ORIGIN/lib"
	
#ETL
cd $SCRIPTPATH/../ETL

autoreconf --install --force
mingw32-configure --prefix=${PREFIX} --includedir=${PREFIX}/include --libdir=${PREFIX}/lib --bindir=${PREFIX}/bin $DEBUG
make install


#synfig-core

cd ../synfig-core/

libtoolize --ltdl --copy --force && \
autoreconf --install --force && \
cp ./configure ./configure.real && echo -e "#/bin/sh \n export PKG_CONFIG_PATH=${PREFIX}/lib/pkgconfig:$PKG_CONFIG_PATH \n ./configure.real \$@  \n " > ./configure && \
chmod +x ./configure
mingw32-configure --prefix=${PREFIX} --includedir=${PREFIX}/include --disable-static --enable-shared --with-magickpp --without-libavcodec --libdir=${PREFIX}/lib --bindir=${PREFIX}/bin --sysconfdir=${PREFIX}/etc --with-boost=/usr/i686-w64-mingw32/sys-root/mingw/ --enable-warnings=minimum $DEBUG
make install -j4

#synfig-studio

cd ../synfig-studio/

#TODO


#pack

#TODO
