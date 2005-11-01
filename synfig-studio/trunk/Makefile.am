# $Header: /opt/voria/cvs/studio/Makefile.am,v 1.3 2005/03/24 21:40:05 darco Exp $

AUTOMAKE_OPTIONS=dist-zip dist-shar dist-bzip2

MAINTAINERCLEANFILES=config/package config/ltmain.sh config/config.guess config/config.sub config/ltmain.sh config/install-sh config/mkinstalldirs config/aclocal.m4 config/missing config/texinfo.tex config/depcomp aclocal.m4 config.h.in configure stamp-h.in Makefile.in config.log config.status .doc_stamp .DS_Store @PACKAGE_TARNAME@-@VERSION@.spec

SUBDIRS=src images

EXTRA_DIST=COPYING macosxbuild.sh win32build.sh win32inst.nsi.in bootstrap config/package config/depcomp config/cxx_macros.m4 @PACKAGE_TARNAME@-@VERSION@.spec config/project.spec.in config/build.cfg config/ETL.m4

# Desktop entry
#desktopdir              = $(datadir)/gnome/apps/Graphics
desktopdir              = $(prefix)/share/applications
desktop_DATA            = synfigstudio.desktop
#desktop_in_files        = synfigstudio.desktop.in
#desktop_DATA            = $(desktop_in_files:.desktop.in=.desktop)

# @INTLTOOL_DESKTOP_RULE@

mimedir              = $(prefix)/share/mime-info
mime_DATA            = synfigstudio.keys synfigstudio.mime

# Icon
icondir                 = $(datadir)/pixmaps
icon_DATA               = images/synfig_icon.png images/sif_icon.png


CVS=cvs
GREP=grep
PRINTF=printf
SH=sh
DOXYGEN=doxygen

stats:
	-@echo
	-@echo  -- Stats
	-@echo
	-@$(PRINTF) "Total lines: "
	-@wc -l $(shell find $(top_srcdir)/src -name '*.[ch]*' | $(GREP) -v libavcodec) | $(GREP) total
	-@$(PRINTF) "Total size: "
	-@du -hcs $(shell find $(top_srcdir)/src -name '*.[ch]*' | $(GREP) -v libavcodec) | $(GREP) total
	-@echo 

tagstable:
	$(CVS) tag -F stable

tagrelease:
	$(CVS) tag -F @PACKAGE_TARNAME@_@VERSION_MAJ@_@VERSION_MIN@_@VERSION_REV@

listfixmes:
	-@echo
	-@echo  -- List of pending FIXMEs
	-@echo
	-@$(GREP) FIXME -n $(shell find $(top_srcdir) -name '*.[ch]*' | grep -v svn)
	-@echo

listhacks:
	-@echo
	-@echo  -- List of pending HACKs
	-@echo
	-@$(GREP) HACK -n $(shell find $(top_srcdir) -name '*.[ch]*' | grep -v svn)
	-@echo

run: check

$(srcdir)/configure:  $(srcdir)/configure.in $(ACLOCAL_M4) $(CONFIGURE_DEPENDENCIES)
	$(srcdir)/bootstrap

.doc_stamp: doxygen.cfg
	$(DOXYGEN) doxygen.cfg
	touch .doc_stamp

win32inst.nsi: $(srcdir)/win32inst.nsi.in
	./config.status win32inst.nsi

installer: all win32inst.nsi
#	convert images/about_dialog.png images/about_dialog.bmp
	make -C images sif_icon.ico
	makensis win32inst.nsi

#if WIN32_PKG	
#package: installer
#else
#if MACOSX_PKG
package: all pkg-info/macosx/synfig-studio.info
	[ -d pkg_root ] && $(RMDIR) pkg_root || true
	make install prefix="`pwd`/pkg_root"
	$(srcdir)/config/package pkg_root pkg-info/macosx/synfig-studio.info -r $(srcdir)/pkg-info/macosx/studio-resources
#endif
#endif

html: .doc_stamp

rtf: .doc_stamp

docs: pdf html

.PHONY: stats tagstable tagrelease listfixmes listhacks check docs pdf html rtf
