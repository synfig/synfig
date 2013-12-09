#!/bin/sh

# TODO: LD_PRELOAD wrapper
# TODO: rpm/deb/tgz packaging
# TODO: i386 build
# TODO: Migrate to crosstool-ng ???
# TODO: Bundle ALL dependent libs
# TODO: GTK themin issues

set -e

export SCRIPTPATH=$(cd `dirname "$0"`; pwd)

BUILDROOT_VERSION=1
BUILDROOT_LIBRARY_SET_ID=1

if [ -z $ARCH ]; then
	export ARCH="64"
fi

if [ -z $THREADS ]; then
	export THREADS=2
fi

export WORKSPACE=$HOME/synfig-buildroot
export PREFIX=$WORKSPACE/linux$ARCH/sys
export PREFIX_BUNDLE=$WORKSPACE/linux$ARCH/bundle
export PREFIX_DEPS=$WORKSPACE/linux$ARCH/sys-deps
export PREFIX_SRC=$WORKSPACE/linux$ARCH/source
export DISTPREFIX=$WORKSPACE/tmp/linux$ARCH
export CACHEDIR=$WORKSPACE/cache

[ -e ${PREFIX_SRC} ] || mkdir -p ${PREFIX_SRC}
[ -e ${PREFIX_DEPS} ] || mkdir -p ${PREFIX_DEPS}

export EMAIL='root@synfig.org'
SOURCES_URL="rsync://download.tuxfamily.org/pub/synfig/packages/sources/base"

# Bundled libraries
LIBSIGCPP_VERSION=2.2.10
GLEW_VERSION=1.5.1
CAIROMM_VERSION=1.8.0
IMAGEMAGICK_VERSION=6.8.6
PANGOMM_VERSION=2.26.3		# required by GTKMM 2.20.3
GTKMM_VERSION=2.20.3 		# !!! we need Notebook.set_action_widget()
FTGL_VERSION=2.1.2
FREEGLUT_VERSION=2.4.0
GTKGLEXT_VERSION=1.2.0
GTKGLEXTMM_VERSION=1.2.0
LIBXMLPP_VERSION=2.22.0
GLIBMM_VERSION=2.24.2		# required by GTKMM 2.20.3
CAIRO_VERSION=1.12.0		# required by the cairo render engine 2013-04-01
BOOST_VERSION=1_53_0

# System libraries
ATK_VERSION=1.29.4			# required by GTK 2.20.1
GLIB_VERSION=2.24.2			# required by GLIBMM 2.24.2
GTK_VERSION=2.20.1			# !!! we need Notebook.set_action_widget()
GTKENGINES_VERSION=2.20.2
PIXMAN_VERSION=0.22.0		# required by CAIRO 1.12.0
PANGO_VERSION=1.24.5
FONTCONFIG_VERSION=2.5.0

if [ -z $DEBUG ]; then
	export DEBUG=0
fi

if [[ $DEBUG == 1 ]]; then
	echo
	echo "Debug mode: enabled"
	echo
	DEBUG_OPT='--enable-debug --enable-optimization=0'
else
	DEBUG_OPT=''
fi

if [[ $ARCH == "32" ]]; then
	export SYS_ARCH=i386
	export LIBDIR="lib"
else
	export SYS_ARCH=amd64
	export LIBDIR="lib64"
fi

export VERSION=`cat ${SCRIPTPATH}/../synfig-core/configure.ac |egrep "AC_INIT\(\[Synfig Core\],"| sed "s|.*Core\],\[||" | sed "s|\],\[.*||"`
pushd "${SCRIPTPATH}" > /dev/null
export REVISION=`git show --pretty=format:%ci HEAD |  head -c 10 | tr -d '-'`
popd > /dev/null

set_environment()
{
	#export LD_LIBRARY_PATH=${PREFIX_DEPS}/lib:/${LIBDIR}:${PREFIX}/${LIBDIR}:${PREFIX}/usr/${LIBDIR}
	#export LD_LIBRARY_PATH=${PREFIX}/lib-native:${PREFIX_BUNDLE}/lib:${PREFIX_DEPS}/lib:${PREFIX}/${LIBDIR}:${PREFIX}/usr/${LIBDIR}
	export LD_PRELOAD=/${LIBDIR}/libc.so.6:/${LIBDIR}/libpthread.so.0:/${LIBDIR}/libdl.so.2
	export LD_LIBRARY_PATH=${PREFIX_BUNDLE}/lib:${PREFIX_DEPS}/lib:${PREFIX}/${LIBDIR}:${PREFIX}/usr/${LIBDIR}
	export PATH=${PREFIX_BUNDLE}/bin:${PREFIX_DEPS}/bin:${PREFIX}/bin:${PREFIX}/usr/bin
	export LDFLAGS="-Wl,-rpath -Wl,\\\$\$ORIGIN/lib -Wl,-rpath -Wl,${PREFIX}/${LIBDIR}" # -L${PREFIX}/usr/${LIBDIR}
	#export CFLAGS=" -nostdinc  -I${PREFIX}/usr/lib/gcc/x86_64-linux-gnu/4.3.2/include -I${PREFIX}/usr/lib/gcc/x86_64-linux-gnu/4.3.2/include-fixed  -I${PREFIX_BUNDLE}/include  -I${PREFIX_DEPS}/include -I${PREFIX}/usr/include"
	#export CXXFLAGS=" -nostdinc   -I${PREFIX}/usr/lib/gcc/../../include/c++/4.3  -I${PREFIX}/usr/lib/gcc/../../include/c++/4.3/x86_64-linux-gnu -I${PREFIX}/usr/lib/gcc/../../include/c++/4.3/backward -I${PREFIX}/usr/lib/gcc/x86_64-linux-gnu/4.3.2/include -I${PREFIX}/usr/lib/gcc/x86_64-linux-gnu/4.3.2/include-fixed -I${PREFIX_BUNDLE}/include  -I${PREFIX_DEPS}/include -I${PREFIX}/usr/include"
	export PKG_CONFIG_PATH=${PREFIX_BUNDLE}/lib/pkgconfig:${PREFIX_DEPS}/lib/pkgconfig:${PREFIX}/usr/lib/pkgconfig
	PERL_VERSION=`perl -v | sed -n '3p' | sed "s|This is perl, v||g" | cut -f 1 -d " "`
	export PERL5LIB="${PREFIX}/etc/perl:${PREFIX_DEPS}/lib/perl/${PERL_VERSION}:${PREFIX_DEPS}/share/perl/${PERL_VERSION}:${PREFIX}/usr/lib/perl5:${PREFIX}/usr/share/perl5:${PREFIX}/usr/lib/perl/${PERL_VERSION}:${PREFIX}/usr/share/perl/${PERL_VERSION}:${PREFIX_DEPS}/lib/site_perl"
}

run_native()
{
	LD_PRELOAD=""
	LD_LIBRARY_PATH=""
	PATH="/usr/local/bin/:/usr/bin:/bin"
	LDFLAGS=""
	CFLAGS=""
	CXXFLAGS=""
	PKG_CONFIG_PATH=""
	PERL5LIB=""
	"$@"
	set_environment
}

mkprefix()
{
	[ ! -e ${PREFIX}/dev ] || rm -rf ${PREFIX}/dev
	[ ! -e ${PREFIX}/proc ] || rm -rf ${PREFIX}/proc
	LD_LIBRARY_PATH=/${LIBDIR}:${PREFIX}/usr/${LIBDIR} \
	PATH=${PREFIX}/usr/sbin:${PREFIX}/sbin:${PREFIX}/usr/bin:${PREFIX}/bin:/usr/local/sbin:/usr/sbin:/sbin:/sbin:/bin:/usr/bin:$PATH HOME=/ LOGNAME=root \
		fakeroot fakechroot \
		debootstrap --variant=fakechroot --arch=$SYS_ARCH --include=sudo --include=apt lenny \
		${PREFIX} http://archive.debian.org/debian
	
	#LD_LIBRARY_PATH=/lib64 PATH=/usr/local/sbin:/usr/sbin:/sbin:/sbin:/bin:/usr/bin:${PREFIX}/usr/bin:$PATH fakeroot fakechroot chroot ${PREFIX}  /debootstrap/debootstrap --second-stage

	DEB_LIST_MINIMAL="\
			build-essential \
			libpng12-dev \
			libjpeg62-dev \
			libfreetype6-dev \
			libxml2-dev \
			libtiff4-dev \
			libjasper-dev \
			x11proto-xext-dev libdirectfb-dev libxfixes-dev libxinerama-dev libxdamage-dev libxcomposite-dev libxcursor-dev libxft-dev libxrender-dev libxt-dev libxrandr-dev libxi-dev libxext-dev libx11-dev \
			bzip2"

	LD_LIBRARY_PATH=/${LIBDIR}:${PREFIX}/usr/${LIBDIR} \
		PATH=${PREFIX}/usr/sbin:${PREFIX}/sbin:${PREFIX}/usr/bin:${PREFIX}/bin:/usr/local/sbin:/usr/sbin:/sbin:/sbin:/bin:/usr/bin:$PATH HOME=/ LOGNAME=root \
		fakeroot fakechroot chroot ${PREFIX} \
		aptitude install -o Aptitude::Cmdline::ignore-trust-violations=true -y ${DEB_LIST_MINIMAL}
	
	echo "Synfig Buildroot v${BUILDROOT_VERSION}" > ${PREFIX}/etc/chroot.id
}

mkprep()
{

if [ -z $NOSU ]; then
	su -c "yum install -y \
		automake \
		debootstrap \
		dpkg \
		fakeroot \
		fakechroot \
		"
fi

if [ ! -e ${PREFIX}/etc/chroot.id ]; then
	mkprefix
elif [[ `cat ${PREFIX}/etc/chroot.id` != "Synfig Buildroot v${BUILDROOT_VERSION}" ]]; then
	#rm -rf ${PREFIX} || true
	mkprefix
fi

#[ ! -e ${PREFIX}/lib-native ] || rm -rf ${PREFIX}/lib-native
#mkdir -p ${PREFIX}/lib-native
#for file in libc.so.6 libpthread.so.0 ; do
#	cp -L /${LIBDIR}/${file} ${PREFIX}/lib-native
#done
	
# Patching libraries ...
for lib in libc libpthread; do
	sed -i "s| /lib/| ${PREFIX}/lib/|g" ${PREFIX}/usr/lib/$lib.so
	sed -i "s| /usr/lib/| ${PREFIX}/usr/lib/|g" ${PREFIX}/usr/lib/$lib.so
done
for file in `find ${PREFIX}/usr/lib/pkgconfig/ -type f -name "*.pc"`; do
	sed -i "s|prefix=/usr|prefix=${PREFIX}/usr|g" ${file}
done
for file in `find ${PREFIX}/usr/lib/ -type f -name "*.la"`; do
	sed -i "s|libdir='/usr/lib'|libdir='${PREFIX}/usr/lib'|g" ${file}
	sed -i "s| /usr/lib| ${PREFIX}/usr/lib|g" ${file}
done

#RANDOM_PREFIX=`tr -cd '[:alnum:]' < /dev/urandom | fold -w8 | head -n1`
#DATE=`date +%s`
#LIB_PATH=${PREFIX}/tmp/lib-\${DATE}-\${RANDOM_PREFIX}
#mkdir -p \${LIB_PATH} || true
#for lib in libc.so.6 libpthread.so.0; do
#	cp -L ${PREFIX}/${LIBDIR}/\${lib} \${LIB_PATH}
#done

[ -e ${PREFIX_DEPS}/bin ] || mkdir -p ${PREFIX_DEPS}/bin

cat > ${PREFIX_DEPS}/bin/gcc-- <<EOF
#!/bin/sh

${PREFIX}/usr/bin/gcc -nostdinc -I${PREFIX}/usr/lib/gcc/x86_64-linux-gnu/4.3.2/include -I${PREFIX}/usr/lib/gcc/x86_64-linux-gnu/4.3.2/include-fixed -I${PREFIX_BUNDLE}/include  -I${PREFIX_DEPS}/include -I${PREFIX}/usr/include "\$@"
EOF
#chmod a+x  ${PREFIX_DEPS}/bin/gcc

cat > ${PREFIX_DEPS}/bin/g++ <<EOF
#!/bin/sh

${PREFIX}/usr/bin/g++ -nostdinc   -I${PREFIX}/usr/lib/gcc/../../include/c++/4.3  -I${PREFIX}/usr/lib/gcc/../../include/c++/4.3/x86_64-linux-gnu -I${PREFIX}/usr/lib/gcc/../../include/c++/4.3/backward -I${PREFIX}/usr/lib/gcc/x86_64-linux-gnu/4.3.2/include -I${PREFIX}/usr/lib/gcc/x86_64-linux-gnu/4.3.2/include-fixed -I${PREFIX_BUNDLE}/include  -I${PREFIX_DEPS}/include -I${PREFIX}/usr/include "\$@"
EOF
chmod a+x  ${PREFIX_DEPS}/bin/g++

cat > ${PREFIX_DEPS}/bin/rsync <<EOF
#!/bin/sh

/usr/bin/rsync "\$@"
EOF
chmod a+x  ${PREFIX_DEPS}/bin/rsync

#for binary in bzip2; do
#	ln -sf /usr/bin/$binary  ${PREFIX_DEPS}/bin/$binary
#done

}

mkglib()
{
PKG_NAME=glib
PKG_VERSION="${GLIB_VERSION}"
TAREXT=bz2
if ! pkg-config ${PKG_NAME}-2.0 --exact-version=${PKG_VERSION}  --print-errors; then
	rsync -av ${SOURCES_URL}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	pushd ${PREFIX_SRC}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xjf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --disable-static --enable-shared --prefix=${PREFIX_DEPS}/
	make -j${THREADS} install
	cd ..
	popd
fi
}

mkfontconfig()
{
PKG_NAME=fontconfig
PKG_VERSION="${FONTCONFIG_VERSION}"
TAREXT=gz
if ! pkg-config ${PKG_NAME} --exact-version=${PKG_VERSION}  --print-errors; then
	rsync -av ${SOURCES_URL}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	pushd ${PREFIX_SRC}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xzf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --disable-static --enable-shared --prefix=${PREFIX_DEPS}/
	make -j${THREADS} install
	cd ..
	popd
fi
}

mkatk()
{
PKG_NAME=atk
PKG_VERSION="${ATK_VERSION}"
TAREXT=bz2
if ! pkg-config ${PKG_NAME} --exact-version=${PKG_VERSION}  --print-errors; then
	rsync -av ${SOURCES_URL}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	pushd ${PREFIX_SRC}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xjf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --disable-static --enable-shared --prefix=${PREFIX_DEPS}/
	make -j${THREADS} install
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
	rsync -av ${SOURCES_URL}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	pushd ${PREFIX_SRC}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xzf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --disable-static --enable-shared --prefix=${PREFIX_DEPS}/
	make -j${THREADS} install
	cd ..
	popd
fi
}

mkcairo()
{
PKG_NAME=cairo
PKG_VERSION="${CAIRO_VERSION}"
TAREXT=gz
if ! pkg-config ${PKG_NAME} --exact-version=${PKG_VERSION}  --print-errors; then
	rsync -av ${SOURCES_URL}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	pushd ${PREFIX_SRC}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xzf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --prefix=${PREFIX_BUNDLE} \
		--disable-static 	\
		--enable-warnings 	\
		--enable-xlib 		\
		--enable-freetype 	\
	    --enable-gobject    \
		--disable-gtk-doc
	make -j${THREADS} install
	cd ..
	popd
fi
}

mkpango()
{
PKG_NAME=pango
PKG_VERSION="${PANGO_VERSION}"
TAREXT=bz2
if ! pkg-config ${PKG_NAME} --exact-version=${PKG_VERSION}  --print-errors; then
	rsync -av ${SOURCES_URL}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	pushd ${PREFIX_SRC}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xjf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --disable-static --enable-shared --prefix=${PREFIX_DEPS}/
	make -j${THREADS} install
	cd ..
	popd
fi
}

mkgtk()
{
PKG_NAME=gtk\+
PKG_VERSION="${GTK_VERSION}"
TAREXT=bz2
if ! pkg-config ${PKG_NAME}-2.0 --exact-version=${PKG_VERSION}  --print-errors; then
	#rsync -av ${SOURCES_URL}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	( cd ${WORKSPACE}/cache/ && wget -c --no-check-certificate http://ftp.gnome.org/pub/gnome/sources/gtk+/${PKG_VERSION%.*}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${PREFIX_SRC}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xjf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --prefix=${PREFIX_DEPS}/ \
		--disable-examples --disable-demos --disable-docs \
		--disable-static --enable-shared
	make -j${THREADS} install
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
	pushd ${PREFIX_SRC}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xjf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --prefix=${PREFIX_DEPS}/ \
		--disable-static --enable-shared
	make -j${THREADS} install
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
	pushd ${PREFIX_SRC}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xjf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} # && cd ${PKG_NAME}-${PKG_VERSION} && patch -p1 < ${WORKSPACE}/cache/libsigc++-2.0_2.0.18-2.diff && cd ..
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --prefix=${PREFIX_BUNDLE} --includedir=${PREFIX_BUNDLE}/include \
		--disable-static --enable-shared
	make -j${THREADS} install
	cd ..
	popd
fi
}

mkglibmm()
{
PKG_NAME=glibmm
PKG_VERSION="${GLIBMM_VERSION}"
TAREXT=bz2
if ! pkg-config ${PKG_NAME}-2.4 --exact-version=${PKG_VERSION}  --print-errors; then
	rsync -av ${SOURCES_URL}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	pushd ${PREFIX_SRC}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xjf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --prefix=${PREFIX_BUNDLE} --includedir=${PREFIX_BUNDLE}/include \
		--disable-fulldocs \
		--disable-static --enable-shared
	make -j${THREADS} install
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
	pushd ${PREFIX_SRC}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xjf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --prefix=${PREFIX_BUNDLE} --includedir=${PREFIX_BUNDLE}/include \
		--disable-static --enable-shared
	make -j${THREADS} install
	cd ..
	popd
fi
}

mkimagemagick()
{
PKG_NAME=ImageMagick
PKG_VERSION="${IMAGEMAGICK_VERSION}-10"
TAREXT=bz2
if ! pkg-config ${PKG_NAME} --exact-version=${IMAGEMAGICK_VERSION}  --print-errors; then
	( cd ${WORKSPACE}/cache/ && wget -c http://www.imagemagick.org/download/legacy/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${PREFIX_SRC}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xjf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --prefix=${PREFIX_BUNDLE} --includedir=${PREFIX_BUNDLE}/include \
		--disable-static --enable-shared \
		--with-modules \
		--without-perl \
		--without-x \
		--with-threads \
		--with-magick_plus_plus
	sed -i 's|^hardcode_libdir_flag_spec=.*|hardcode_libdir_flag_spec=""|g' libtool
	sed -i 's|^runpath_var=LD_RUN_PATH|runpath_var=DIE_RPATH_DIE|g' libtool
	make -j${THREADS} install
	cd ..
	popd
fi
}

mkboost()
{
PKG_NAME=boost
PKG_VERSION="${BOOST_VERSION}"
TAREXT=bz2
if ! cat ${PREFIX_BUNDLE}/include/boost/version.hpp |egrep "BOOST_LIB_VERSION \"${PKG_VERSION%_*}\""; then
	#PATH_BAK=$PATH
	#PATH="${PREFIX_DEPS}/bin-gcc/:$PATH"
	#rm -rf ${PREFIX_DEPS}/lib/libboost_program_options* || true
	#rm -rf ${PREFIX_BUNDLE}/lib/libboost_program_options* || true
	rsync -av ${SOURCES_URL}/${PKG_NAME}_${PKG_VERSION}.tar.${TAREXT} ${WORKSPACE}/cache/${PKG_NAME}_${PKG_VERSION}.tar.${TAREXT}
	pushd ${PREFIX_SRC}
	[ ! -d ${PKG_NAME}_${PKG_VERSION} ] && tar -xjf ${WORKSPACE}/cache/${PKG_NAME}_${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}_${PKG_VERSION}
	./bootstrap.sh --prefix=${PREFIX_BUNDLE} \
		--libdir=${PREFIX_BUNDLE}/lib \
		--exec-prefix=${PREFIX_BUNDLE} \
		--with-libraries=program_options
	./b2
	./b2 install || true
	cd ..
	popd
	#PATH="$PATH_BAK"
fi
#cp ${PREFIX_DEPS}/lib/libboost_program_options.so.*.0 ${PREFIX_BUNDLE}/lib/
}

mkcairomm()
{
PKG_NAME=cairomm
PKG_VERSION="${CAIROMM_VERSION}"
TAREXT=gz
if ! pkg-config ${PKG_NAME}-1.0 --exact-version=${PKG_VERSION}  --print-errors; then
	rsync -av ${SOURCES_URL}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	pushd ${PREFIX_SRC}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xzf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --prefix=${PREFIX_BUNDLE} --includedir=${PREFIX_BUNDLE}/include \
		--enable-docs=no \
		--disable-static --enable-shared
	make -j${THREADS} install
	cd ..
	popd
fi
}

mkpangomm()
{
PKG_NAME=pangomm
PKG_VERSION="${PANGOMM_VERSION}"
TAREXT=bz2
if ! pkg-config ${PKG_NAME}-1.4 --exact-version=${PKG_VERSION}  --print-errors; then
	rsync -av ${SOURCES_URL}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	pushd ${PREFIX_SRC}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xjf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --prefix=${PREFIX_BUNDLE} --includedir=${PREFIX_BUNDLE}/include \
		--disable-docs \
		--disable-static --enable-shared
	make -j${THREADS} install
	cd ..
	popd
fi
}

mkgtkmm()
{
PKG_NAME=gtkmm
PKG_VERSION="${GTKMM_VERSION}"
TAREXT=bz2
if ! pkg-config ${PKG_NAME}-2.4 --exact-version=${PKG_VERSION}  --print-errors; then
	( cd ${WORKSPACE}/cache/ && wget -c --no-check-certificate http://ftp.gnome.org/pub/GNOME/sources/gtkmm/${PKG_VERSION%.*}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	#rsync -av ${SOURCES_URL}/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	pushd ${PREFIX_SRC}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xjf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --prefix=${PREFIX_BUNDLE} --includedir=${PREFIX_BUNDLE}/include \
		--disable-examples --disable-demos --disable-docs \
		--disable-static --enable-shared
	make -j${THREADS} install
	cd ..
	popd
fi
}

mkautoconf()
{
PKG_NAME=autoconf
PKG_VERSION="2.69"
TAREXT=gz

mkdir -p $PREFIX/tmp/${PKG_NAME}-bin || true
cat > $PREFIX/tmp/${PKG_NAME}-bin/m4 <<EOF
#!/bin/sh

/usr/bin/m4  "\$@"

EOF
chmod a+x  $PREFIX/tmp/${PKG_NAME}-bin/m4



PATH_BAK=$PATH
PATH="$PREFIX/tmp/${PKG_NAME}-bin/:$PATH"

if [ ! -e ${PREFIX_DEPS}/bin/autoconf ]; then
	( cd ${WORKSPACE}/cache/ && wget -c http://ftp.gnu.org/gnu/autoconf/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${PREFIX_SRC}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xzf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --prefix=${PREFIX_DEPS}
	make -j${THREADS} install
	cd ..
	popd
fi


PATH="$PATH_BAK"
}

mkautomake()
{
PKG_NAME=automake
PKG_VERSION="1.14"
TAREXT=gz
if [ ! -e ${PREFIX_DEPS}/bin/automake ]; then
	( cd ${WORKSPACE}/cache/ && wget -c http://ftp.gnu.org/gnu/automake/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${PREFIX_SRC}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xzf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --prefix=${PREFIX_DEPS}
	make -j${THREADS} install
	cd ..
	popd
fi
}

mklibtool()
{
PKG_NAME=libtool
PKG_VERSION="2.4.2"
TAREXT=gz

mkdir -p $PREFIX/tmp/${PKG_NAME}-bin || true
cat > $PREFIX/tmp/${PKG_NAME}-bin/gcc <<EOF
#!/bin/sh

${PREFIX}/usr/bin/gcc  "\$@"

EOF
chmod a+x  $PREFIX/tmp/${PKG_NAME}-bin/gcc



PATH_BAK=$PATH
PATH="$PREFIX/tmp/${PKG_NAME}-bin/:$PATH"


if [ ! -e ${PREFIX_DEPS}/bin/libtoolize ]; then
	( cd ${WORKSPACE}/cache/ && wget -c http://ftpmirror.gnu.org/libtool/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${PREFIX_SRC}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xzf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --prefix=${PREFIX_DEPS}
	make -j${THREADS} install
	cd ..
	popd
fi

[ -e ${PREFIX_BUNDLE}/lib/ ] || mkdir -p ${PREFIX_BUNDLE}/lib/
rm -rf ${PREFIX_BUNDLE}/lib/libltdl* || true
cp ${PREFIX_DEPS}/lib/libltdl.so* ${PREFIX_BUNDLE}/lib/

PATH="$PATH_BAK"
}

mkintltool()
{
PKG_NAME=intltool
PKG_VERSION="0.50.2"
TAREXT=gz


if [ ! -e ${PREFIX_DEPS}/bin/intltoolize ]; then
	( cd ${WORKSPACE}/cache/ && wget -c --no-check-certificate https://launchpad.net/intltool/trunk/0.50.2/+download/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${PREFIX_SRC}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xzf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --prefix=${PREFIX_DEPS}
	make -j${THREADS} install
	cd ..
	popd
fi

}

mkgettext()
{

PKG_NAME=gettext
PKG_VERSION="0.17"
TAREXT=gz

mkdir -p $PREFIX/tmp/gettext-bin || true
cat > $PREFIX/tmp/gettext-bin/gij <<EOF
#!/bin/sh

false
EOF
chmod a+x  $PREFIX/tmp/gettext-bin/gij

cat > $PREFIX/tmp/gettext-bin/gcc <<EOF
#!/bin/sh

${PREFIX}/usr/bin/gcc  "\$@"

EOF
chmod a+x  $PREFIX/tmp/gettext-bin/gcc



PATH_BAK=$PATH
PATH="$PREFIX/tmp/gettext-bin/:$PATH"


if [ ! -e ${PREFIX_DEPS}/bin/gettext ]; then
	( cd ${WORKSPACE}/cache/ && wget -c http://ftp.gnu.org/pub/gnu/gettext/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} )
	pushd ${PREFIX_SRC}
	[ ! -d ${PKG_NAME}-${PKG_VERSION} ] && tar -xzf ${WORKSPACE}/cache/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
	# cd ${PKG_NAME}-${PKG_VERSION} && patch -p1 < ${WORKSPACE}/cache/gettext-${PKG_VERSION}-4.patch && cd ..
	cd ${PKG_NAME}-${PKG_VERSION}
	./configure --prefix=${PREFIX_DEPS} \
		--disable-java --disable-native-java
	make -j${THREADS} install
	cd ..
	popd
fi

PATH="$PATH_BAK"
}


#ETL
mketl()
{
cd $SCRIPTPATH/../ETL
make clean || true
run_native autoreconf --install --force
./configure --prefix=${PREFIX_BUNDLE} \
	--includedir=${PREFIX_BUNDLE}/include --libdir=${PREFIX_BUNDLE}/lib \
	--bindir=${PREFIX_BUNDLE}/bin \
	$DEBUG_OPT
make -j${THREADS} install
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
./configure --prefix=${PREFIX_BUNDLE} \
	--includedir=${PREFIX_BUNDLE}/include \
	--libdir=${PREFIX_BUNDLE}/lib --bindir=${PREFIX_BUNDLE}/bin \
	--sysconfdir=${PREFIX_BUNDLE}/etc \
	--disable-static --enable-shared \
	--with-magickpp --without-libavcodec \
	--with-boost=${PREFIX_BUNDLE}/ \
	--enable-warnings=minimum \
	$DEBUG_OPT
make -j${THREADS} install
}

#synfig-studio
mksynfigstudio()
{
cd $SCRIPTPATH/../synfig-studio/
make clean || true
[ ! -e config.cache ] || rm config.cache
/bin/sh ./bootstrap.sh
./configure --prefix=${PREFIX_BUNDLE} \
	--includedir=${PREFIX}/include \
	--libdir=${PREFIX_BUNDLE}/lib --bindir=${PREFIX_BUNDLE}/bin \
	--sysconfdir=${PREFIX_BUNDLE}/etc --datadir=${PREFIX_BUNDLE}/share  \
	--disable-static --enable-shared \
	$DEBUG_OPT
make -j${THREADS} install

}

mkconfig()
{

cat > ${PREFIX_BUNDLE}/synfigstudio <<EOF
#!/bin/sh

# Create install-location-dependent config files for Pango and GDK image loaders
# We have to do this every time because its possible that BIN_DIR has changed

sed "s?@ROOTDIR@/modules?$LIB_DIR/modules?" < $ETC_DIR/pango.modules.in > $USER_ARDOUR_DIR/pango.modules
sed "s?@ROOTDIR@/loaders?$LIB_DIR/loaders?" < $ETC_DIR/gdk-pixbuf.loaders.in > $USER_ARDOUR_DIR/gdk-pixbuf.loaders

# 1 check if test application starts without warnings
LANG=C GTK_PATH=/usr/lib64/gtk-2.0/2.10.0/ /home/zelgadis/synfig-buildroot/linux64/bundle/bin/gtk-test --g-fatal-warnings | egrep "Gtk+ version too old (micro mismatch)"

# If everything is fine then start with GTK_PATH

GTK_PATH=/usr/lib64/gtk-2.0/2.10.0/ /home/zelgadis/synfig-buildroot/linux64/bundle/bin/synfigstudio

# otherwise start with custom GTKRC

GTK2_RC_FILES=/home/zelgadis/synfig-buildroot/linux64/bundle/gtkrc:$GTK2_RC_FILES /home/zelgadis/synfig-buildroot/linux64/bundle/bin/synfigstudio

EOF

}

mkpreloader()
{

mkdir -p ${PREFIX_SRC}/preloader
pushd ${PREFIX_SRC}/preloader >/dev/null
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
cp synfig-pkg-preloader.so ${PREFIX_BUNDLE}/lib
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

[ -e ffmpeg-latest-win32-static.7z ] || wget -c http://ffmpeg.zeranoe.com/builds/win32/static/ffmpeg-latest-win32-static.7z
[ ! -d ffmpeg ] || rm -rf ffmpeg
mkdir -p ffmpeg
cd ffmpeg
7z e ../ffmpeg-latest-win32-static.7z
cp ffmpeg.exe $PREFIX/bin
[ -d $PREFIX/licenses ] || mkdir -p $PREFIX/licenses
cp *.txt $PREFIX/licenses
cd ..
rm -rf ffmpeg

[ -e portable-python-3.2.5.1.zip ] || wget -c http://download.tuxfamily.org/synfig/packages/sources/portable-python-3.2.5.1.zip
[ ! -d python ] || rm -rf python
unzip portable-python-3.2.5.1.zip
[ ! -d $PREFIX/python ] || rm -rf $PREFIX/python
mv python $PREFIX



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

mv synfigstudio-${VERSION}.exe ../synfigstudio-${VERSION}-${REVISION}-${ARCH}bit.exe
}

mkall()
{
	mkprep
	
	set_environment
	
	# build tools
	mkautoconf
	mkautomake
	mklibtool
	mkintltool
	mkgettext
	
	# system libraries
	mkglib
	mkfontconfig
	mkatk
	mkpixman
	mkcairo # bundled library
	mkpango
	mkgtk
	
	# synfig-core deps
	mklibsigcpp
	mkglibmm
	mklibxmlpp
	#if [[ $OPENGL == 1 ]]; then
	#	mkglew
	#fi
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
	#mkpackage
}

chmod u+rwX ${PREFIX}

if [ -z $1 ]; then
	mkall
else
	echo "Executing custom user command..."
	mkprep
	set_environment


	$@
fi

chmod a-rwX ${PREFIX}
