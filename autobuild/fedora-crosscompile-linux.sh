#!/bin/bash

# TODO: LD_PRELOAD wrapper
# TODO: Bundle ALL dependent lib (libpng issue)
# TODO: GTK theming issues
# TODO: FFmpeg/libx264 - stick to particular version
# TODO: FFmpeg/libx264/mlt - cache sources
# TODO: Debug builds for dependent libraries
# TODO: Use BUILDROOT_LIBRARY_SET_ID

set -e

export SCRIPTPATH=$(cd `dirname "$0"`; pwd)

RELEASE=9

BUILDROOT_VERSION=5
BUILDROOT_LIBRARY_SET_ID=2

if [ -z $ARCH ]; then
	if [[ `uname -i` == "x86_64" ]]; then
		export ARCH="64"
	else
		export ARCH="32"
	fi
fi

if [ -z $THREADS ]; then
	export THREADS=2
fi

if [ -z $DEBUG ]; then
	export DEBUG=0
fi

if [[ $DEBUG == 1 ]]; then
	echo
	echo "Debug mode: enabled"
	echo
	DEBUG_OPT='--enable-debug --enable-optimization=0'
	export SUFFIX="-debug"
else
	DEBUG_OPT=''
fi

if [ ! -z $SUBSET ]; then
	export SUFFIX="-$SUBSET"
fi

export WORKSPACE=$HOME/synfig-buildroot
export SYSPREFIX=$WORKSPACE/linux$ARCH$SUFFIX/sys
export PREFIX=$WORKSPACE/linux$ARCH$SUFFIX/build
#export DEPSPREFIX=$WORKSPACE/linux$ARCH$SUFFIX/build
export DEPSPREFIX=$WORKSPACE/linux$ARCH$SUFFIX/sys-deps
export TOOLSPREFIX=$WORKSPACE/linux$ARCH$SUFFIX/tools
export SRCPREFIX=$WORKSPACE/linux$ARCH$SUFFIX/source
export DISTPREFIX=$WORKSPACE/linux$ARCH$SUFFIX/dist
export CACHEDIR=$WORKSPACE/cache

[ -e ${SRCPREFIX} ] || mkdir -p ${SRCPREFIX}
[ -e ${WORKSPACE}/cache ] || mkdir -p ${WORKSPACE}/cache

export EMAIL='root@synfig.org'
SOURCES_URL="rsync://download.tuxfamily.org/pub/synfig/packages/sources/base"

# Bundled libraries
LIBSIGCPP_VERSION=2.2.10
GLEW_VERSION=1.5.1
CAIROMM_VERSION=1.10.0
IMAGEMAGICK_VERSION=6.8.9
PANGOMM_VERSION=2.34.0		# required by GTKMM 2.20.3
GTKMM_VERSION=3.10.1
FTGL_VERSION=2.1.2
FREEGLUT_VERSION=2.4.0
GTKGLEXT_VERSION=1.2.0
GTKGLEXTMM_VERSION=1.2.0
LIBXMLPP_VERSION=2.22.0
GLIBMM_VERSION=2.38.1		# required by GTKMM 3.10.1
CAIRO_VERSION=1.12.18
BOOST_VERSION=1_53_0
ATK_VERSION=2.10.0
AT_SPI2_VERSION=2.10.2
AT_SPI2_ATK_VERSION=2.10.2
GLIB_VERSION=2.38.2
GDK_PIXBUF_VERSION=2.30.3
GTK_VERSION=3.10.9
PIXMAN_VERSION=0.30.0		# required by CAIRO 1.12.0
HARFBUZZ_VERSION=0.9.24
PANGO_VERSION=1.36.1
ATKMM_VERSION=2.22.7

# System libraries
FONTCONFIG_VERSION=2.11.0
JACK_VERSION=0.124.1

if [[ $ARCH == "32" ]]; then
	export SYS_ARCH=i386
	export RPM_ARCH=i386
	export GCC_ARCH=i486
	export LIBDIR="lib"
	export HOST=i686-pc-linux-gnu
	if ( cat /etc/issue | egrep "Ubuntu" ); then
		export UBUNTU_LIBDIR="/lib/i386-linux-gnu/"
	fi
else
	export SYS_ARCH=amd64
	export RPM_ARCH=x86_64
	export GCC_ARCH=x86_64
	export LIBDIR="lib64"
	export HOST=x86_64-pc-linux-gnu
	if ( cat /etc/issue | egrep "Ubuntu" ); then
		export UBUNTU_LIBDIR="/lib/x86_64-linux-gnu/"
	fi
fi

#if [[ `uname -i` == "x86_64" ]]; then
#export NATIVE_LIBDIR="lib64"


pushd "${SCRIPTPATH}" > /dev/null
export VERSION=`cat ${SCRIPTPATH}/../synfig-core/configure.ac |egrep "AC_INIT\(\[Synfig Core\],"| sed "s|.*Core\],\[||" | sed "s|\],\[.*||"`
export REVISION=`git show --pretty=format:%ci HEAD |  head -c 10 | tr -d '-'`
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
if [[ ${VERSION##*-RC} != ${VERSION} ]]; then
	#if [[ $BREED == 'master' ]]; then
		BREED=rc${VERSION##*-RC}
	#else
	#	BREED=rc${VERSION##*-RC}.$BREED
	#fi
	VERSION=${VERSION%%-*}
fi
[[ $DEBUG == 1 ]] && BREED=${BREED}.dbg
BREED=`echo $BREED | tr _ . | tr - .`	# No "-" or "_" characters, becuse RPM and DEB complain

popd > /dev/null

set_environment()
{
	#export LD_LIBRARY_PATH=${DEPSPREFIX}/lib:/${LIBDIR}:${SYSPREFIX}/${LIBDIR}:${SYSPREFIX}/usr/${LIBDIR}
	#export LD_LIBRARY_PATH=${SYSPREFIX}/lib-native:${PREFIX}/lib:${DEPSPREFIX}/lib:${SYSPREFIX}/${LIBDIR}:${SYSPREFIX}/usr/${LIBDIR}
	if ( cat /etc/issue | egrep "Ubuntu" ); then
		export LD_PRELOAD=${UBUNTU_LIBDIR}/libc.so.6:${UBUNTU_LIBDIR}/libpthread.so.0:${UBUNTU_LIBDIR}/libdl.so.2
	else
		export LD_PRELOAD=/${LIBDIR}/libc.so.6:/${LIBDIR}/libpthread.so.0:/${LIBDIR}/libdl.so.2
	fi
	export LD_LIBRARY_PATH=${PREFIX}/lib:${DEPSPREFIX}/lib:${SYSPREFIX}/${LIBDIR}:${SYSPREFIX}/usr/${LIBDIR}
	
	export PATH=${PREFIX}/bin:${DEPSPREFIX}/bin:${SYSPREFIX}/bin:${SYSPREFIX}/usr/bin
	export LDFLAGS="-Wl,-rpath -Wl,\\\$\$ORIGIN/lib -L${PREFIX}/lib -L${DEPSPREFIX}/lib -L${SYSPREFIX}/${LIBDIR} -L${SYSPREFIX}/usr/${LIBDIR}"
	#export CFLAGS=" -nostdinc  -I${SYSPREFIX}/usr/lib/gcc/x86_64-linux-gnu/4.3.2/include -I${SYSPREFIX}/usr/lib/gcc/x86_64-linux-gnu/4.3.2/include-fixed  -I${PREFIX}/include  -I${DEPSPREFIX}/include -I${SYSPREFIX}/usr/include"
	GCC_VER=4.4
	export CFLAGS="-I${SYSPREFIX}/usr/include -I${PREFIX}/include -I${SYSPREFIX}/usr/include/${GCC_ARCH}-linux-gnu" 
	#export CXXFLAGS="-I${SYSPREFIX}/usr/include/linux/  -I${SYSPREFIX}/usr/include/c++/${GCC_VER}/ -I${SYSPREFIX}/usr/include/c++/${GCC_VER}/${GCC_ARCH}-linux-gnu/ -I${SYSPREFIX}/usr/lib/gcc/${GCC_ARCH}-linux-gnu/${GCC_VER}/include/ -I${SYSPREFIX}/usr/lib/gcc/${GCC_ARCH}-linux-gnu/${GCC_VER}/include-fixed/  -I${SYSPREFIX}/usr/${GCC_ARCH}-linux-gnu/include"
	#export CXXFLAGS="-I${SYSPREFIX}/usr/local/include/x86_64-linux-gnu -I${SYSPREFIX}/usr/lib/gcc/x86_64-linux-gnu/4.4.5/include -I${SYSPREFIX}/usr/lib/gcc/x86_64-linux-gnu/4.4.5/include-fixed -I${SYSPREFIX}/usr/lib/gcc/../../x86_64-linux-gnu/include -I${SYSPREFIX}/usr/include/x86_64-linux-gnu"
	#export CXXFLAGS=" -nostdinc   -I${SYSPREFIX}/usr/lib/gcc/../../include/c++/4.3  -I${SYSPREFIX}/usr/lib/gcc/../../include/c++/4.3/x86_64-linux-gnu -I${SYSPREFIX}/usr/lib/gcc/../../include/c++/4.3/backward -I${SYSPREFIX}/usr/lib/gcc/x86_64-linux-gnu/4.3.2/include -I${SYSPREFIX}/usr/lib/gcc/x86_64-linux-gnu/4.3.2/include-fixed -I${PREFIX}/include  -I${DEPSPREFIX}/include -I${SYSPREFIX}/usr/include"
	export PKG_CONFIG_PATH=${PREFIX}/lib/pkgconfig:${DEPSPREFIX}/lib/pkgconfig:${SYSPREFIX}/usr/lib/pkgconfig:${SYSPREFIX}/usr/share/pkgconfig
	PERL_VERSION=`perl -v | grep "This is perl" | sed "s|This is perl, v||g" | cut -f 1 -d " "`
	export NM=nm
	export PERL5LIB="${SYSPREFIX}/etc/perl:${DEPSPREFIX}/lib/perl/${PERL_VERSION}:${DEPSPREFIX}/share/perl/${PERL_VERSION}:${SYSPREFIX}/usr/lib/perl5:${SYSPREFIX}/usr/share/perl5:${SYSPREFIX}/usr/lib/perl/${PERL_VERSION}:${SYSPREFIX}/usr/share/perl/${PERL_VERSION}:${DEPSPREFIX}/lib/site_perl"
	if [[ $ARCH == "32" ]]; then
		export CFLAGS="$CFLAGS -m32"
		export CXXFLAGS="$CXXFLAGS -m32"
		export LDFLAGS="$LDFLAGS -m32"
	fi
}

run_native()
{
	LD_PRELOAD=""
	LD_LIBRARY_PATH=""
	PATH="/usr/local/bin/:/usr/sbin:/usr/bin:/bin"
	LDFLAGS=""
	CFLAGS=""
	CXXFLAGS=""
	PKG_CONFIG_PATH=""
	PERL5LIB=""
	if [[ $ARCH == "32" ]]; then
		export CFLAGS="$CFLAGS -m32"
		export CXXFLAGS="$CXXFLAGS -m32"
		export LDFLAGS="$LDFLAGS -m32"
	fi
	"$@"
	set_environment
}

mkprefix_()
{
	[ ! -e ${SYSPREFIX}/dev ] || rm -rf ${SYSPREFIX}/dev
	[ ! -e ${SYSPREFIX}/proc ] || rm -rf ${SYSPREFIX}/proc
	
	#deb http://debootstrap.invalid/ lenny main
	
	LD_LIBRARY_PATH=${UBUNTU_LIBDIR}:/${LIBDIR}:${SYSPREFIX}/usr/${LIBDIR} \
	PATH=/usr/local/sbin:/usr/sbin:/sbin:/sbin:/bin:/usr/bin:${SYSPREFIX}/usr/sbin:${SYSPREFIX}/sbin:${SYSPREFIX}/usr/bin:${SYSPREFIX}/bin:$PATH HOME=/ LOGNAME=root \
		fakeroot \
		${SYSPREFIX}/usr/bin/apt-get \
		-o Dir::Etc::SourceList="${SYSPREFIX}/etc/apt/sources.list" \
		-o Dir::Etc::Parts="${SYSPREFIX}/etc/apt/apt.conf.d/" \
		-o Dir::State::Lists="${SYSPREFIX}/var/lib/apt/lists/" \
		-o Dir::Cache::Archives="${SYSPREFIX}/var/cache/apt/archives/" \
		-o Dir::Etc::Preferences="${SYSPREFIX}/etc/apt/preferences" \
		 --print-uris --yes install m4
}

mkprefix()
{
	#[ -e $WORKSPACE/linux$ARCH$SUFFIX/tools/bin/fakeroot ] || mkfakeroot
	#[ -e $WORKSPACE/linux$ARCH$SUFFIX/tools/bin/fakechroot ] || mkfakechroot
	
	DEB_LIST_MINIMAL="\
			build-essential \
			libpng12-dev \
			libjpeg62-dev \
			libfreetype6-dev \
			libxml2-dev \
			libtiff4-dev \
			libjasper-dev \
			libffi-dev \
			libasound2-dev \
			x11proto-xext-dev libdirectfb-dev libxfixes-dev libxinerama-dev libxdamage-dev libxcomposite-dev libxcursor-dev libxft-dev libxrender-dev libxt-dev libxrandr-dev libxi-dev libxext-dev libx11-dev \
			libxtst-dev \
			libpthread-stubs0-dev \
			libxml-parser-perl \
			libdb-dev uuid-dev \
			libdbus-1-dev \
			wget mawk \
			bzip2"
	
	INCLUDE_LIST=""
	for deb in $DEB_LIST_MINIMAL; do
		INCLUDE_LIST="$deb,$INCLUDE_LIST"
	done
	
	[ ! -e ${SYSPREFIX}/dev ] || rm -rf ${SYSPREFIX}/dev
	[ ! -e ${SYSPREFIX}/proc ] || rm -rf ${SYSPREFIX}/proc
	# --foreign --variant=fakechroot
	LD_LIBRARY_PATH=${UBUNTU_LIBDIR}:/${LIBDIR}:${SYSPREFIX}/usr/${LIBDIR} \
	PATH=/usr/local/sbin:/usr/sbin:/sbin:/sbin:/bin:/usr/bin:${SYSPREFIX}/usr/sbin:${SYSPREFIX}/sbin:${SYSPREFIX}/usr/bin:${SYSPREFIX}/bin:$PATH HOME=/ LOGNAME=root \
		fakeroot fakechroot \
		debootstrap --variant=fakechroot --download-only --keep-debootstrap-dir --arch=$SYS_ARCH \
		--include=$INCLUDE_LIST \
		squeeze ${SYSPREFIX} http://ftp.ru.debian.org/debian #http://archive.debian.org/debian

	#LD_LIBRARY_PATH=${UBUNTU_LIBDIR}:/${LIBDIR}:${SYSPREFIX}/usr/${LIBDIR} PATH=/usr/local/sbin:/usr/sbin:/sbin:/sbin:/bin:/usr/bin:${SYSPREFIX}/usr/sbin:${SYSPREFIX}/sbin:${SYSPREFIX}/usr/bin:${SYSPREFIX}/bin:$PATH HOME=/ LOGNAME=root fakeroot fakechroot debootstrap --variant=fakechroot --arch=$SYS_ARCH --foreign --keep-debootstrap-dir --include=sudo --include=apt lenny ${SYSPREFIX} http://archive.debian.org/debian
	
	#LD_LIBRARY_PATH=${UBUNTU_LIBDIR}:/${LIBDIR}:${SYSPREFIX}/usr/${LIBDIR} PATH=/usr/local/sbin:/usr/sbin:/sbin:/sbin:/bin:/usr/bin:${SYSPREFIX}/usr/sbin:${SYSPREFIX}/sbin:${SYSPREFIX}/usr/bin:${SYSPREFIX}/bin:$PATH fakeroot fakechroot linux32 chroot ${SYSPREFIX} #${SYSPREFIX}/debootstrap/debootstrap --second-stage

	for file in `ls -1 ${SYSPREFIX}/var/cache/apt/archives/*.deb`; do
		echo $file
		dpkg -x $file ${SYSPREFIX}
	done
	
	echo "Synfig Buildroot v${BUILDROOT_VERSION}" > ${SYSPREFIX}/etc/chroot.id
}

mkprep()
{

MISSING_PKGS=""
for PKG in debootstrap dpkg fakeroot fakechroot rpmbuild alien git; do
	if ! ( which $PKG > /dev/null ) ; then
		MISSING_PKGS="$MISSING_PKGS $PKG"
	fi
done

for PKG in dpkg-dev; do
	if ! ( rpm -qv $PKG > /dev/null ); then
		MISSING_PKGS="$MISSING_PKGS $PKG"
	fi
done

if [ ! -z "$MISSING_PKGS" ]; then
	echo "ERROR: Please install following packages:"
	echo "         $MISSING_PKGS"
	exit 1
fi

if [ ! -e ${SYSPREFIX}/etc/chroot.id ]; then
	mkprefix
elif [[ `cat ${SYSPREFIX}/etc/chroot.id` != "Synfig Buildroot v${BUILDROOT_VERSION}" ]]; then
	#rm -rf ${SYSPREFIX} || true
	mkprefix
fi

#[ ! -e ${SYSPREFIX}/lib-native ] || rm -rf ${SYSPREFIX}/lib-native
#mkdir -p ${SYSPREFIX}/lib-native
#for file in libc.so.6 libpthread.so.0 ; do
#	cp -L /${LIBDIR}/${file} ${SYSPREFIX}/lib-native
#done

ln -sf ${SYSPREFIX}/usr/bin/mawk ${SYSPREFIX}/usr/bin/awk
	
# Patching libraries ...
for lib in libc libpthread; do
	sed -i "s| /lib/| ${SYSPREFIX}/lib/|g" ${SYSPREFIX}/usr/lib/$lib.so
	sed -i "s| /usr/lib/| ${SYSPREFIX}/usr/lib/|g" ${SYSPREFIX}/usr/lib/$lib.so
done
for file in `find ${SYSPREFIX}/usr/lib/pkgconfig/ -type f -name "*.pc"`; do
	sed -i "s|prefix=/usr|prefix=${SYSPREFIX}/usr|g" ${file}
done
for file in `find ${SYSPREFIX}/usr/bin/ -type f -name "*-config"`; do
	sed -i "s|prefix=/usr|prefix=${SYSPREFIX}/usr|g" ${file}
done
for file in `find ${SYSPREFIX}/usr/lib/ -type f -name "*.la"`; do
	sed -i "s|libdir='/usr/lib'|libdir='${SYSPREFIX}/usr/lib'|g" ${file}
	sed -i "s| /usr/lib| ${SYSPREFIX}/usr/lib|g" ${file}
done

# Fixing symlinks
if [[ $ARCH == 64 ]]; then
	rm ${SYSPREFIX}/lib64
	rm ${SYSPREFIX}/usr/lib64
	ln -sf ${SYSPREFIX}/lib ${SYSPREFIX}/lib64
	ln -sf ${SYSPREFIX}/usr/lib ${SYSPREFIX}/usr/lib64
fi
# For some reason we have to specify ${SYSPREFIX}/usr/lib explicitly
for CHECKPATH in ${SYSPREFIX} ${SYSPREFIX}/usr/lib ${SYSPREFIX}/bin; do
	for LINK in `find -L ${CHECKPATH} -xtype l`; do
		LINK_TARGET=`readlink "$LINK"`
		if [[ ${LINK_TARGET::1} == "/" ]]; then
			if [[ "${LINK_TARGET:0:${#SYSPREFIX}}" != "$SYSPREFIX" ]]; then
				echo "Fixing link: $LINK ($LINK_TARGET)"
				ln -sf "${SYSPREFIX}/${LINK_TARGET}" "${LINK}" || true
			fi
		fi			
	done
done

ln -sf ${SYSPREFIX}/usr/bin/gcc ${SYSPREFIX}/usr/bin/cc

[ -e "${PREFIX}/lib" ] || mkdir -p ${PREFIX}/lib
#cp ${SYSPREFIX}/usr/lib/libltdl* ${PREFIX}/lib/
cp ${SYSPREFIX}/usr/lib/libpng12* ${PREFIX}/lib/
cp ${SYSPREFIX}/usr/lib/libdb-4*.so ${PREFIX}/lib/
cp ${SYSPREFIX}/lib/libpcre.so* ${PREFIX}/lib/
cp ${SYSPREFIX}/usr/lib/libffi*.so* ${PREFIX}/lib
# SDL deps
cp ${SYSPREFIX}/usr/lib/libdirect-*.so* ${PREFIX}/lib/
cp ${SYSPREFIX}/usr/lib/libdirectfb-*.so* ${PREFIX}/lib/
cp ${SYSPREFIX}/usr/lib/libfusion*.so* ${PREFIX}/lib/

#RANDOM_SYSPREFIX=`tr -cd '[:alnum:]' < /dev/urandom | fold -w8 | head -n1`
#DATE=`date +%s`
#LIB_PATH=${SYSPREFIX}/tmp/lib-\${DATE}-\${RANDOM_SYSPREFIX}
#mkdir -p \${LIB_PATH} || true
#for lib in libc.so.6 libpthread.so.0; do
#	cp -L ${SYSPREFIX}/${LIBDIR}/\${lib} \${LIB_PATH}
#done

[ -e ${DEPSPREFIX}/bin ] || mkdir -p ${DEPSPREFIX}/bin

cat > ${DEPSPREFIX}/bin/gcc-- <<EOF
#!/bin/sh

${SYSPREFIX}/usr/bin/gcc -nostdinc -I${SYSPREFIX}/usr/lib/gcc/${GCC_ARCH}-linux-gnu/4.3.2/include -I${SYSPREFIX}/usr/lib/gcc/${GCC_ARCH}-linux-gnu/4.3.2/include-fixed -I${PREFIX}/include  -I${DEPSPREFIX}/include -I${SYSPREFIX}/usr/include  "\$@"
EOF
#chmod a+x  ${DEPSPREFIX}/bin/gcc

cat > ${DEPSPREFIX}/bin/g++-- <<EOF
#!/bin/sh

${SYSPREFIX}/usr/bin/g++ -nostdinc   -I${SYSPREFIX}/usr/lib/gcc/../../include/c++/4.3  -I${SYSPREFIX}/usr/lib/gcc/../../include/c++/4.3/${GCC_ARCH}-linux-gnu -I${SYSPREFIX}/usr/lib/gcc/../../include/c++/4.3/backward -I${SYSPREFIX}/usr/lib/gcc/${GCC_ARCH}-linux-gnu/4.3.2/include -I${SYSPREFIX}/usr/lib/gcc/${GCC_ARCH}-linux-gnu/4.3.2/include-fixed -I${PREFIX}/include  -I${DEPSPREFIX}/include -I${SYSPREFIX}/usr/include "\$@"
EOF
#chmod a+x  ${DEPSPREFIX}/bin/g++

cat > ${DEPSPREFIX}/bin/rsync <<EOF
#!/bin/sh

/usr/bin/rsync "\$@"
EOF
chmod a+x  ${DEPSPREFIX}/bin/rsync

cat > ${DEPSPREFIX}/bin/python <<EOF
#!/bin/sh

/usr/bin/python "\$@"
EOF
chmod a+x  ${DEPSPREFIX}/bin/python

#for binary in bzip2; do
#	ln -sf /usr/bin/$binary  ${DEPSPREFIX}/bin/$binary
#done

}

mkglib()
{
PKG_NAME=glib
PKG_VERSION="${GLIB_VERSION}"
TAREXT=xz
if ! pkg-config ${PKG_NAME}-2.0 --exact-version=${PKG_VERSION}  --print-errors; then
	cd ${CACHEDIR}
    [ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://ftp.gnome.org/pub/gnome/sources/glib/2.38/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --host=${HOST} --disable-static --enable-shared --prefix=${PREFIX}/
	make -j${THREADS}
	make install
	cd ..
fi
}

mkfontconfig()
{
PKG_NAME=fontconfig
PKG_VERSION="${FONTCONFIG_VERSION}"
TAREXT=bz2
if ! pkg-config ${PKG_NAME} --exact-version=${PKG_VERSION}  --print-errors; then
	( cd ${WORKSPACE}/cache/ && wget -c --no-check-certificate http://www.freedesktop.org/software/fontconfig/release/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --host=${HOST} --disable-static --enable-shared --prefix=${DEPSPREFIX}/
	make -j${THREADS}
	make install
	cd ..
	popd
fi
sed -i "s?<cachedir>${DEPSPREFIX}.*</cachedir>??"  ${DEPSPREFIX}/etc/fonts/fonts.conf
}

mkatk()
{
PKG_NAME=atk
PKG_VERSION="${ATK_VERSION}"
TAREXT=xz
if ! pkg-config ${PKG_NAME} --exact-version=${PKG_VERSION}  --print-errors; then
	cd ${CACHEDIR}
    [ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://ftp.gnome.org/pub/gnome/sources/atk/${PKG_VERSION%.*}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --host=${HOST} --disable-static --enable-shared --prefix=${PREFIX}/
	make -j${THREADS}
	make install
	cd ..
	popd
fi
}

mkatkmm()
{
PKG_NAME=atkmm
PKG_VERSION="${ATKMM_VERSION}"
TAREXT=xz
if ! pkg-config ${PKG_NAME} --exact-version=${PKG_VERSION}  --print-errors; then
	cd ${CACHEDIR}
    [ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://ftp.gnome.org/pub/gnome/sources/${PKG_NAME}/${PKG_VERSION%.*}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --host=${HOST} --disable-static --enable-shared --prefix=${PREFIX}/
	make -j${THREADS}
	make install
	cd ..
	popd
fi
}

mkatspi2()
{
PKG_NAME=at-spi2-core
PKG_VERSION="${AT_SPI2_VERSION}"
TAREXT=xz
if ! pkg-config at-spi2 --exact-version=${PKG_VERSION}  --print-errors; then
	( cd ${WORKSPACE}/cache/ && wget -c --no-check-certificate http://ftp.gnome.org/pub/gnome/sources/${PKG_NAME}/${PKG_VERSION%.*}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	[ ! -e config.cache ] || rm config.cache
	./configure --build=${HOST} --prefix=${PREFIX}/ \
		--disable-static --enable-shared
	make -j${THREADS}
	make install
	cd ..
	popd
fi
}

mkatspi2atk()
{
mkatspi2
PKG_NAME=at-spi2-atk
PKG_VERSION="${AT_SPI2_ATK_VERSION}"
TAREXT=xz	
if ! pkg-config atk-bridge-2.0 --exact-version=${PKG_VERSION}  --print-errors; then
	( cd ${WORKSPACE}/cache/ && wget -c --no-check-certificate http://ftp.gnome.org/pub/gnome/sources/${PKG_NAME}/${PKG_VERSION%.*}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	[ ! -e config.cache ] || rm config.cache
	./configure --build=${HOST} --prefix=${PREFIX}/ \
		--disable-static --enable-shared
	make -j${THREADS}
	make install
	cd ..
	popd
fi
}





mkpixman()
{
PKG_NAME=pixman
PKG_VERSION="${PIXMAN_VERSION}"
TAREXT=gz
if ! pkg-config ${PKG_NAME}-1 --exact-version=${PKG_VERSION}  --print-errors; then
	( cd ${WORKSPACE}/cache/ && wget -c --no-check-certificate http://cairographics.org/releases/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xzf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --host=${HOST} --disable-static --enable-shared --prefix=${PREFIX}/
	make -j${THREADS}
	make install
	cd ..
	popd
fi
}

mkharfbuzz()
{
PKG_NAME=harfbuzz
PKG_VERSION="${HARFBUZZ_VERSION}"
TAREXT=bz2
if ! pkg-config ${PKG_NAME} --exact-version=${PKG_VERSION}  --print-errors; then
	( cd ${WORKSPACE}/cache/ && wget -c --no-check-certificate http://www.freedesktop.org/software/harfbuzz/release/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --host=${HOST} --prefix=${PREFIX}
	make -j${THREADS}
	make install
	cd ..
	popd
fi
}

mkcairo()
{
PKG_NAME=cairo
PKG_VERSION="${CAIRO_VERSION}"
TAREXT=xz
if ! pkg-config ${PKG_NAME} --exact-version=${PKG_VERSION}  --print-errors; then
	( cd ${WORKSPACE}/cache/ && wget -c --no-check-certificate http://cairographics.org/releases/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --host=${HOST} --prefix=${PREFIX} \
		--disable-static 	\
		--enable-warnings 	\
		--enable-xlib 		\
		--enable-freetype 	\
		--enable-pdf		\
	    --enable-gobject    \
		--disable-gtk-doc
	make -j${THREADS}
	make install
	cd ..
	popd
fi
}

mkgdkpixbuf()
{
PKG_NAME=gdk-pixbuf
PKG_VERSION="${GDK_PIXBUF_VERSION}"
TAREXT=xz
if ! pkg-config ${PKG_NAME} --exact-version=${PKG_VERSION}  --print-errors; then
	( cd ${WORKSPACE}/cache/ && wget -c --no-check-certificate http://ftp.gnome.org/pub/gnome/sources/gdk-pixbuf/${PKG_VERSION%.*}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	[ ! -e config.cache ] || rm config.cache
	./configure --host=${HOST} --prefix=${PREFIX}/ \
		--disable-static --enable-shared
	make -j${THREADS}
	make install
	cd ..
	popd
fi
}

mkpango()
{
PKG_NAME=pango
PKG_VERSION="${PANGO_VERSION}"
TAREXT=xz
if ! pkg-config ${PKG_NAME} --exact-version=${PKG_VERSION}  --print-errors; then
	( cd ${WORKSPACE}/cache/ && wget -c --no-check-certificate http://ftp.gnome.org/pub/gnome/sources/pango/${PKG_VERSION%.*}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	[ ! -e config.cache ] || rm config.cache
	./configure --host=${HOST} --prefix=${PREFIX}/ \
		--disable-static --enable-shared \
		--with-included-modules=yes
	make -j${THREADS}
	make install
	cd ..
	popd
fi
}

mkgtk()
{
PKG_NAME=gtk\+
PKG_VERSION="${GTK_VERSION}"
TAREXT=xz
if ! pkg-config ${PKG_NAME}-3.0 --exact-version=${PKG_VERSION}  --print-errors; then
	( cd ${WORKSPACE}/cache/ && wget -c --no-check-certificate http://ftp.gnome.org/pub/gnome/sources/gtk+/${PKG_VERSION%.*}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	[ ! -e config.cache ] || rm config.cache
	./configure --build=${HOST} --prefix=${PREFIX}/ \
		--disable-static --enable-shared
	make -j${THREADS}
	make install
	cd ..
	popd
fi
}

mkgtkengines()
{
PKG_NAME=gtk-engines
PKG_VERSION="${GTKENGINES_VERSION}"
TAREXT=bz2
if ! pkg-config ${PKG_NAME}-2.0 --exact-version=${PKG_VERSION}  --print-errors; then
	#rsync -av ${SOURCES_URL}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	( cd ${WORKSPACE}/cache/ && wget -c --no-check-certificate http://ftp.gnome.org/pub/gnome/sources/gtk-engines/${PKG_VERSION%.*}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xjf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --host=${HOST} --prefix=${DEPSPREFIX}/ \
		--disable-static --enable-shared
	make -j${THREADS}
	make install
	cd ..
	popd
fi
}

mklibsigcpp()
{
PKG_NAME=libsigc++
PKG_VERSION="${LIBSIGCPP_VERSION}"
TAREXT=bz2
if ! pkg-config sigc++-2.0 --exact-version=${PKG_VERSION}  --print-errors; then
	#rsync -av ${SOURCES_URL}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	#rsync -av ${SOURCES_URL}/libsigc++-2.0_2.0.18-2.diff ${WORKSPACE}/cache/libsigc++-2.0_2.0.18-2.diff
	( cd ${WORKSPACE}/cache/ && wget -c --no-check-certificate http://ftp.gnome.org/pub/GNOME/sources/libsigc++/${PKG_VERSION%.*}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xjf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} # && cd ${PKG_NAME}-${PKG_VERSION} && patch -p1 < ${WORKSPACE}/cache/libsigc++-2.0_2.0.18-2.diff && cd ..
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --host=${HOST} --prefix=${PREFIX} --includedir=${PREFIX}/include \
		--disable-static --enable-shared
	make -j${THREADS}
	make install
	cd ..
	popd
fi
}

mkglibmm()
{
PKG_NAME=glibmm
PKG_VERSION="${GLIBMM_VERSION}"
TAREXT=xz
if ! pkg-config ${PKG_NAME}-2.4 --exact-version=${PKG_VERSION}  --print-errors; then
	( cd ${WORKSPACE}/cache/ && wget -c --no-check-certificate http://ftp.gnome.org/pub/GNOME/sources/${PKG_NAME}/${PKG_VERSION%.*}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --host=${HOST} --prefix=${PREFIX} --includedir=${PREFIX}/include \
		--disable-fulldocs \
		--disable-static --enable-shared
	make -j${THREADS}
	make install
	cd ..
	popd
fi
}

mklibxmlpp()
{
PKG_NAME=libxml++
PKG_VERSION="${LIBXMLPP_VERSION}"
TAREXT=bz2
if ! pkg-config libxml++-2.6 --exact-version=${PKG_VERSION}  --print-errors; then
	rsync -av ${SOURCES_URL}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xjf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --host=${HOST} --prefix=${PREFIX} --includedir=${PREFIX}/include \
		--disable-static --enable-shared
	make -j${THREADS}
	make install
	cd ..
	popd
fi
}

mkimagemagick()
{
PKG_NAME=ImageMagick
PKG_VERSION="${IMAGEMAGICK_VERSION}-8"
TAREXT=bz2
if ! pkg-config ${PKG_NAME} --exact-version=${IMAGEMAGICK_VERSION}  --print-errors; then
	( cd ${WORKSPACE}/cache/ && wget -c http://www.imagemagick.org/download/releases/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xjf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --host=${HOST} --prefix=${PREFIX} --includedir=${PREFIX}/include \
		--disable-static --enable-shared \
		--with-modules \
		--without-perl \
		--without-x \
		--with-threads \
		--with-magick_plus_plus
	sed -i 's|^hardcode_libdir_flag_spec=.*|hardcode_libdir_flag_spec=""|g' libtool
	sed -i 's|^runpath_var=LD_RUN_PATH|runpath_var=DIE_RPATH_DIE|g' libtool
	make -j${THREADS}
	make install
	cd ..
	popd
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
    ./configure --host=${HOST} --prefix=${PREFIX} --includedir=${PREFIX}/include \
		--disable-static --enable-shared
    
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
    ./configure --host=${HOST} --prefix=${PREFIX} --includedir=${PREFIX}/include \
		--disable-static --enable-shared

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
    ./configure \
        --host=${HOST} --prefix=${PREFIX} --includedir=${PREFIX}/include \
		--disable-static --enable-shared

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
    ./configure \
        --host=${HOST} --prefix=${PREFIX} --includedir=${PREFIX}/include \
		--disable-static --enable-shared

    make all -j$THREADS
    make install -j$THREADS

fi
}

mklame()
{
PKG_NAME=lame
PKG_VERSION=3.99.5
TAREXT=gz

if [ ! -f ${PREFIX}/../${PKG_NAME}-${PKG_VERSION}.done ]; then
    cd $CACHEDIR
    [ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://sourceforge.net/projects/lame/files/lame/3.99/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    cd $SRCPREFIX
    if [ ! -d ${PKG_NAME}-${PKG_VERSION} ]; then
        tar -xzf $CACHEDIR/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    fi
    cd ${PKG_NAME}-${PKG_VERSION}
    [ ! -e config.cache ] || rm config.cache
    ./configure --host=${HOST} --prefix=${PREFIX} --includedir=${PREFIX}/include \
		--disable-static --enable-shared

    make all -j$THREADS
    make install -j$THREADS
    
    touch ${PREFIX}/../${PKG_NAME}-${PKG_VERSION}.done

fi
}

mklibtheora()
{
	
mklibogg

PKG_NAME=libtheora
PKG_VERSION=1.1.1
TAREXT=gz

if [ ! -f ${PREFIX}/../${PKG_NAME}-${PKG_VERSION}.done ]; then
    cd $CACHEDIR
    [ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://downloads.xiph.org/releases/theora/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    cd $SRCPREFIX
    if [ ! -d ${PKG_NAME}-${PKG_VERSION} ]; then
        tar -xzf $CACHEDIR/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    fi
    cd ${PKG_NAME}-${PKG_VERSION}
    [ ! -e config.cache ] || rm config.cache
    ./configure --host=${HOST} --prefix=${PREFIX} --includedir=${PREFIX}/include \
		--disable-static --enable-shared

    make all -j$THREADS
    make install -j$THREADS
    
    touch ${PREFIX}/../${PKG_NAME}-${PKG_VERSION}.done

fi
}

mkx264()
{
PKG_NAME=x264
PKG_VERSION=3.99.5
TAREXT=gz

if [ ! -f ${PREFIX}/../${PKG_NAME}-${PKG_VERSION}.done ]; then
    if [ -d x264 ]; then
	   cd x264
	   /usr/bin/git pull -v
	   cd ..
	else
	   /usr/bin/git clone git://git.videolan.org/x264.git
	fi
    cd ${PKG_NAME}
    [ ! -e config.cache ] || rm config.cache
    ./configure --host=${HOST} --prefix=${PREFIX} --includedir=${PREFIX}/include \
		--disable-static --enable-shared

    make all -j$THREADS
    make install -j$THREADS
    
    touch ${PREFIX}/../${PKG_NAME}-${PKG_VERSION}.done

fi
}


mkfaac()
{
PKG_NAME=faac
PKG_VERSION=1.28
TAREXT=bz2

if [ ! -f ${PREFIX}/../${PKG_NAME}-${PKG_VERSION}.done ]; then
    cd $CACHEDIR
    [ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://downloads.sourceforge.net/faac/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    cd $SRCPREFIX
    if [ ! -d ${PKG_NAME}-${PKG_VERSION} ]; then
        tar -xjf $CACHEDIR/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    fi
    cd ${PKG_NAME}-${PKG_VERSION}
    [ ! -e config.cache ] || rm config.cache
    ./configure --host=${HOST} --prefix=${PREFIX} --includedir=${PREFIX}/include \
		--disable-static --enable-shared
		
	if [ ! -f common/mp4v2/mpeg4ip.h.patch ]; then
		sed -i '126 s|^|//|' common/mp4v2/mpeg4ip.h
		touch common/mp4v2/mpeg4ip.h.patch
	fi

    make all -j$THREADS
    make install -j$THREADS
    
    sed -i '53 s|^|//|' ${PREFIX}/include/faac.h
    
    touch ${PREFIX}/../${PKG_NAME}-${PKG_VERSION}.done

fi
}

mkyasm()
{
PKG_NAME=yasm
PKG_VERSION=1.2.0
TAREXT=gz
if [ ! -f ${PREFIX}/../${PKG_NAME}-${PKG_VERSION}.done ]; then
	cd $CACHEDIR
    [ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://www.tortall.net/projects/yasm/releases/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    cd $SRCPREFIX
    if [ ! -d ${PKG_NAME}-${PKG_VERSION} ]; then
        tar -xzf $CACHEDIR/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    fi
    cd ${PKG_NAME}-${PKG_VERSION}
    [ ! -e config.cache ] || rm config.cache
    
	./configure --host=${HOST} --disable-static --enable-shared --prefix=${DEPSPREFIX}/
	make -j${THREADS}
	make install
	cd ..
	
	touch ${PREFIX}/../${PKG_NAME}-${PKG_VERSION}.done
fi
}

mkffmpeg()
{

#mkfaac
mkyasm
mklame
mklibtheora
mklibvorbis
mkx264

PKG_NAME=ffmpeg
PKG_VERSION=2.4.x
TAREXT=bz2
if [ ! -f ${PREFIX}/../${PKG_NAME}-${PKG_VERSION}.done ]; then
	
	pushd ${SRCPREFIX}
	
	if [ -d ffmpeg ]; then
	   cd ffmpeg
	   /usr/bin/git pull
	   cd ..
	else
		/usr/bin/git clone git://source.ffmpeg.org/ffmpeg.git ffmpeg
	fi

	#rsync -av ${SOURCES_URL}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	
	#[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xjf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	#cd ${PKG_NAME}-${PKG_VERSION}
	
	cd ${PKG_NAME}
	
	./configure --prefix=${PREFIX} \
		--arch=${SYS_ARCH} \
		--enable-rpath \
		--enable-gpl --enable-nonfree \
		--enable-libx264 --enable-libmp3lame \
		--enable-libtheora --enable-libvorbis \
		--disable-static --enable-shared
		#--enable-libfaac
	make -j${THREADS}
	make install
	cd ..
	popd
	
	touch ${PREFIX}/../${PKG_NAME}-${PKG_VERSION}.done
fi
}

mksdl()
{
	
PKG_NAME=SDL
PKG_VERSION=1.2.15
TAREXT=gz

if [ ! -f ${PREFIX}/../${PKG_NAME}-${PKG_VERSION}.done ]; then

    cd $CACHEDIR
    [ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://www.libsdl.org/release/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    cd $SRCPREFIX
    if [ ! -d ${PKG_NAME}-${PKG_VERSION} ]; then
        tar -xzf $CACHEDIR/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    fi
    cd ${PKG_NAME}-${PKG_VERSION}
    [ ! -e config.cache ] || rm config.cache
    
	./configure --host=${HOST} --disable-static --enable-shared --prefix=${PREFIX}/
	make -j${THREADS}
	make install
	cd ..
	
	touch ${PREFIX}/../${PKG_NAME}-${PKG_VERSION}.done
fi
}

mkmlt()
{
mksdl
mkffmpeg
mklibsamplerate
mklibvorbis
mksox
	
PKG_NAME=mlt
PKG_VERSION=0.9.1
TAREXT=gz

if [ ! -f ${PREFIX}/../${PKG_NAME}-${PKG_VERSION}.done ]; then

    #export CPPFLAGS=" -I/usr/${TOOLCHAIN_HOST}/sys-root/mingw/include/SDL $CPPFLAGS"
    #export LDFLAGS=" $LDFLAGS -lmingw32 -lSDLmain -lSDL -mwindows"
	
    cd $SRCPREFIX
    if [ ! -d ${PKG_NAME} ]; then
        /usr/bin/git clone https://github.com/morevnaproject/mlt
    fi
    cd mlt
    [ ! -e config.cache ] || rm config.cache

    ./configure \
        --host=${HOST} --prefix=${PREFIX} --includedir=${PREFIX}/include \
        --avformat-shared=${PREFIX}/ \
        --target-arch=${RPM_ARCH} \
        --enable-gpl --disable-decklink \
        $DEBUG
        
     touch src/modules/disable-gtk2

    make all -j$THREADS
    make install -j$THREADS
    
    if [ ! -f ${PREFIX}/lib/mlt/libmltsdl.so ]; then
		echo "ERROR: No SDL module compiled for MLT."
		exit 1
    fi
	
	touch ${PREFIX}/../${PKG_NAME}-${PKG_VERSION}.done
fi
}

mkboost()
{
PKG_NAME=boost
PKG_VERSION="${BOOST_VERSION}"
TAREXT=bz2
if ! cat ${PREFIX}/include/boost/version.hpp |egrep "BOOST_LIB_VERSION \"${PKG_VERSION%_*}\""; then
	#PATH_BAK=$PATH
	#PATH="${DEPSPREFIX}/bin-gcc/:$PATH"
	#rm -rf ${DEPSPREFIX}/lib/libboost_program_options* || true
	#rm -rf ${PREFIX}/lib/libboost_program_options* || true
	rsync -av ${SOURCES_URL}/${PKG_NAME}_${PKG_VERSION}.tar.${TAREXT} ${WORKSPACE}/cache/${PKG_NAME}_${PKG_VERSION}.tar.${TAREXT}
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}_${PKG_VERSION} ] && tar -xjf ${WORKSPACE}/cache/${PKG_NAME}_${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}_${PKG_VERSION}
	./bootstrap.sh --prefix=${PREFIX} \
		--libdir=${PREFIX}/lib \
		--exec-prefix=${PREFIX} \
		--with-libraries=program_options
	./b2
	./b2 install || true
	cd ..
	popd
	#PATH="$PATH_BAK"
fi
#cp ${DEPSPREFIX}/lib/libboost_program_options.so.*.0 ${PREFIX}/lib/
}

mkcairomm()
{
PKG_NAME=cairomm
PKG_VERSION="${CAIROMM_VERSION}"
TAREXT=gz
if ! pkg-config ${PKG_NAME}-1.0 --exact-version=${PKG_VERSION}  --print-errors; then
	( cd ${WORKSPACE}/cache/ && wget -c --no-check-certificate http://cairographics.org/releases/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xzf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --host=${HOST} --prefix=${PREFIX} --includedir=${PREFIX}/include \
		--enable-docs=no \
		--disable-static --enable-shared
	make -j${THREADS}
	make install
	cd ..
	popd
fi
}

mkpangomm()
{
PKG_NAME=pangomm
PKG_VERSION="${PANGOMM_VERSION}"
TAREXT=xz
if ! pkg-config ${PKG_NAME}-1.4 --exact-version=${PKG_VERSION}  --print-errors; then
	( cd ${WORKSPACE}/cache/ && wget -c --no-check-certificate http://ftp.gnome.org/pub/GNOME/sources/${PKG_NAME}/${PKG_VERSION%.*}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --host=${HOST} --prefix=${PREFIX} --includedir=${PREFIX}/include \
		--disable-docs \
		--disable-static --enable-shared
	make -j${THREADS}
	make install
	cd ..
	popd
fi
}

mkgtkmm()
{
mkatkmm
PKG_NAME=gtkmm
PKG_VERSION="${GTKMM_VERSION}"
TAREXT=xz
if ! pkg-config ${PKG_NAME}-3.0 --exact-version=${PKG_VERSION}  --print-errors; then
	( cd ${WORKSPACE}/cache/ && wget -c --no-check-certificate http://ftp.gnome.org/pub/GNOME/sources/${PKG_NAME}/${PKG_VERSION%.*}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	#rsync -av ${SOURCES_URL}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --host=${HOST} --prefix=${PREFIX} --includedir=${PREFIX}/include \
		--disable-examples --disable-demos --disable-docs \
		--disable-static --enable-shared
	make -j${THREADS}
	make install
	cd ..
	popd
fi
}

mkjack()
{
PKG_NAME=jack-audio-connection-kit
PKG_VERSION="${JACK_VERSION}"
TAREXT=gz
if ! pkg-config jack --exact-version=${PKG_VERSION}  --print-errors; then
	rsync -av ${SOURCES_URL}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xzf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --host=${HOST} --prefix=${DEPSPREFIX} --includedir=${DEPSPREFIX}/include \
		--libdir=${DEPSPREFIX}/lib \
		--disable-static --enable-shared
	make -j${THREADS}
	make install
	cd ..
	popd
fi

[ -e "${PREFIX}/lib.extra" ] || mkdir -p ${PREFIX}/lib.extra
if [ ${PREFIX} != ${DEPSPREFIX} ]; then
	cp ${DEPSPREFIX}/lib/libjack.so* ${PREFIX}/lib.extra/
fi
}

mkm4()
{
PKG_NAME=m4
PKG_VERSION="1.4.17"
TAREXT=gz

# TODO: DEPSPREFIX -> TOOLSPREFIX ?

if [ ! -e ${DEPSPREFIX}/bin/m4 ]; then
	( cd ${WORKSPACE}/cache/ && wget -c http://ftp.gnu.org/gnu/m4/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xzf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --host=${HOST} --prefix=${DEPSPREFIX}
	make -j${THREADS}
	make install
	cd ..
	popd
fi

}

mkautoconf()
{
PKG_NAME=autoconf
PKG_VERSION="2.69"
TAREXT=gz

# TODO: DEPSPREFIX -> TOOLSPREFIX ?

if [ ! -e ${DEPSPREFIX}/bin/autoconf ]; then
	( cd ${WORKSPACE}/cache/ && wget -c http://ftp.gnu.org/gnu/autoconf/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xzf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --host=${HOST} --prefix=${DEPSPREFIX}
	make -j${THREADS}
	make install
	cd ..
	popd
fi
}

mkautomake()
{
PKG_NAME=automake
PKG_VERSION="1.14"
TAREXT=gz

# TODO: DEPSPREFIX -> TOOLSPREFIX ?

if [ ! -e ${DEPSPREFIX}/bin/automake ]; then
	( cd ${WORKSPACE}/cache/ && wget -c http://ftp.gnu.org/gnu/automake/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xzf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --host=${HOST} --prefix=${DEPSPREFIX}
	make -j${THREADS}
	make install
	cd ..
	popd
fi
}

mklibtool()
{
PKG_NAME=libtool
PKG_VERSION="2.4.2"
TAREXT=gz

mkdir -p $SYSPREFIX/tmp/${PKG_NAME}-bin || true
cat > $SYSPREFIX/tmp/${PKG_NAME}-bin/gcc <<EOF
#!/bin/sh

${SYSPREFIX}/usr/bin/gcc  "\$@"

EOF
chmod a+x  $SYSPREFIX/tmp/${PKG_NAME}-bin/gcc



PATH_BAK=$PATH
PATH="$SYSPREFIX/tmp/${PKG_NAME}-bin/:$PATH"

if [ ! -e ${DEPSPREFIX}/bin/libtoolize ]; then
	( cd ${WORKSPACE}/cache/ && wget -c http://ftpmirror.gnu.org/libtool/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xzf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	IFS=" " ./configure --host=${HOST} --prefix=${DEPSPREFIX} --enable-ltdl-install
	make -j${THREADS}
	make install
	cd ..
	popd
fi

[ -e ${PREFIX}/lib/ ] || mkdir -p ${PREFIX}/lib/
if [[ "${PREFIX}" != "${DEPSPREFIX}" ]]; then
	#rm -rf ${PREFIX}/lib/libltdl* || true
	cp ${DEPSPREFIX}/lib/libltdl.so* ${PREFIX}/lib/
fi

PATH="$PATH_BAK"
}

mkintltool()
{
PKG_NAME=intltool
PKG_VERSION="0.50.2"
TAREXT=gz


if [ ! -e ${DEPSPREFIX}/bin/intltoolize ]; then
	( cd ${WORKSPACE}/cache/ && wget -c --no-check-certificate https://launchpad.net/intltool/trunk/0.50.2/+download/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xzf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --host=${HOST} --prefix=${DEPSPREFIX}
	make -j${THREADS}
	make install
	cd ..
	popd
fi

}

mkgettext()
{

PKG_NAME=gettext
PKG_VERSION="0.17"
TAREXT=gz

mkdir -p $SYSPREFIX/tmp/gettext-bin || true
cat > $SYSPREFIX/tmp/gettext-bin/gij <<EOF
#!/bin/sh

false
EOF
chmod a+x  $SYSPREFIX/tmp/gettext-bin/gij

cat > $SYSPREFIX/tmp/gettext-bin/gcc <<EOF
#!/bin/sh

${SYSPREFIX}/usr/bin/gcc "\$@"

EOF
chmod a+x  $SYSPREFIX/tmp/gettext-bin/gcc

#rm -rf $SYSPREFIX/tmp/gettext-bin

PATH_BAK=$PATH
PATH="$SYSPREFIX/tmp/gettext-bin/:$PATH"


if [ ! -e ${DEPSPREFIX}/bin/gettext ]; then
	( cd ${WORKSPACE}/cache/ && wget -c http://ftp.gnu.org/pub/gnu/gettext/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xzf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	# cd ${PKG_NAME}-${PKG_VERSION} && patch -p1 < ${WORKSPACE}/cache/gettext-${PKG_VERSION}-4.patch && cd ..
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --host=${HOST} --prefix=${DEPSPREFIX} \
		--disable-java --disable-native-java
	make -j${THREADS}
	make install
	cd ..
	popd
fi

PATH="$PATH_BAK"
}


# Not used
mkfakeroot()
{
	run_native mkfakeroot_
}

# Not used
mkfakeroot_()
{
PKG_NAME=fakeroot
PKG_VERSION="1.18.4"
TAREXT=bz2
if [ ! -e ${TOOLSPREFIX}/bin/fakeroot ]; then
	( cd ${WORKSPACE}/cache/ && wget -c http://ftp.de.debian.org/debian/pool/main/f/${PKG_NAME}/${PKG_NAME}_${PKG_VERSION}.orig.tar.${TAREXT} )
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xjf ${WORKSPACE}/cache/${PKG_NAME}_${PKG_VERSION}.orig.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --host=${HOST} --prefix=${TOOLSPREFIX}/
	make -j${THREADS}
	make install
	cd ..
	popd
fi
}

# Not used
mkfakechroot()
{
	run_native mkfakechroot_
}

# Not used
mkfakechroot_()
{
PKG_NAME=fakechroot
PKG_VERSION="2.16"
TAREXT=gz
if [ ! -e ${DEPSPREFIX}/bin/automake ]; then
	( cd ${WORKSPACE}/cache/ && wget -c http://ftp.de.debian.org/debian/pool/main/f/${PKG_NAME}/${PKG_NAME}_${PKG_VERSION}.orig.tar.${TAREXT} )
	pushd ${SRCPREFIX}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xzf ${WORKSPACE}/cache/${PKG_NAME}_${PKG_VERSION}.orig.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --host=${HOST} --prefix=$WORKSPACE/linux$ARCH$SUFFIX/tools/
	make -j${THREADS}
	make install
	cd ..
	popd
fi
}

#ETL
mketl()
{
cd $SCRIPTPATH/../ETL
make clean || true
run_native autoreconf --install --force
./configure --host=${HOST} --prefix=${PREFIX} \
	--includedir=${PREFIX}/include --libdir=${PREFIX}/lib \
	--bindir=${PREFIX}/bin \
	$DEBUG_OPT
make -j${THREADS}
make install
}

#synfig-core
mksynfig()
{
cd $SCRIPTPATH/../synfig-core/
make clean || true
[ ! -e config.cache ] || rm config.cache
echo "Running libtoolize ..."
libtoolize --ltdl --copy --force
echo "Running autoreconf ..."
autoreconf --install --force
./configure --host=${HOST} --prefix=${PREFIX} \
	--includedir=${PREFIX}/include \
	--libdir=${PREFIX}/lib --bindir=${PREFIX}/bin \
	--sysconfdir=${PREFIX}/etc \
	--disable-static --enable-shared \
	--with-magickpp --without-libavcodec \
	--with-boost=${PREFIX}/ \
	--enable-warnings=minimum \
	$DEBUG_OPT
make -j${THREADS}
make install
}

#synfig-studio
mksynfigstudio()
{
cd $SCRIPTPATH/../synfig-studio/
make clean || true
[ ! -e config.cache ] || rm config.cache
/bin/sh ./bootstrap.sh
./configure --host=${HOST} --prefix=${PREFIX} \
	--includedir=${SYSPREFIX}/include \
	--libdir=${PREFIX}/lib --bindir=${PREFIX}/bin \
	--sysconfdir=${PREFIX}/etc --datadir=${PREFIX}/share  \
	--disable-static --enable-shared \
	$DEBUG_OPT
make -j${THREADS}
make install

}

mkconfig()
{
	
if [ ${PREFIX} == ${DEPSPREFIX} ]; then
	#if [ ! -e "${PREFIX}/etc/pango/pango.modules.in" ]; then
	#	sed "s?${PREFIX}/lib/pango/1.6.0/modules?@ROOTDIR@/modules?" < ${PREFIX}/etc/pango/pango.modules > ${PREFIX}/etc/pango/pango.modules.in
	#fi


	if [ ! -e "${PREFIX}/etc/gtk-2.0/gdk-pixbuf.loaders.in" ]; then
		sed "s?${PREFIX}/lib/gtk-2.0/2.10.0/loaders?@ROOTDIR@/loaders?" < ${PREFIX}/etc/gtk-2.0/gdk-pixbuf.loaders > ${PREFIX}/etc/gtk-2.0/gdk-pixbuf.loaders.in
	fi
fi

cat > ${PREFIX}/synfig <<EOF
#!/bin/sh

SYSPREFIX=\`dirname "\$0"\`

export LD_LIBRARY_PATH=\${SYSPREFIX}/lib:\$LD_LIBRARY_PATH
export SYNFIG_ROOT=\${SYSPREFIX}/
export SYNFIG_MODULE_LIST=\${SYSPREFIX}/etc/synfig_modules.cfg
export MLT_DATA="\${SYSPREFIX}/share/mlt/"
export MLT_REPOSITORY="\${SYSPREFIX}/lib/mlt/"

\$SYSPREFIX/bin/synfig "\$@"
EOF
	chmod a+x $PREFIX/synfig

cat > ${PREFIX}/synfigstudio <<EOF
#!/bin/sh

SYSPREFIX=\`dirname "\$0"\`

# Check if this system have JACK installed
if ( ! ldconfig -p | grep libjack.so >/dev/null ) || ( ! which jackd >/dev/null ) ; then
	# No JACK, so disable this functionality.
	# (The bundled libjack won't work correctly anyway).
	export SYNFIG_DISABLE_JACK=1
	export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:\${SYSPREFIX}/lib.extra
fi

USER_CONFIG_DIR=\$HOME/.config/synfig
export ETC_DIR=\${SYSPREFIX}/etc
export LD_LIBRARY_PATH=\${SYSPREFIX}/lib:\$LD_LIBRARY_PATH
export SYNFIG_ROOT=\${SYSPREFIX}/
export SYNFIG_MODULE_LIST=\${SYSPREFIX}/etc/synfig_modules.cfg
#export GDK_PIXBUF_MODULEDIR="\${SYSPREFIX}/lib/gtk-2.0/2.10.0/loaders"
export FONTCONFIG_PATH="\${SYSPREFIX}/etc/fonts"
export MLT_DATA="\${SYSPREFIX}/share/mlt/"
export MLT_REPOSITORY="\${SYSPREFIX}/lib/mlt/"

# Create install-location-dependent config files for Pango and GDK image loaders
# We have to do this every time because its possible that SYSPREFIX has changed

[ -e "\$USER_CONFIG_DIR" ] || mkdir -p "\$USER_CONFIG_DIR"

#sed "s?@ROOTDIR@/modules?\${SYSPREFIX}/lib/pango/1.6.0/modules?" < \$ETC_DIR/pango/pango.modules.in > \$USER_CONFIG_DIR/pango/pango.modules
if [ -e \$ETC_DIR/gtk-2.0/gdk-pixbuf.loaders.in ]; then
	sed "s?@ROOTDIR@/loaders?\${SYSPREFIX}/lib/gtk-2.0/2.10.0/loaders?" < \$ETC_DIR/gtk-2.0/gdk-pixbuf.loaders.in > \$GDK_PIXBUF_MODULE_FILE
	export GDK_PIXBUF_MODULE_FILE="\${USER_CONFIG_DIR}/gdk-pixbuf.loaders"
fi

\${SYSPREFIX}/bin/synfigstudio "\$@"

exit 0

# 1 check if test application starts without warnings
LANG=C GTK_PATH=/usr/lib64/gtk-2.0/2.10.0/ /home/zelgadis/synfig-buildroot/linux64/bundle/bin/gtk-test --g-fatal-warnings | egrep "Gtk+ version too old (micro mismatch)"

# If everything is fine then start with GTK_PATH

GTK_PATH=/usr/lib64/gtk-2.0/2.10.0/ /home/zelgadis/synfig-buildroot/linux64/bundle/bin/synfigstudio

# otherwise start with custom GTKRC

GTK2_RC_FILES=/home/zelgadis/synfig-buildroot/linux64/bundle/gtkrc:$GTK2_RC_FILES /home/zelgadis/synfig-buildroot/linux64/bundle/bin/synfigstudio

EOF

chmod +x ${PREFIX}/synfigstudio

}

mkpreloader()
{

mkdir -p ${SRCPREFIX}/preloader
pushd ${SRCPREFIX}/preloader >/dev/null
cat > synfig-pkg-preloader.c  <<EOF
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

static int (*fopen_orig)(const char * pathname, int flags) = NULL;

int open(const char * pathname, int flags) {
	//printf("Blocking");
	printf("Testing %s read\n", pathname);
    if (fopen_orig == NULL)
        fopen_orig = dlsym(RTLD_NEXT, "open");
    if (strstr(pathname, "/home/zelgadis/synfig-buildroot/linux64/sys") != NULL) {
        printf("Blocking %s read\n", pathname);
        return -1;
    }
    return fopen_orig(pathname, flags);
}

FILE *fopen(const char *path, const char *mode) {
    //printf("In our own fopen, opening %s\n", path);

    FILE *(*original_fopen)(const char*, const char*);
    original_fopen = dlsym(RTLD_NEXT, "fopen");
    return (*original_fopen)(path, mode);
}
EOF
gcc -shared -fPIC -ldl -O2 -o synfig-pkg-preloader.so synfig-pkg-preloader.c
cp synfig-pkg-preloader.so ${PREFIX}/lib
}

mkpackage()
{
	[ ! -e ${DISTPREFIX} ] || rm -rf ${DISTPREFIX}
	mkdir -p ${DISTPREFIX}
	cp -r  ${PREFIX}/etc ${DISTPREFIX}
	cp -r  ${PREFIX}/lib ${DISTPREFIX}
	cp -r  ${PREFIX}/lib.extra ${DISTPREFIX}
	cp -r  ${PREFIX}/share ${DISTPREFIX}
	
	cp -r  ${PREFIX}/synfig ${DISTPREFIX}
	cp -r  ${PREFIX}/synfigstudio ${DISTPREFIX}
	
	mkdir -p ${DISTPREFIX}/bin
	BINARIES="\
		synfig
		synfigstudio"
	for FILE in $BINARIES; do
		cp -r  ${PREFIX}/bin/${FILE} ${DISTPREFIX}/bin/
	done
	
	#cleaning devel stuff
	rm -f ${DISTPREFIX}/lib/*.la
	rm -f ${DISTPREFIX}/lib/*.a
	rm -f ${DISTPREFIX}/lib/cairo/*.la
	rm -rf ${DISTPREFIX}/include
	rm -rf ${DISTPREFIX}/lib/gdkmm-2.4
	rm -rf ${DISTPREFIX}/lib/libxml++-2.6
	rm -rf ${DISTPREFIX}/lib/giomm-2.4
	rm -rf ${DISTPREFIX}/lib/glibmm-2.4
	rm -rf ${DISTPREFIX}/lib/pangomm-1.4
	rm -rf ${DISTPREFIX}/lib/gtkmm-2.4
	rm -rf ${DISTPREFIX}/lib/pkgconfig
	rm -rf ${DISTPREFIX}/lib/sigc++-2.0
	rm -rf ${DISTPREFIX}/share/doc
	rm -rf ${DISTPREFIX}/share/devhelp
	rm -rf ${DISTPREFIX}/share/gtk-doc
	rm -rf ${DISTPREFIX}/share/gtkmm-2.4
	rm -rf ${DISTPREFIX}/share/aclocal
	rm -rf ${DISTPREFIX}/share/ImageMagick-6.4.0
	rm -rf ${DISTPREFIX}/share/man
	
	mkpackage_tar
	mkpackage_rpm
	mkpackage_deb
}
	
mkpackage_tar()
{
	#== tar.bz2 ==
	rm -f ${WORKSPACE}/synfigstudio-${VERSION}-${REVISION}.$BREED.$RELEASE.${RPM_ARCH}.tar.bz2 || true
	pushd ${DISTPREFIX}/..
	[ ! -d synfigstudio-${VERSION}-${REVISION}.$BREED.$RELEASE.${RPM_ARCH} ] || rm -rf synfigstudio-${VERSION}-${REVISION}.$BREED.$RELEASE.${RPM_ARCH}
	cp -rf dist synfigstudio-${VERSION}-${REVISION}.$BREED.$RELEASE.${RPM_ARCH}
	tar cjf ${WORKSPACE}/synfigstudio-${VERSION}-${REVISION}.$BREED.$RELEASE.${RPM_ARCH}.tar.bz2 synfigstudio-${VERSION}-${REVISION}.$BREED.$RELEASE.${RPM_ARCH}
	rm -rf synfigstudio-${VERSION}-${REVISION}.$BREED.$RELEASE.${RPM_ARCH}
	popd
}

mkpackage_rpm()
{
	#== rpm ==
    cat > ${WORKSPACE}/linux$ARCH$SUFFIX/synfigstudio.spec << EOF
%define __spec_install_post /bin/true

Name:           synfigstudio
Version:        ${VERSION}
Release:        ${REVISION}.${BREED}.${RELEASE}
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
It eliminates the need for tweening, preventing the need to hand-draw
each frame. Synfig features spatial and temporal resolution independence
(sharp and smooth at any resolution or framerate), high dynamic range
images, and a flexible plugin system.


%prep


%build

%install
rm -rf \$RPM_BUILD_ROOT
mkdir -p \$RPM_BUILD_ROOT/opt/synfig
cp -r  ${DISTPREFIX}/* \$RPM_BUILD_ROOT/opt/synfig
mkdir -p \$RPM_BUILD_ROOT/usr/share
mv \$RPM_BUILD_ROOT/opt/synfig/share/applications \$RPM_BUILD_ROOT/usr/share
mv \$RPM_BUILD_ROOT/opt/synfig/share/appdata \$RPM_BUILD_ROOT/usr/share
mv \$RPM_BUILD_ROOT/opt/synfig/share/icons \$RPM_BUILD_ROOT/usr/share
mv \$RPM_BUILD_ROOT/opt/synfig/share/mime \$RPM_BUILD_ROOT/usr/share
mv \$RPM_BUILD_ROOT/opt/synfig/share/mime-info \$RPM_BUILD_ROOT/usr/share
mkdir -p \$RPM_BUILD_ROOT/usr/share/pixmaps
ln -sf /opt/synfig/share/pixmaps/sif_icon.png \$RPM_BUILD_ROOT/usr/share/pixmaps/sif_icon.png
ln -sf /opt/synfig/share/pixmaps/synfig_icon.png \$RPM_BUILD_ROOT/usr/share/pixmaps/synfig_icon.png
mkdir -p \$RPM_BUILD_ROOT/usr/bin
cp \$RPM_BUILD_ROOT/opt/synfig/synfig \$RPM_BUILD_ROOT/usr/bin/
cp \$RPM_BUILD_ROOT/opt/synfig/synfigstudio \$RPM_BUILD_ROOT/usr/bin/
sed -i 's|^SYSPREFIX=.*|SYSPREFIX=/opt/synfig|' \$RPM_BUILD_ROOT/usr/bin/synfig
sed -i 's|^SYSPREFIX=.*|SYSPREFIX=/opt/synfig|' \$RPM_BUILD_ROOT/usr/bin/synfigstudio


%clean
rm -rf \$RPM_BUILD_ROOT

%post
if [ -x /usr/bin/update-mime-database ]; then
  update-mime-database /usr/share/mime
fi
if [ -x /usr/bin/update-desktop-database ]; then
  update-desktop-database
fi

%postun
if [ -x /usr/bin/update-mime-database ]; then
  update-mime-database /usr/share/mime
fi
if [ -x /usr/bin/update-desktop-database ]; then
  update-desktop-database
fi

%files
%defattr(-,root,root,-)
/opt/synfig/
/usr/share/*
/usr/bin/*

%changelog
* Sat Mar 21 2009 Konstantin Dmitriev <ksee.zelgadis@gmail.com> - 0.61.09-2354.morevnapackage.1
- Update to SVN2354
- Include ImageMagick-c++

* Wed Jan 14 2009 Konstantin Dmitriev <ksee.zelgadis@gmail.com> - 0.61.09-2316.morevnapackage.1
- First release

EOF
    run_native linux${ARCH} rpmbuild -bb --target ${RPM_ARCH} ${WORKSPACE}/linux$ARCH$SUFFIX/synfigstudio.spec
    
    cp $HOME/rpmbuild/RPMS/${RPM_ARCH}/synfigstudio-${VERSION}-${REVISION}.${BREED}.$RELEASE.${RPM_ARCH}.rpm ${WORKSPACE}
}

mkpackage_deb()
{    
    cd ${WORKSPACE}
    #if [[ $ARCH == '64' ]]; then
	#	run_native fakeroot alien -k --scripts synfigstudio-${VERSION}-${REVISION}.${BREED}.$RELEASE.${RPM_ARCH}.rpm
    #else
		rm -rf synfigstudio-${VERSION} || true
		rm -rf synfigstudio_${VERSION}-${REVISION}.${BREED}.${RELEASE}_${RPM_ARCH}.deb || true
		run_native fakeroot alien -g -k --scripts synfigstudio-${VERSION}-${REVISION}.${BREED}.$RELEASE.${RPM_ARCH}.rpm
		
		# Allow to build i386 on x86_64 host
		run_native sed -i "s|Architecture: .*|Architecture: any|g" synfigstudio-${VERSION}/debian/control
		# Workaround permissions problem
		echo >> synfigstudio-${VERSION}/debian/postinst
		echo "chmod a+rX -R /opt/synfig" >> synfigstudio-${VERSION}/debian/postinst
		echo "chmod a+rX /opt" >> synfigstudio-${VERSION}/debian/postinst
		
		pushd synfigstudio-${VERSION} >/dev/null
		run_native dpkg-buildpackage -rfakeroot -a${SYS_ARCH} -d || true
		if [ ! -e ../synfigstudio_${VERSION}-${REVISION}.${BREED}.${RELEASE}_${SYS_ARCH}.deb ]; then
			echo "Failed to generate deb package"
			exit 1
		fi
		popd >/dev/null
		rm -rf synfigstudio-${VERSION}.orig
		rm -rf synfigstudio_${VERSION}.orig.tar.gz
		rm -rf synfigstudio_${VERSION}-${REVISION}.${BREED}.${RELEASE}_${SYS_ARCH}.changes
		rm -rf synfigstudio_${VERSION}-${REVISION}.${BREED}.${RELEASE}.diff.gz
		rm -rf synfigstudio_${VERSION}-${REVISION}.${BREED}.${RELEASE}.dsc
    #fi
    rm -rf synfigstudio-${VERSION}
}

mkall()
{
	mkprep
	
	set_environment
	
	# build tools
	mkm4
	mkautoconf
	mkautomake
	mklibtool
	mkintltool
	mkgettext
	
	# system libraries
	mkglib
	mkharfbuzz
	mkfontconfig
	mkatk
	mkatspi2atk
	mkpixman
	mkcairo # bundled library
	mkpango
	mkgdkpixbuf
	mkgtk
	mkjack
	
	# synfig-core deps
	mklibsigcpp
	mkglibmm
	mklibxmlpp
	#if [[ $OPENGL == 1 ]]; then
	#	mkglew
	#fi
	mkmlt
	mkimagemagick
	mkboost
	
	# synfig-studio deps
	mkcairomm
	mkpangomm
	mkgtkmm
	#if [[ $OPENGL == 1 ]]; then
	#	mkfreeglut
	#	mkftgl
	#	mkgtkglext
	#	mkgtkglextmm
	#fi
	
	mketl
	mksynfig
	mksynfigstudio
	mkconfig
	mkpackage
}

do_cleanup()
{
	echo "Cleaning up..."
	if [ ${PREFIX} != ${DEPSPREFIX} ]; then
		[ ! -e ${DEPSPREFIX} ] || mv ${DEPSPREFIX} ${DEPSPREFIX}.off
	fi
	[ ! -e ${SYSPREFIX} ] || mv ${SYSPREFIX} ${SYSPREFIX}.off
	exit
}

trap do_cleanup INT SIGINT SIGTERM EXIT

[ ! -e ${SYSPREFIX}.off ] || mv ${SYSPREFIX}.off ${SYSPREFIX}
[ ! -e ${DEPSPREFIX}.off ] || mv ${DEPSPREFIX}.off ${DEPSPREFIX}
[ -e ${DEPSPREFIX} ] || mkdir -p ${DEPSPREFIX}

if [ -z $1 ]; then
	mkall
else
	echo "Executing custom user command..."
	#mkprep
	set_environment


	$@
fi

do_cleanup
