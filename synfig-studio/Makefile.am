# $Id$

MAINTAINERCLEANFILES = \
	COPYING \
	INSTALL \
	config/ltmain.sh \
	doxygen.cfg \
	config/config.guess \
	config/config.sub \
	config/ltmain.sh \
	config/install-sh \
	config/mkinstalldirs \
	config/aclocal.m4 \
	config/missing \
	config/texinfo.tex \
	config/depcomp \
	aclocal.m4 \
	config.h.in \
	configure \
	stamp-h.in \
	Makefile.in \
	config.log \
	config.status \
	.doc_stamp \
	.DS_Store

DISTCLEANFILES = \
	org.synfig.SynfigStudio.desktop \
	$(appdata_DATA)

if WITH_IMAGES
IMAGE_DIR = images
endif

#if WITH_SOUNDS
SOUND_DIR = sounds
#endif

SUBDIRS = \
	build_tools \
	brushes \
	src \
	$(IMAGE_DIR) \
	$(SOUND_DIR) \
	plugins \
	ui \
	po

EXTRA_DIST = \
	COPYING \
	TODO \
	m4/subs.m4 \
	doxygen.cfg.in \
	doxygen.cfg \
	macosxbuild.sh \
	config/package \
	config/depcomp \
	m4/cxx_macros.m4 \
	m4/ETL.m4 \
	ChangeLog.old \
	org.synfig.SynfigStudio.desktop.in \
	org.synfig.SynfigStudio.appdata.xml.in \
	org.synfig.SynfigStudio.xml.in \
	synfigstudio-thumbnailer.schemas.in

DISTCHECK_CONFIGURE_FLAGS = --disable-update-mimedb

# Desktop entry
desktopdir = $(prefix)/share/applications
desktop_DATA = org.synfig.SynfigStudio.desktop

@INTLTOOL_DESKTOP_RULE@

mimeinfodir              = $(prefix)/share/mime-info
mimeinfo_DATA            = synfigstudio.keys synfigstudio.mime

# Appdata
appdatadir = $(datadir)/appdata
appdata_DATA = org.synfig.SynfigStudio.appdata.xml
@INTLTOOL_XML_RULE@

# Icon
if WITH_IMAGES
icondir                 = $(datadir)/pixmaps
icon_DATA               = images/synfig_icon.png images/sif_icon.png
endif

mimedir              = $(prefix)/share/mime/packages
mime_DATA            = org.synfig.SynfigStudio.xml

ACLOCAL_AMFLAGS=-I m4

GREP=grep
PRINTF=printf
SH=sh
DOXYGEN=doxygen

SVN=svn
TAG=@PACKAGE_TARNAME@_@VERSION_MAJ@_@VERSION_MIN@_@VERSION_REV@

ChangeLog:
	../autobuild/git2cl > ChangeLog

stats:
	-@echo
	-@echo  -- Stats
	-@echo
	-@$(PRINTF) "Total lines: "
	-@wc -l `shell find $(top_srcdir)/src -name '*.[ch]*' | $(GREP) -v libavcodec` | $(GREP) total
	-@$(PRINTF) "Total size: "
	-@du -hcs `shell find $(top_srcdir)/src -name '*.[ch]*' | $(GREP) -v libavcodec` | $(GREP) total
	-@echo

listfixmes:
	-@echo
	-@echo  -- List of pending FIXMEs
	-@echo
	-@$(GREP) FIXME -n `shell find $(top_srcdir) -name '*.[ch]*' | grep -v svn`
	-@echo

listhacks:
	-@echo
	-@echo  -- List of pending HACKs
	-@echo
	-@$(GREP) HACK -n `shell find $(top_srcdir) -name '*.[ch]*' | grep -v svn`
	-@echo

run: check

.doc_stamp: doxygen.cfg
	$(DOXYGEN) doxygen.cfg
	touch .doc_stamp

package-osx: all pkg-info/macosx/synfig-studio.info
	[ -d pkg_root ] && $(RMDIR) pkg_root || true
	convert images/installer_logo_osx.png $(srcdir)/pkg-info/macosx/studio-resources/background.tif
	make install prefix="`pwd`/pkg_root"
	$(srcdir)/config/package pkg_root pkg-info/macosx/synfig-studio.info -r $(srcdir)/pkg-info/macosx/studio-resources
if MACOSX_PKG
package: package-osx
endif

install-data-hook:
if ENABLE_UPDATE_MIMEDB
	$(UPDATE_MIME_DATABASE) "$(DESTDIR)$(datadir)/mime"
endif

uninstall-hook:
if ENABLE_UPDATE_MIMEDB
	$(UPDATE_MIME_DATABASE) "$(DESTDIR)$(datadir)/mime"
endif

html: .doc_stamp

rtf: .doc_stamp

docs: pdf html

.PHONY: stats listfixmes listhacks check docs pdf html rtf
