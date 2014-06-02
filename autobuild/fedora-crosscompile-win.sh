#!/bin/sh

set -e

export SCRIPTPATH=$(cd `dirname "$0"`; pwd)

if [ -z $ARCH ]; then
	export ARCH="32"
fi

if [ -z $THREADS ]; then
	export THREADS=4
fi

export TOOLCHAIN="mingw$ARCH" # mingw32 | mingw64

if [[ $TOOLCHAIN == "mingw32" ]]; then
    export TOOLCHAIN_HOST="i686-w64-mingw32"
    export EXT_ARCH=i386
elif [[ $TOOLCHAIN == "mingw64" ]]; then
    export TOOLCHAIN_HOST="x86_64-w64-mingw32"
    export EXT_ARCH=x86_64
else
    echo "Error: Unknown toolchain"
    exit 1
fi

export WORKSPACE=$HOME/synfig-buildroot
export PREFIX=$WORKSPACE/win$ARCH/build
export DISTPREFIX=$WORKSPACE/win$ARCH/dist
export SRCPREFIX=$WORKSPACE/win$ARCH/source
export CACHEDIR=$WORKSPACE/cache
export PKG_CONFIG_PATH=${PREFIX}/lib/pkgconfig:/usr/${TOOLCHAIN_HOST}/sys-root/mingw/lib/pkgconfig/
export PKG_CONFIG_LIBDIR=${PREFIX}/lib/pkgconfig
export PATH=${PREFIX}/bin:$PATH
export LD_LIBRARY_PATH=${PREFIX}/lib

export CC=${TOOLCHAIN_HOST}-gcc
export CXX=${TOOLCHAIN_HOST}-g++
export CPPFLAGS=" -I/usr/${TOOLCHAIN_HOST}/sys-root/mingw/include -I${PREFIX}/include"
export LDFLAGS=" -L/usr/${TOOLCHAIN_HOST}/sys-root/mingw/lib -L${PREFIX}/lib "


if [ -z $DEBUG ]; then
	export DEBUG=0
fi

if [[ $DEBUG == 1 ]]; then
	echo
	echo "Debug mode: enabled"
	echo
	DEBUG='--enable-debug --enable-optimization=0'
else
	DEBUG=''
fi

[ -e $SRCPREFIX ] || mkdir -p $SRCPREFIX
[ -e $CACHE ] || mkdir -p $CACHE

export VERSION=`cat ${SCRIPTPATH}/../synfig-core/configure.ac |egrep "AC_INIT\(\[Synfig Core\],"| sed "s|.*Core\],\[||" | sed "s|\],\[.*||"`
pushd "${SCRIPTPATH}" > /dev/null
export REVISION=`git show --pretty=format:%ci HEAD |  head -c 10 | tr -d '-'`
popd > /dev/null

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
		${TOOLCHAIN}-gcc-c++ \
		${TOOLCHAIN}-libxml++ \
		${TOOLCHAIN}-cairo \
		${TOOLCHAIN}-pango \
		${TOOLCHAIN}-boost \
		${TOOLCHAIN}-libjpeg-turbo \
		${TOOLCHAIN}-gtkmm24 \
		${TOOLCHAIN}-glibmm24 \
		${TOOLCHAIN}-libltdl \
		${TOOLCHAIN}-dlfcn \
		${TOOLCHAIN}-SDL \
		mingw32-nsis \
		p7zip \
		ImageMagick \
		"
fi

# copy libs
[ -d ${PREFIX}/bin ] || mkdir -p ${PREFIX}/bin
for file in \
   av*.dll \
   ffmpeg.exe \
   iconv.dll \
   libatk-\*.dll \
   libatkmm-1.6-1.dll \
   libboost_program_options\*.dll \
   libbz2\*.dll \
   libcairo\*.dll \
   libdl.dll \
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
   libgomp*.dll \
   libgthread\*.dll \
   libgtk\*.dll \
   libharfbuzz\*.dll \
   libiconv\*.dll \
   libintl\*.dll \
   libjasper\*.dll \
   libjpeg\*.dll \
   libltdl*.dll \
   liblzma\*.dll \
   libMagick*.dll \
   libmlt*.dll \
   libogg*.dll \
   libpango\*.dll \
   libpixman\*.dll \
   libpng\*.dll \
   libsamplerate*.dll \
   libsigc\*.dll \
   libsox*.dll \
   libstdc++\*.dll \
   libsynfig\*.dll \
   libtiff\*.dll \
   libturbojpeg.dll \
   libvorbis*.dll \
   libwinpthread*.dll \
   libxml2\*.dll \
   libxml++\*.dll \
   libz\*.dll \
   postproc*.dll \
   pthread\*.dll \
   SDL.dll \
   swscale*.dll \
   swresample*.dll \
   zlib\*.dll \
   convert.exe \
   pango-querymodules.exe \
   synfig.exe \
   synfigstudio.exe \
# this extra line is required!
do
	cp /usr/${TOOLCHAIN_HOST}/sys-root/mingw/bin/$file ${PREFIX}/bin || true
done

[ -d ${PREFIX}/etc ] || mkdir -p ${PREFIX}/etc
for file in \
   fonts \
   gtk-2.0 \
   pango \
# this extra line is required!
do
	cp -rf /usr/${TOOLCHAIN_HOST}/sys-root/mingw/etc/$file ${PREFIX}/etc
done

[ -d ${PREFIX}/lib ] || mkdir -p ${PREFIX}/lib
for file in \
   gtk-2.0 \
   gdk-pixbuf-2.0 \
   pango \
   pkgconfig \
# this extra line is required!
do
	cp -rf /usr/${TOOLCHAIN_HOST}/sys-root/mingw/lib/$file ${PREFIX}/lib
done

[ -d ${PREFIX}/share ] || mkdir -p ${PREFIX}/share
for file in \
   fontconfig \
   themes \
   xml \
# this extra line is required!
do
	cp -rf /usr/${TOOLCHAIN_HOST}/sys-root/mingw/share/$file ${PREFIX}/share || true
done

# cleaning source tree
for dir in ETL synfig-core synfig-studio; do
	pushd $SCRIPTPATH/../$dir > /dev/null
	make clean || true
	popd > /dev/null
done

}

#ImageMagick
mkimagemagick()
{
PKG_NAME=ImageMagick
#PKG_VERSION=6.8.6-10
PKG_VERSION=6.8.7-10
#PKG_VERSION=6.8.8-7
TAREXT=bz2

if ! pkg-config ${PKG_NAME} --exact-version=${PKG_VERSION%-*}  --print-errors; then
	cd $CACHEDIR
	[ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://www.imagemagick.org/download/legacy/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd $SRCPREFIX
	if [ ! -d ${PKG_NAME}-${PKG_VERSION} ]; then
		tar -xjf $CACHEDIR/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	fi
	cd ${PKG_NAME}-${PKG_VERSION}
	[ ! -e config.cache ] || rm config.cache
	${TOOLCHAIN}-configure \
	--prefix=${PREFIX} \
	--exec-prefix=${PREFIX} \
	--bindir=${PREFIX}/bin \
	--sbindir=${PREFIX}/sbin \
	--libexecdir=${PREFIX}/lib \
	--datadir=${PREFIX}/share \
	--localstatedir=${PREFIX}/var \
	--sysconfdir=${PREFIX}/etc \
	--datarootdir=${PREFIX}/share \
	--datadir=${PREFIX}/share \
	--includedir=${PREFIX}/include \
	--libdir=${PREFIX}/lib \
	--mandir=${PREFIX}/share/man \
	--program-prefix="" \
	--disable-static --enable-shared \
	--without-modules \
	--without-perl \
	--without-x \
	--without-wmf \
	--with-threads \
	--with-magick_plus_plus

	make install -j$THREADS
fi
}

mklibogg()
{

PKG_NAME=libogg
PKG_VERSION=1.3.1
TAREXT=gz

if ! pkg-config ogg --exact-version=${PKG_VERSION}  --print-errors; then
    cd $CACHEDIR
    [ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://downloads.xiph.org/releases/ogg/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    cd $SRCPREFIX
    if [ ! -d ${PKG_NAME}-${PKG_VERSION} ]; then
        tar -xzf $CACHEDIR/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    fi
    cd ${PKG_NAME}-${PKG_VERSION}
    [ ! -e config.cache ] || rm config.cache
    ${TOOLCHAIN}-configure \
        --prefix=${PREFIX} \
        --exec-prefix=${PREFIX} \
        --bindir=${PREFIX}/bin \
        --sbindir=${PREFIX}/sbin \
        --libexecdir=${PREFIX}/lib \
        --libdir=${PREFIX}/lib \
        --includedir=${PREFIX}/include \
        --datadir=${PREFIX}/share \
        --localstatedir=${PREFIX}/var \
        --sysconfdir=${PREFIX}/etc \
        --datarootdir=${PREFIX}/share

    make all -j$THREADS
    make install -j$THREADS

fi
}

mklibvorbis()
{
mklibogg

PKG_NAME=libvorbis
PKG_VERSION=1.3.4
TAREXT=gz

if ! pkg-config vorbis --exact-version=${PKG_VERSION}  --print-errors; then
    cd $CACHEDIR
    [ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://downloads.xiph.org/releases/vorbis/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    cd $SRCPREFIX
    if [ ! -d ${PKG_NAME}-${PKG_VERSION} ]; then
        tar -xzf $CACHEDIR/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    fi
    cd ${PKG_NAME}-${PKG_VERSION}
    [ ! -e config.cache ] || rm config.cache
    ${TOOLCHAIN}-configure \
        --prefix=${PREFIX} \
        --exec-prefix=${PREFIX} \
        --bindir=${PREFIX}/bin \
        --sbindir=${PREFIX}/sbin \
        --libexecdir=${PREFIX}/lib \
        --libdir=${PREFIX}/lib \
        --includedir=${PREFIX}/include \
        --datadir=${PREFIX}/share \
        --localstatedir=${PREFIX}/var \
        --sysconfdir=${PREFIX}/etc \
        --datarootdir=${PREFIX}/share

    make all -j$THREADS
    make install -j$THREADS

fi
}

mklibsamplerate()
{
PKG_NAME=libsamplerate
PKG_VERSION=0.1.8
TAREXT=gz

if ! pkg-config samplerate --exact-version=${PKG_VERSION}  --print-errors; then
    cd $CACHEDIR
    [ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://www.mega-nerd.com/SRC/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    cd $SRCPREFIX
    if [ ! -d ${PKG_NAME}-${PKG_VERSION} ]; then
        tar -xzf $CACHEDIR/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    fi
    cd ${PKG_NAME}-${PKG_VERSION}
    [ ! -e config.cache ] || rm config.cache
    ${TOOLCHAIN}-configure \
        --prefix=${PREFIX} \
        --exec-prefix=${PREFIX} \
        --bindir=${PREFIX}/bin \
        --sbindir=${PREFIX}/sbin \
        --libexecdir=${PREFIX}/lib \
        --libdir=${PREFIX}/lib \
        --includedir=${PREFIX}/include \
        --datadir=${PREFIX}/share \
        --localstatedir=${PREFIX}/var \
        --sysconfdir=${PREFIX}/etc \
        --datarootdir=${PREFIX}/share

    make all -j$THREADS
    make install -j$THREADS

fi
}

mksox()
{
PKG_NAME=sox
PKG_VERSION=14.4.1
TAREXT=gz

if ! pkg-config ${PKG_NAME} --exact-version=${PKG_VERSION}  --print-errors; then
    cd $CACHEDIR
    [ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://download.tuxfamily.org/synfig/packages/sources/base/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    cd $SRCPREFIX
    if [ ! -d ${PKG_NAME}-${PKG_VERSION} ]; then
        tar -xzf $CACHEDIR/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    fi
    cd ${PKG_NAME}-${PKG_VERSION}
    [ ! -e config.cache ] || rm config.cache
    ${TOOLCHAIN}-configure \
        --prefix=${PREFIX} \
        --exec-prefix=${PREFIX} \
        --bindir=${PREFIX}/bin \
        --sbindir=${PREFIX}/sbin \
        --libexecdir=${PREFIX}/lib \
        --libdir=${PREFIX}/lib \
        --includedir=${PREFIX}/include \
        --mandir=${PREFIX}/man \
        --datadir=${PREFIX}/share \
        --localstatedir=${PREFIX}/var \
        --sysconfdir=${PREFIX}/etc \
        --datarootdir=${PREFIX}/share 

    make all -j$THREADS
    make install -j$THREADS

fi
}

mkffmpeg()
{
    export FFMPEG_VERSION=2.2.2
    if ! pkg-config libswscale --exact-version=${FFMPEG_VERSION}  --print-errors; then
        cd $CACHEDIR
        [ -e ffmpeg-${FFMPEG_VERSION}-win${ARCH}-dev.7z ] || wget http://ffmpeg.zeranoe.com/builds/win${ARCH}/dev/ffmpeg-${FFMPEG_VERSION}-win${ARCH}-dev.7z
        [ -e ffmpeg-${FFMPEG_VERSION}-win${ARCH}-shared.7z ] || wget http://ffmpeg.zeranoe.com/builds/win${ARCH}/shared/ffmpeg-${FFMPEG_VERSION}-win${ARCH}-shared.7z
        [ ! -d ffmpeg ] || rm -rf ffmpeg
        cd $SRCPREFIX
        mkdir -p ffmpeg
        cd ffmpeg
        7z x -y $CACHEDIR/ffmpeg-${FFMPEG_VERSION}-win${ARCH}-dev.7z
        cp -rf ffmpeg-${FFMPEG_VERSION}-win${ARCH}-dev/include/* ${PREFIX}/include/
        cp -rf ffmpeg-${FFMPEG_VERSION}-win${ARCH}-dev/lib/* ${PREFIX}/lib/
        7z x -y $CACHEDIR/ffmpeg-${FFMPEG_VERSION}-win${ARCH}-shared.7z
        cp -rf ffmpeg-${FFMPEG_VERSION}-win${ARCH}-shared/bin/ffmpeg.exe ${PREFIX}/bin
        cp -rf ffmpeg-${FFMPEG_VERSION}-win${ARCH}-shared/bin/*.dll ${PREFIX}/bin
        mkdir -p ${PREFIX}/share/ffmpeg/presets/ || true
        cp -rf ffmpeg-${FFMPEG_VERSION}-win${ARCH}-shared/presets/* /${PREFIX}/share/ffmpeg/presets/

		for PKG in libswscale libavformat libavdevice; do
			cat > ${PREFIX}/lib/pkgconfig/${PKG}.pc <<EOF
prefix=${PREFIX}
exec_prefix=${PREFIX}
libdir=${PREFIX}/lib
includedir=${PREFIX}/include

Name: ${PKG}
Description: Dynamic module loader for GLib
Version: ${FFMPEG_VERSION}

EOF
		done
    fi
}

mkmlt()
{
mkffmpeg
mklibsamplerate
mklibvorbis
mksox
	
PKG_NAME=mlt
PKG_VERSION=0.9.1
TAREXT=gz

if ! pkg-config ${PKG_NAME}\+\+ --exact-version=${PKG_VERSION}  --print-errors; then

    export CPPFLAGS=" -I/usr/${TOOLCHAIN_HOST}/sys-root/mingw/include/SDL $CPPFLAGS"
    export LDFLAGS=" $LDFLAGS -lmingw32 -lSDLmain -lSDL -mwindows"
	
    #[ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://download.tuxfamily.org/synfig/packages/sources/base/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    #if [ ! -d ${PKG_NAME}-${PKG_VERSION} ]; then
    #    tar -xzf ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    #fi
    #cd ${PKG_NAME}-${PKG_VERSION}
    cd $SRCPREFIX
    if [ ! -d ${PKG_NAME} ]; then
        git clone https://github.com/morevnaproject/mlt
    fi
    cd mlt
    [ ! -e config.cache ] || rm config.cache
    rm -rf ${PREFIX}/lib/libmlt* || true
    rm -rf ${PREFIX}/bin/libmlt* || true

    ${TOOLCHAIN}-configure \
        --prefix=${PREFIX} \
        --exec-prefix=${PREFIX} \
        --bindir=${PREFIX}/bin \
        --sbindir=${PREFIX}/sbin \
        --libexecdir=${PREFIX}/lib \
        --libdir=${PREFIX}/lib \
        --datadir=${PREFIX}/share \
        --mandir=${PREFIX}/man \
        --localstatedir=${PREFIX}/var \
        --sysconfdir=${PREFIX}/etc \
        --datarootdir=${PREFIX}/share \
        --avformat-shared=${PREFIX}/ \
        --enable-gpl --disable-decklink \
        --target-os=MinGW --target-arch=$EXT_ARCH \
        $DEBUG

    make all -j$THREADS
    make install -j$THREADS

    mv ${PREFIX}/melt ${PREFIX}/bin/melt.exe
    mv ${PREFIX}/libmlt*.dll ${PREFIX}/bin

    mkdir -p ${PREFIX}/bin/lib || true
    mkdir -p ${PREFIX}/bin/share || true
    cp -rf ${PREFIX}/lib/mlt ${PREFIX}/bin/lib/
    cp -rf ${PREFIX}/share/mlt ${PREFIX}/bin/share/

fi
}

#ETL
mketl()
{
cd $SCRIPTPATH/../ETL
make clean || true
autoreconf --install --force
${TOOLCHAIN}-configure --prefix=${PREFIX} --includedir=${PREFIX}/include --libdir=${PREFIX}/lib --bindir=${PREFIX}/bin $DEBUG
make install
}

#synfig-core
mksynfig()
{
cd $SCRIPTPATH/../synfig-core/
make clean || true
[ ! -e config.cache ] || rm config.cache
libtoolize --ltdl --copy --force
autoreconf --install --force
cp ./configure ./configure.real
echo -e "#/bin/sh \n export PKG_CONFIG_PATH=${PREFIX}/lib/pkgconfig:/usr/${TOOLCHAIN_HOST}/sys-root/mingw/lib/pkgconfig/:$PKG_CONFIG_PATH \n ./configure.real \$@  \n " > ./configure
chmod +x ./configure
${TOOLCHAIN}-configure --prefix=${PREFIX} --includedir=${PREFIX}/include --disable-static --enable-shared --with-magickpp --without-libavcodec --libdir=${PREFIX}/lib --bindir=${PREFIX}/bin --sysconfdir=${PREFIX}/etc --with-boost=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/ --enable-warnings=minimum $DEBUG
make install -j$THREADS
}

#synfig-studio
mksynfigstudio()
{
cd $SCRIPTPATH/../synfig-studio/
make clean || true
[ ! -e config.cache ] || rm config.cache
/bin/sh ./bootstrap.sh
cp ./configure ./configure.real
echo -e "#/bin/sh \n export PKG_CONFIG_PATH=${PREFIX}/lib/pkgconfig:/usr/${TOOLCHAIN_HOST}/sys-root/mingw/lib/pkgconfig/:$PKG_CONFIG_PATH \n ./configure.real \$@  \n " > ./configure
chmod +x ./configure
${TOOLCHAIN}-configure --prefix=${PREFIX} --includedir=${PREFIX}/include --disable-static --enable-shared --libdir=${PREFIX}/lib --bindir=${PREFIX}/bin --sysconfdir=${PREFIX}/etc --datadir=${PREFIX}/share  $DEBUG
make install -j$THREADS
cp -rf ${PREFIX}/share/pixmaps/synfigstudio/* ${PREFIX}/share/pixmaps/ && rm -rf ${PREFIX}/share/pixmaps/synfigstudio

[ -e ${PREFIX}/etc/gtk-2.0 ] || mkdir -p ${PREFIX}/etc/gtk-2.0
cat > ${PREFIX}/etc/gtk-2.0/gtkrc <<EOF

# Enable native look
gtk-theme-name = "MS-Windows"

# Use small toolbar buttons
gtk-toolbar-style = 0

EOF
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

[ -e portable-python-3.2.5.1.zip ] || wget http://download.tuxfamily.org/synfig/packages/sources/portable-python-3.2.5.1.zip
[ ! -d python ] || rm -rf python
unzip portable-python-3.2.5.1.zip
[ ! -d $PREFIX/python ] || rm -rf $PREFIX/python
mv python $PREFIX

gen_list_nsh()
{
[ ! -e $2.nsh ] || rm $2.nsh
[ ! -e $2-uninst.nsh ] || rm $2-uninst.nsh
for line in `find $1 -print`; do
	directory=`dirname $line`
	line1=`echo $directory | sed "s|\./||g" | sed "s|/|\\\\\|g"`
	line2=`echo $line | sed "s|\./||g" | sed "s|/|\\\\\|g"`
	if [ -d $line ]; then
		echo "RMDir \"\$INSTDIR\\$line2\"" >> $2-uninst.nsh
	elif [ ! -L $line ]; then
		echo "SetOutPath \"\$INSTDIR\\$line1\""  >> $2.nsh
		echo "File \"$line2\"" >> $2.nsh
		echo "Delete \"\$INSTDIR\\$line2\"" >> $2-uninst.nsh
	fi
done
# reverse order of uninstall commands
cp $2-uninst.nsh $2-uninst.nsh.tmp
tac $2-uninst.nsh.tmp > $2-uninst.nsh
rm $2-uninst.nsh.tmp
}

cd $PREFIX

#generate file lists

gen_list_nsh bin bin
sed -i '/ffmpeg\.exe/d' bin.nsh		# exclude ffmpeg from he list of binaries - it will go into separate group
gen_list_nsh etc etc
gen_list_nsh examples examples
gen_list_nsh lib/gtk-2.0 lib-gtk
gen_list_nsh lib/synfig lib-synfig
gen_list_nsh licenses licenses
#gen_list_nsh python python # -- takes too long
gen_list_nsh share/locale share-locale
gen_list_nsh share/pixmaps share-pixmaps
gen_list_nsh share/synfig share-synfig
gen_list_nsh share/themes share-themes

cp -f $SCRIPTPATH/synfigstudio.nsi $PREFIX/synfigstudio.nsi
sed -i "s/@VERSION@/$VERSION/g" $PREFIX/synfigstudio.nsi
cp -f $SCRIPTPATH/win${ARCH}-specific.nsh $PREFIX/arch-specific.nsh
makensis synfigstudio.nsi

mv synfigstudio-${VERSION}.exe ../../synfigstudio-${VERSION}-${REVISION}-${ARCH}bit.exe
}

mkall()
{
	mkprep
	mkimagemagick
	mkmlt
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

