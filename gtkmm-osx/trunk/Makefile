#export

# Directories
#ROOT_DIR=/Users/darco/Projects/GTK
ROOT_DIR=$(shell pwd)
PREFIX_DIR=$(ROOT_DIR)/output
PACKAGETMP_DIR=$(ROOT_DIR)/pkg_out
#PREFIX_DIR=/usr/local

GETTEXT_VER=0.14.5
GETTEXT_DIR=$(ROOT_DIR)/gettext-$(GETTEXT_VER)
GETTEXT_DL=http://mirrors.kernel.org/gnu/gettext/gettext-$(GETTEXT_VER).tar.gz

LIBICONV_VER=1.9.1
LIBICONV_DIR=$(ROOT_DIR)/libiconv-$(LIBICONV_VER)
LIBICONV_DL=http://mirrors.kernel.org/gnu/libiconv/libiconv-$(GETTEXT_VER).tar.gz

GLIB_VER=2.4.7
GLIB_VER=2.8.6
GLIB_DIR=$(ROOT_DIR)/glib-$(GLIB_VER)
GLIB_DL=http://ftp.gtk.org/pub/gtk/v2.8/glib-$(GLIB_VER).tar.gz

PANGO_VER=1.4.1
PANGO_VER=1.10.4
PANGO_DIR=$(ROOT_DIR)/pango-$(PANGO_VER)
PANGO_DL=http://ftp.gtk.org/pub/gtk/v2.8/pango-$(PANGO_VER).tar.gz

CAIRO_VER=1.0.2
CAIRO_DIR=$(ROOT_DIR)/cairo-$(CAIRO_VER)
CAIRO_DL=http://cairographics.org/releases/cairo-$(CAIRO_VER).tar.gz

ATK_VER=1.6.1
ATK_VER=1.10.3
ATK_DIR=$(ROOT_DIR)/atk-$(ATK_VER)
ATK_DL=http://ftp.gtk.org/pub/gtk/v2.8/atk-$(ATK_VER).tar.gz

GTK_VER=2.4.13
GTK_VER=2.8.13
GTK_DIR=$(ROOT_DIR)/gtk+-$(GTK_VER)
GTK_DL=http://ftp.gtk.org/pub/gtk/v2.8/gtk+-$(GTK_VER).tar.gz

LIBPNG_VER=1.2.5
LIBPNG_DIR=$(ROOT_DIR)/libpng-$(LIBPNG_VER)

LIBJPEG_VER=6b
LIBJPEG_DIR=$(ROOT_DIR)/jpeg-$(LIBJPEG_VER)

LIBTIFF_VER=3.5.7
LIBTIFF_DIR=$(ROOT_DIR)/tiff-v$(LIBTIFF_VER)

LIBSIGC_DIR=$(ROOT_DIR)/libsigc++-1.2.4
#LIBSIGC2_DIR=$(ROOT_DIR)/libsigc++-2.0.5

LIBSIGC2_VER=2.0.16
LIBSIGC2_DIR=$(ROOT_DIR)/libsigc++-$(LIBSIGC2_VER)
LIBSIGC2_DL=http://ftp.gnome.org/pub/GNOME/sources/libsigc++/2.0/libsigc++-$(LIBSIGC2_VER).tar.gz


GLIBMM_VER=2.4.5
GLIBMM_VER=2.8.5
GLIBMM_DIR=$(ROOT_DIR)/glibmm-$(GLIBMM_VER)
GLIBMM_DL=http://ftp.gnome.org/pub/GNOME/sources/glibmm/2.8/glibmm-$(GLIBMM_VER).tar.gz

GTKMM_VER=2.4.7
GTKMM_VER=2.8.3
GTKMM_DIR=$(ROOT_DIR)/gtkmm-$(GTKMM_VER)
GTKMM_DL=http://ftp.gnome.org/pub/GNOME/sources/gtkmm/2.8/gtkmm-$(GTKMM_VER).tar.gz

LIBXML2_VER=2.6.23
LIBXML2_DIR=$(ROOT_DIR)/libxml2-$(LIBXML2_VER)

LIBXMLPP_VER=1.0.4
LIBXMLPP_DIR=$(ROOT_DIR)/libxml++-$(LIBXMLPP_VER)
LIBXMLPP_DL=http://ftp.gnome.org/pub/GNOME/sources/libxml++/1.0/libxml++-$(LIBXMLPP_VER).tar.gz


FONTCONFIG_VER=2.2.96
FONTCONFIG_DIR=$(ROOT_DIR)/fontconfig-$(FONTCONFIG_VER)

INSTALL_PREFIX=/usr/local

# Other
prefix=$(PREFIX_DIR)
TEMP_FILE=$(ROOT_DIR)/tempfile.xxx
PKG_CONFIG_PATH=$(PREFIX_DIR)/lib/pkgconfig:/usr/X11R6/lib/pkgconfig:/usr/lib/pkgconfig
PREFIX_SYMBOL=@PREFIX@

#export DYLD_LIBRARY_PATH=$(PREFIX_DIR)/lib:/lib:/usr/local/lib:/lib:/usr/lib
export LD_PREBIND=1
export LD_PREBIND_ALLOW_OVERLAP=1
export LD_PREBIND_ALL_TWOLEVEL_MODULES=1
export MACOSX_DEPLOYMENT_TARGET=10.4
export CPP=gcc -E

# Programs
#MAKE=export MACOSX_DEPLOYMENT_TARGET=$(MACOSX_DEPLOYMENT_TARGET) && make
MAKE=make -j2
CP=cp
MV=mv
MKDIR=mkdir
GREP=grep
STRIP=strip
FIND=find
CHMOD=chmod
PRINTF=printf
SED=sed
PACKAGE=./package
CURL=curl -f
UNTARZ=tar xvzf
UNTARJ=tar xvjf
CAT=cat

OFLAGS=
ARCHFLAGS=-arch i386 -arch ppc

OFLAGS=-faltivec -maltivec
ARCHFLAGS=-arch ppc

CFLAGS=-I$(PREFIX_DIR)/include -no-cpp-precomp -prebind $(OFLAGS) $(ARCHFLAGS)
CC=gcc $(CFLAGS)
CC3=gcc $(CFLAGS)
CXXFLAGS=$(CFLAGS)
CXX=g++ $(CXXFLAGS)
CXXCPP=g++ -E
CPP=gcc -E
#LDFLAGS=-L$(PREFIX_DIR)/lib -prebind -flat_namespace -fcoalesce-templates
#LDFLAGS=-L$(PREFIX_DIR)/lib -prebind -prebind_allow_overlap -prebind_all_twolevel_modules -twolevel_namespace  -undefined dynamic_lookup
LDFLAGS=-L$(PREFIX_DIR)/lib -prebind -prebind_all_twolevel_modules -twolevel_namespace -bind_at_load -undefined dynamic_lookup


DESTDIR_FIX=for filename in `$(FIND) . | $(GREP) Makefile | $(GREP) -v -e Makefile.am -e Makefile.in` ; do { echo Patching $$filename ... && sed 's:LIBDIR=\\"$$(libdir)\\":LIBDIR=\\"$(INSTALL_PREFIX)/lib\\":g;s:DATA_PREFIX=\\"$$(prefix)\\":DATA_PREFIX=\\"$(INSTALL_PREFIX)\\":g; s:LOCALEDIR=\\"$$(localedir)\\":LOCALEDIR=\\"$(INSTALL_PREFIX)/share/locale\\":g; s:LOCALE_ALIAS_PATH=\\"$$(aliaspath)\\":LOCALE_ALIAS_PATH=\\"$(INSTALL_PREFIX)/share/locale\\":g;s:SYSCONFDIR=\\"$$(sysconfdir)\\":SYSCONFDIR=\\"$(INSTALL_PREFIX)/etc\\":g;' < $$filename > $(TEMP_FILE) && cp $(TEMP_FILE) $$filename ; } ; done ; 

# Flags
CONFIG_FLAGS=--prefix=$(PREFIX_DIR) --disable-static --enable-shared "CC=$(CC)" "CXX=$(CXX)" "CXXCPP=$(CXXCPP)" "CPP=$(CPP)" "PKG_CONFIG_PATH=$(PKG_CONFIG_PATH)" "LDFLAGS=$(LDFLAGS)" "MACOSX_DEPLOYMENT_TARGET=$(MACOSX_DEPLOYMENT_TARGET)"

# Targets
all: gettext glib libpng libjpeg cairo pango atk gtk libsigc2 glibmm gtkmm libxmlpp

packages: gettext-pkg glib-pkg pango-pkg atk-pkg libpng-pkg libjpeg-pkg cairo-pkg gtk-pkg libsigc2-pkg glibmm-pkg gtkmm-pkg libxmlpp-pkg

clean: gettext-clean glib-clean pango-clean atk-clean libpng-clean libjpeg-clean cairo-clean gtk-clean libsigc-clean libsigc2-clean glibmm-clean gtkmm-clean libxmlpp-clean
	$(RM) -r gtkmm.pkg

clean-all: clean
	$(RM) -r output

strip:
	$(STRIP) $(PREFIX_DIR)/lib/*.dylib

setup-uninstall:
	$(PRINTF) "\
#!/bin/bash\n\
cd @PREFIX@\n\
sudo for file in "'`'"cat bin/uninstall_gtk_list.txt"'`'"\n\
do\n\
	printf .\n\
	rm $$file\n\
done\n\
" > $(PREFIX_DIR)/bin/uninstall_gtk
	$(CHMOD) +x $(PREFIX_DIR)/bin/uninstall_gtk
	cd $(PREFIX_DIR) && find . | tac > bin/uninstall_gtk_list.txt

#PACKAGE_PREP="for filename in `find . | $(GREP) -e '\.la' -e '\.pc' -e '[-]config' -e '\.loaders' -e '\.immodules'` ; do [ -d $$filename ] || { $(GREP) -l '$(ROOT_DIR)' $$filename && $(SED) 's;$(PREFIX_DIR);@PREFIX@;g;s:-L$(ROOT_DIR)/gettext-0.11.5/intl::g;' < $$filename > $(TEMP_FILE) && $(CP) $(TEMP_FILE) $$filename ;} ; done; $(RM) $(TEMP_FILE)"

PACKAGE_PREP=for filename in `find | $(GREP) -e '\.la' -e '\.pc' -e '[-]config' -e '\.loaders' -e '\.immodules'` ; do [ -d $$filename ] || { $(SED) 's;$(PACKAGETMP_DIR);@PREFIX@;g;s;$(PREFIX_DIR);@PREFIX@;g;s:-L$(ROOT_DIR)/gettext-0.11.5/intl::g;' < $$filename > $(TEMP_FILE) && $(CP) $(TEMP_FILE) $$filename && echo $$filename ;} ; done; $(RM) $(TEMP_FILE) ; $(RM) -R share/doc

package-prep: setup-uninstall
	-cd $(PREFIX_DIR) && $(PACKAGE_PREP)	
#	-cd $(PREFIX_DIR) && for filename in `$(CAT) $(PREFIX_DIR)/bin/uninstall_gtk_list.txt | $(GREP) -e "\.la" -e "\.pc" -e -config -e "\.loaders" -e "\.immodules"` ;\
#		do [ -d $$filename ] || { \
#			$(GREP) -l "$(ROOT_DIR)" $$filename && \
#			$(SED) "s;$(PREFIX_DIR);@PREFIX@;g;s:-L$(ROOT_DIR)/gettext-0.11.5/intl::g;" \
#			< $$filename > $(TEMP_FILE) && \
#			$(CP) $(TEMP_FILE) $$filename \
#	;} ; done
#	$(RM) $(TEMP_FILE)


package:
	$(PACKAGE) $(PREFIX_DIR) gtkmm.info -r PkgResource -bzip
#	-[ -d gtkmm.xpm ] && mv gtkmm.xpm gtkmm.pkg

gettext: gettext-$(GETTEXT_VER)
	cd $(GETTEXT_DIR) && ./configure $(CONFIG_FLAGS) "LDFLAGS=$(LDFLAGS) -L$(GETTEXT_DIR)/intl" --with-libiconv-prefix=/usr/lib
#	$(SED) "s/__USER_LABEL_PREFIX__/_/g;" < $(GETTEXT_DIR)/intl/libgnuintl.h > $(TEMP_FILE)
#	$(CP) $(TEMP_FILE) $(GETTEXT_DIR)/intl/libgnuintl.h
#	$(RM) $(TEMPFILE)
#cd $(GETTEXT_DIR) && $(DESTDIR_FIX)
	$(MAKE) -C $(GETTEXT_DIR)
	$(MAKE) -C $(GETTEXT_DIR) install

gettext-pkg:
	$(RM) -r $(ROOT_DIR)/pkg_out
	$(MAKE) -C $(GETTEXT_DIR) install-strip prefix=$(ROOT_DIR)/pkg_out
	-cd $(ROOT_DIR)/pkg_out && $(PACKAGE_PREP)
	$(SED) "s/@PACKAGE@/gettext/g;s/@VERSION@/$(GETTEXT_VER)/;" < package.info > gettext.info
	$(PACKAGE) $(ROOT_DIR)/pkg_out gettext.info -r PkgResource -bzip
	$(RM) -r $(ROOT_DIR)/pkg_out
	mv $(ROOT_DIR)/gettext.pkg packages

gettext-clean:
	$(MAKE) -C $(GETTEXT_DIR) clean

gettext-$(GETTEXT_VER):
	$(CURL) $(GETTEXT_DL) -o gettext-$(GETTEXT_VER).tar.gz
	$(UNTARZ) gettext-$(GETTEXT_VER).tar.gz
	
libiconv:
	cd $(LIBICONV_DIR) && ./configure $(CONFIG_FLAGS)
	cd $(LIBICONV_DIR) && $(DESTDIR_FIX)
	$(MAKE) -C $(LIBICONV_DIR)
	$(MAKE) -C $(LIBICONV_DIR) install

libiconv-clean:
	$(MAKE) -C $(LIBICONV_DIR) clean

glib: glib-$(GLIB_VER)
	cd $(GLIB_DIR) && ./configure $(CONFIG_FLAGS)
	cd $(GLIB_DIR) && $(DESTDIR_FIX)
	$(MAKE) -C $(GLIB_DIR)
	$(MAKE) -C $(GLIB_DIR) install

glib-pkg:
	$(RM) -r $(ROOT_DIR)/pkg_out
	$(MAKE) -C $(GLIB_DIR) install-strip prefix=$(ROOT_DIR)/pkg_out
	-cd $(ROOT_DIR)/pkg_out && $(PACKAGE_PREP)
	$(SED) "s/@PACKAGE@/glib/g;s/@VERSION@/$(GLIB_VER)/;" < package.info > glib.info
	$(PACKAGE) $(ROOT_DIR)/pkg_out glib.info -r PkgResource -bzip
	$(RM) -r $(ROOT_DIR)/pkg_out
	mv $(ROOT_DIR)/glib.pkg packages

glib-clean:
	$(MAKE) -C $(GLIB_DIR) clean

glib-$(GLIB_VER):
	$(CURL) $(GLIB_DL) -o glib-$(GLIB_VER).tar.gz
	$(UNTARZ) glib-$(GLIB_VER).tar.gz




cairo: cairo-$(CAIRO_VER)
	cd $(CAIRO_DIR) && ./configure $(CONFIG_FLAGS)
	cd $(CAIRO_DIR) && $(DESTDIR_FIX)
	$(MAKE) -C $(CAIRO_DIR)
	$(MAKE) -C $(CAIRO_DIR) install

cairo-pkg:
	$(RM) -r $(ROOT_DIR)/pkg_out
	$(MAKE) -C $(CAIRO_DIR) install-strip prefix=$(ROOT_DIR)/pkg_out
	-cd $(ROOT_DIR)/pkg_out && $(PACKAGE_PREP)
	$(SED) "s/@PACKAGE@/cairo/g;s/@VERSION@/$(CAIRO_VER)/;" < package.info > cairo.info
	$(PACKAGE) $(ROOT_DIR)/pkg_out cairo.info -r PkgResource -bzip
	$(RM) -r $(ROOT_DIR)/pkg_out
	mv $(ROOT_DIR)/cairo.pkg packages

cairo-clean:
	$(MAKE) -C $(CAIRO_DIR) clean

cairo-$(CAIRO_VER):
	$(CURL) $(CAIRO_DL) -o cairo-$(CAIRO_VER).tar.gz
	$(UNTARZ) cairo-$(CAIRO_VER).tar.gz




pango: pango-$(PANGO_VER)
	cd $(PANGO_DIR) && ./configure $(CONFIG_FLAGS) 
	cd $(PANGO_DIR) && $(DESTDIR_FIX)
	$(MAKE) -C $(PANGO_DIR)
	$(MAKE) -C $(PANGO_DIR) install

pango-pkg:
	$(RM) -r $(ROOT_DIR)/pkg_out
	$(MAKE) -C $(PANGO_DIR) install-strip prefix=$(ROOT_DIR)/pkg_out
	-cd $(ROOT_DIR)/pkg_out && $(PACKAGE_PREP)
	$(SED) "s/@PACKAGE@/pango/g;s/@VERSION@/$(PANGO_VER)/;" < package.info > pango.info
	$(PACKAGE) $(ROOT_DIR)/pkg_out pango.info -r PkgResource -bzip
	$(RM) -r $(ROOT_DIR)/pkg_out
	mv $(ROOT_DIR)/pango.pkg packages

pango-clean:
	$(MAKE) -C $(PANGO_DIR) clean

pango-$(PANGO_VER):
	$(CURL) $(PANGO_DL) -o pango-$(PANGO_VER).tar.gz
	$(UNTARZ) pango-$(PANGO_VER).tar.gz

atk: atk-$(ATK_VER)
	cd $(ATK_DIR) && ./configure $(CONFIG_FLAGS) 
	cd $(ATK_DIR) && $(DESTDIR_FIX)
	$(MAKE) -C $(ATK_DIR)
	$(MAKE) -C $(ATK_DIR) install

atk-pkg:
	$(RM) -r $(ROOT_DIR)/pkg_out
	$(MAKE) -C $(ATK_DIR) install-strip prefix=$(ROOT_DIR)/pkg_out
	-cd $(ROOT_DIR)/pkg_out && $(PACKAGE_PREP)
	$(SED) "s/@PACKAGE@/atk/g;s/@VERSION@/$(ATK_VER)/;" < package.info > atk.info
	$(PACKAGE) $(ROOT_DIR)/pkg_out atk.info -r PkgResource -bzip
	$(RM) -r $(ROOT_DIR)/pkg_out
	mv $(ROOT_DIR)/atk.pkg packages

atk-clean:
	$(MAKE) -C $(ATK_DIR) clean

atk-$(ATK_VER):
	$(CURL) $(ATK_DL) -o atk-$(ATK_VER).tar.gz
	$(UNTARZ) atk-$(ATK_VER).tar.gz

libpng:
	$(MAKE) -C $(LIBPNG_DIR) -f makefile.macosx prefix=$(PREFIX_DIR)
	$(MAKE) -C $(LIBPNG_DIR) -f makefile.macosx install prefix=$(PREFIX_DIR)
	$(MV) $(PREFIX_DIR)/lib/libpng.dylib $(PREFIX_DIR)/lib/libpng.dylib.xxx

libpng-pkg:
	$(RM) -r $(ROOT_DIR)/pkg_out
	$(MKDIR) $(ROOT_DIR)/pkg_out
	$(MAKE) -C $(LIBPNG_DIR) -f makefile.macosx install prefix=$(ROOT_DIR)/pkg_out
	-cd $(ROOT_DIR)/pkg_out && $(PACKAGE_PREP)
	$(SED) "s/@PACKAGE@/libpng/g;s/@VERSION@/$(LIBPNG_VER)/;" < package.info > libpng.info
	$(PACKAGE) $(ROOT_DIR)/pkg_out libpng.info -r PkgResource -bzip
	$(RM) -r $(ROOT_DIR)/pkg_out
	mv $(ROOT_DIR)/libpng.pkg packages

libpng-clean:
	$(MAKE) -C $(LIBPNG_DIR) -f makefile.macosx clean


gtk: gtk-$(GTK_VER)
	-$(MV) $(PREFIX_DIR)/lib/libjpeg.dylib.xxx $(PREFIX_DIR)/lib/libjpeg.dylib
	-$(MV) $(PREFIX_DIR)/lib/libpng.dylib.xxx $(PREFIX_DIR)/lib/libpng.dylib
	$(SED) -i bak "s/ libjpeg.dylib//" $(PREFIX_DIR)/lib/libjpeg.la
	cd $(GTK_DIR) && ./configure $(CONFIG_FLAGS)  LIBS="-L/usr/X11R6/lib -lfontconfig -lxinerama" --without-libtiff
	-$(MV) $(PREFIX_DIR)/lib/libjpeg.dylib $(PREFIX_DIR)/lib/libjpeg.dylib.xxx
	-$(MV) $(PREFIX_DIR)/lib/libpng.dylib $(PREFIX_DIR)/lib/libpng.dylib.xxx
	cd $(GTK_DIR) && $(DESTDIR_FIX)
	$(MAKE) -C $(GTK_DIR)
	$(MAKE) -C $(GTK_DIR) install
	$(SED) -i bak "s:dependency_libs='':dependency_libs='-L/usr/X11R6/lib -lX11 -lXinerama':" $(PREFIX_DIR)/lib/libgdk-x11-2.0.la

gtk-pkg:
	$(RM) -r $(ROOT_DIR)/pkg_out
	$(MAKE) -C $(GTK_DIR) install-strip prefix=$(ROOT_DIR)/pkg_out
	-cd $(ROOT_DIR)/pkg_out && $(PACKAGE_PREP)
	$(SED) "s/@PACKAGE@/gtk/g;s/@VERSION@/$(GTK_VER)/;" < package.info > gtk.info
	$(PACKAGE) $(ROOT_DIR)/pkg_out gtk.info -r PkgResource -bzip
	#$(RM) -r $(ROOT_DIR)/pkg_out
	mv $(ROOT_DIR)/gtk.pkg packages

gtk-clean:
	$(MAKE) -C $(GTK_DIR) clean

gtk-$(GTK_VER):
	$(CURL) $(GTK_DL) -o gtk-$(GTK_VER).tar.gz
	$(UNTARZ) gtk-$(GTK_VER).tar.gz

libjpeg:
	cd $(LIBJPEG_DIR) && ./configure $(CONFIG_FLAGS)
	$(MAKE) -C $(LIBJPEG_DIR)
	$(MAKE) -C $(LIBJPEG_DIR) install
	$(MV) $(PREFIX_DIR)/lib/libjpeg.dylib $(PREFIX_DIR)/lib/libjpeg.dylib.xxx
#	$(RM) $(PREFIX_DIR)/lib/libjpeg.dylib

libjpeg-pkg:
	$(RM) -r $(ROOT_DIR)/pkg_out
	$(MKDIR) $(ROOT_DIR)/pkg_out
	$(MKDIR) $(ROOT_DIR)/pkg_out/include
	$(MKDIR) $(ROOT_DIR)/pkg_out/lib
	$(MKDIR) $(ROOT_DIR)/pkg_out/bin
	$(MKDIR) $(ROOT_DIR)/pkg_out/man
	$(MKDIR) $(ROOT_DIR)/pkg_out/man/man1
	$(MKDIR) $(ROOT_DIR)/pkg_out/share
	$(MAKE) -C $(LIBJPEG_DIR) install prefix=$(ROOT_DIR)/pkg_out
	-cd $(ROOT_DIR)/pkg_out && $(PACKAGE_PREP)
	$(SED) "s/@PACKAGE@/libjpeg/g;s/@VERSION@/$(LIBJPEG_VER)/;" < package.info > libjpeg.info
	$(PACKAGE) $(ROOT_DIR)/pkg_out libjpeg.info -r PkgResource -bzip
	$(RM) -r $(ROOT_DIR)/pkg_out
	mv $(ROOT_DIR)/libjpeg.pkg packages

libjpeg-clean:
	$(MAKE) -C $(LIBJPEG_DIR) clean


libtiff:
	MAKE=make && cd $(LIBTIFF_DIR) && ./configure --prefix=$(PREFIX_DIR) --noninteractive --with-CC="$(CC3)" --with-CFLAGS="$(CFLAGS)"
	$(MAKE) -C $(LIBTIFF_DIR) -e
	$(MAKE) -C $(LIBTIFF_DIR) install

libtiff-pkg:
	$(RM) -r $(ROOT_DIR)/pkg_out
	$(MAKE) -C $(LIBTIFF_DIR) install-strip prefix=$(ROOT_DIR)/pkg_out
	-cd $(ROOT_DIR)/pkg_out && $(PACKAGE_PREP)
	$(SED) "s/@PACKAGE@/libtiff/g;s/@VERSION@/$(LIBTIFF_VER)/;" < package.info > libtiff.info
	$(PACKAGE) $(ROOT_DIR)/pkg_out libtiff.info -r PkgResource -bzip
	$(RM) -r $(ROOT_DIR)/pkg_out
	mv $(ROOT_DIR)/libtiff.pkg packages

libtiff-clean:
	$(MAKE) -C $(LIBTIFF_DIR) clean

#LDFLAGS=-L$(PREFIX_DIR)/lib -prebind -flat_namespace # -fcoalesce-templates

libsigc:
	#cd $(LIBSIGC_DIR) && ./configure $(CONFIG_FLAGS) LDFLAGS="$(LDFLAGS) -lstdc++ -lsupc++" "CC=g++ $(CFLAGS)"
	cd $(LIBSIGC_DIR) && ./configure $(CONFIG_FLAGS) LDFLAGS="$(LDFLAGS) -lstdc++" "CC=g++ $(CFLAGS)"
	$(MAKE) -C $(LIBSIGC_DIR)
	$(MAKE) -C $(LIBSIGC_DIR) install

libsigc-pkg:
	$(RM) -r $(ROOT_DIR)/pkg_out
	$(MAKE) -C $(LIBSIGC_DIR) install-strip prefix=$(ROOT_DIR)/pkg_out
	-cd $(ROOT_DIR)/pkg_out && $(PACKAGE_PREP)
	$(SED) "s/@PACKAGE@/libsigc/g;s/@VERSION@/$(LIBSIGC_VER)/;" < package.info > libsigc.info
	$(PACKAGE) $(ROOT_DIR)/pkg_out libsigc.info -r PkgResource -bzip
	$(RM) -r $(ROOT_DIR)/pkg_out
	mv $(ROOT_DIR)/libsigc.pkg packages

libsigc-clean:
	$(MAKE) -C $(LIBSIGC_DIR) clean


glibmm: glibmm-$(GLIBMM_VER)
	#cd $(GLIBMM_DIR) && ./configure $(CONFIG_FLAGS) "LDFLAGS=$(LDFLAGS) -L/usr/X11R6/lib -lXinerama -lstdc++ -lsupc++"  "CC=g++ $(CFLAGS)"
	cd $(GLIBMM_DIR) && ./configure $(CONFIG_FLAGS) "LDFLAGS=$(LDFLAGS) -L/usr/X11R6/lib -lXinerama -lstdc++"  "CC=g++ $(CFLAGS)"
	cd $(GLIBMM_DIR) && $(DESTDIR_FIX)
	$(MAKE) -C $(GLIBMM_DIR)
	$(MAKE) -C $(GLIBMM_DIR) install

glibmm-pkg:
	$(RM) -r $(ROOT_DIR)/pkg_out
	$(MAKE) -C $(GLIBMM_DIR) install-strip prefix=$(ROOT_DIR)/pkg_out
	-cd $(ROOT_DIR)/pkg_out && $(PACKAGE_PREP)
	$(SED) "s/@PACKAGE@/glibmm/g;s/@VERSION@/$(GLIBMM_VER)/;" < package.info > glibmm.info
	$(PACKAGE) $(ROOT_DIR)/pkg_out glibmm.info -r PkgResource -bzip
	$(RM) -r $(ROOT_DIR)/pkg_out
	mv $(ROOT_DIR)/glibmm.pkg packages

gibmm-clean:
	$(MAKE) -C $(GLIBMM_DIR) clean

glibmm-$(GLIBMM_VER):
	$(CURL) $(GLIBMM_DL) -o glibmm-$(GLIBMM_VER).tar.gz
	$(UNTARZ) glibmm-$(GLIBMM_VER).tar.gz


gtkmm: gtkmm-$(GTKMM_VER)
	#cd $(GTKMM_DIR) && ./configure $(CONFIG_FLAGS) "LDFLAGS=$(LDFLAGS) -L/usr/X11R6/lib -lXinerama -lstdc++ -lsupc++"  "CC=g++ $(CFLAGS)"
	cd $(GTKMM_DIR) && ./configure $(CONFIG_FLAGS) "LDFLAGS=$(LDFLAGS) -L/usr/X11R6/lib -lXinerama -lstdc++"  "CC=g++ $(CFLAGS)"
	cd $(GTKMM_DIR) && $(DESTDIR_FIX)
	$(MAKE) -C $(GTKMM_DIR)
	$(MAKE) -C $(GTKMM_DIR) install

gtkmm-pkg:
	$(RM) -r $(ROOT_DIR)/pkg_out
	$(MAKE) -C $(GTKMM_DIR) install-strip prefix=$(ROOT_DIR)/pkg_out
	-cd $(ROOT_DIR)/pkg_out && $(PACKAGE_PREP)
	$(SED) "s/@PACKAGE@/gtkmm/g;s/@VERSION@/$(GTKMM_VER)/;" < package.info > gtkmm.info
	$(PACKAGE) $(ROOT_DIR)/pkg_out gtkmm.info -r PkgResource -bzip
	$(RM) -r $(ROOT_DIR)/pkg_out
	mv $(ROOT_DIR)/gtkmm.pkg packages

gtkmm-clean:
	$(MAKE) -C $(GTKMM_DIR) clean

gtkmm-$(GTKMM_VER):
	$(CURL) $(GTKMM_DL) -o gtkmm-$(GTKMM_VER).tar.gz
	$(UNTARZ) gtkmm-$(GTKMM_VER).tar.gz


libsigc2: libsigc++-$(LIBSIGC2_VER)
	#cd $(LIBSIGC2_DIR) && ./configure $(CONFIG_FLAGS) LDFLAGS="$(LDFLAGS) -lstdc++ -lsupc++" "CC=g++ $(CFLAGS)"
	cd $(LIBSIGC2_DIR) && ./configure $(CONFIG_FLAGS) LDFLAGS="$(LDFLAGS) -lstdc++" "CC=g++ $(CFLAGS)"
	$(MAKE) -C $(LIBSIGC2_DIR)
	$(MAKE) -C $(LIBSIGC2_DIR) install

libsigc2-pkg:
	$(RM) -r $(ROOT_DIR)/pkg_out
	$(MAKE) -C $(LIBSIGC2_DIR) install-strip prefix=$(ROOT_DIR)/pkg_out
	-cd $(ROOT_DIR)/pkg_out && $(PACKAGE_PREP)
	$(SED) "s/@PACKAGE@/libsigc2/g;s/@VERSION@/$(LIBSIGC2_VER)/;" < package.info > libsigc2.info
	$(PACKAGE) $(ROOT_DIR)/pkg_out libsigc2.info -r PkgResource -bzip
	$(RM) -r $(ROOT_DIR)/pkg_out
	mv $(ROOT_DIR)/libsigc2.pkg packages

libsigc2-clean:
	$(MAKE) -C $(LIBSIGC2_DIR) clean

libsigc++-$(LIBSIGC2_VER):
	$(CURL) $(LIBSIGC2_DL) -o libsigc2-$(LIBSIGC2_VER).tar.gz
	$(UNTARZ) libsigc2-$(LIBSIGC2_VER).tar.gz

LDFLAGS=-L$(PREFIX_DIR)/lib -prebind -prebind_all_twolevel_modules -twolevel_namespace -bind_at_load
CONFIG_FLAGS=--prefix=$(PREFIX_DIR) --disable-static --enable-shared "CC=$(CC)" "CXX=$(CXX)" "CXXCPP=$(CXXCPP)" "PKG_CONFIG_PATH=$(PKG_CONFIG_PATH)" "LDFLAGS=$(LDFLAGS)" "MACOSX_DEPLOYMENT_TARGET=$(MACOSX_DEPLOYMENT_TARGET)"

libxml2:
	cd $(LIBXML2_DIR) && ./configure $(CONFIG_FLAGS) 
	cd $(LIBXML2_DIR) && $(DESTDIR_FIX)
	$(MAKE) -C $(LIBXML2_DIR)
	$(MAKE) -C $(LIBXML2_DIR) install

libxml2-pkg:
	$(RM) -r $(ROOT_DIR)/pkg_out
	$(MAKE) -C $(LIBXML2_DIR) install-strip prefix=$(ROOT_DIR)/pkg_out
	-cd $(ROOT_DIR)/pkg_out && $(PACKAGE_PREP)
	$(SED) "s/@PACKAGE@/libxml2/g;s/@VERSION@/$(LIBXML2_VER)/;" < package.info > libxml2.info
	$(PACKAGE) $(ROOT_DIR)/pkg_out libxml2.info -r PkgResource -bzip
	$(RM) -r $(ROOT_DIR)/pkg_out
	mv $(ROOT_DIR)/libxml2.pkg packages

libxml2-clean:
	$(MAKE) -C $(LIBXML2_DIR) clean


fontconfig:
	cd $(FONTCONFIG_DIR) && ./configure $(CONFIG_FLAGS) --disable-docs --without-docdir HASDOCBOOK=no 
	cd $(FONTCONFIG_DIR) && $(DESTDIR_FIX)
	$(MAKE) -C $(FONTCONFIG_DIR)
	$(MAKE) -C $(FONTCONFIG_DIR) install

fontconfig-pkg:
	$(RM) -r $(ROOT_DIR)/pkg_out
	$(MAKE) -C $(FONTCONFIG_DIR) install-strip prefix=$(ROOT_DIR)/pkg_out
	-cd $(ROOT_DIR)/pkg_out && $(PACKAGE_PREP)
	$(SED) "s/@PACKAGE@/fontconfig/g;s/@VERSION@/$(FONTCONFIG_VER)/;" < package.info > fontconfig.info
	$(PACKAGE) $(ROOT_DIR)/pkg_out fontconfig.info -r PkgResource -bzip
	$(RM) -r $(ROOT_DIR)/pkg_out
	mv $(ROOT_DIR)/fontconfig.pkg packages

fontconfig-clean:
	$(MAKE) -C $(FONTCONFIG_DIR) clean

LDFLAGS=-L$(PREFIX_DIR)/lib -prebind -prebind_all_twolevel_modules -twolevel_namespace -bind_at_load
CONFIG_FLAGS=--prefix=$(PREFIX_DIR) --disable-static --enable-shared "CC=$(CC)" "CXX=$(CXX)" "CXXCPP=$(CXXCPP)" "PKG_CONFIG_PATH=$(PKG_CONFIG_PATH)" "LDFLAGS=$(LDFLAGS)" "MACOSX_DEPLOYMENT_TARGET=$(MACOSX_DEPLOYMENT_TARGET)"

libxmlpp: $(LIBXMLPP_DIR)
	cd $(LIBXMLPP_DIR) && ./configure $(CONFIG_FLAGS)
	cd $(LIBXMLPP_DIR) && $(DESTDIR_FIX)
	$(MAKE) -C $(LIBXMLPP_DIR)
	$(MAKE) -C $(LIBXMLPP_DIR) install

libxmlpp-pkg:
	$(RM) -r $(ROOT_DIR)/pkg_out
	$(MAKE) -C $(LIBXMLPP_DIR) install-strip prefix=$(ROOT_DIR)/pkg_out
	-cd $(ROOT_DIR)/pkg_out && $(PACKAGE_PREP)
	$(SED) "s/@PACKAGE@/libxmlpp/g;s/@VERSION@/$(LIBXMLPP_VER)/;" < package.info > libxmlpp.info
	$(PACKAGE) $(ROOT_DIR)/pkg_out libxmlpp.info -r PkgResource -bzip
	$(RM) -r $(ROOT_DIR)/pkg_out
	mv $(ROOT_DIR)/libxmlpp.pkg packages

libxmlpp-clean:
	$(MAKE) -C $(LIBXMLPP_DIR) clean

$(LIBXMLPP_DIR):
	$(CURL) $(LIBXMLPP_DL) -o libxmlpp-$(LIBXMLPP_VER).tar.gz
	$(UNTARZ) libxmlpp-$(LIBXMLPP_VER).tar.gz

.PHONY: package
