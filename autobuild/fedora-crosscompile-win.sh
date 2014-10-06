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
elif [[ $TOOLCHAIN == "mingw64" ]]; then
    export TOOLCHAIN_HOST="x86_64-w64-mingw32"
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
export PATH=${PREFIX}/bin:$PATH
export LD_LIBRARY_PATH=${PREFIX}/lib


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
		${TOOLCHAIN}-libtiff \
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
   libpango\*.dll \
   libpixman\*.dll \
   libpng\*.dll \
   libsigc\*.dll \
   libstdc++\*.dll \
   libsynfig\*.dll \
   libtiff\*.dll \
   libturbojpeg.dll \
   libwinpthread*.dll \
   libxml2\*.dll \
   libxml++\*.dll \
   libz\*.dll \
   pthread\*.dll \
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

export PREP_VERSION=1

if [[ `cat "$PREFIX/prep-done"` != "${PREP_VERSION}" ]]; then

mkimagemagick

echo ${PREP_VERSION} > "$PREFIX/prep-done"

fi
}

#ImageMagick
mkimagemagick()
{
PKG_NAME=ImageMagick
#PKG_VERSION=6.8.6-10
PKG_VERSION=6.8.7-10
#PKG_VERSION=6.8.8-7
TAREXT=bz2

cd $CACHEDIR
[ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://www.imagemagick.org/download/legacy/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
cd $SRCPREFIX
if [ ! -d ${PKG_NAME}-${PKG_VERSION} ]; then
    tar -xjf $CACHEDIR/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    cd ${PKG_NAME}-${PKG_VERSION}
else
    cd ${PKG_NAME}-${PKG_VERSION}
fi
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

