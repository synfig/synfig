# $Id$

MAINTAINERCLEANFILES=COPYING INSTALL @PACKAGE_TARNAME@.pc.in @PACKAGE_TARNAME@.tar.gz doxygen.cfg config/config.guess config/config.sub config/ltmain.sh config/ltmain.sh.orig config/ltmain.sh.rej config/install-sh config/mkinstalldirs config/aclocal.m4 config/missing config/texinfo.tex config/depcomp aclocal.m4 config.h.in configure stamp-h.in Makefile.in config.log config.status .doc_stamp .DS_Store @PACKAGE_TARNAME@.spec include/etl_profile.h.in libltdl/COPYING.LIB  libltdl/Makefile.in  libltdl/acinclude.m4  libltdl/config-h.in  libltdl/configure.in  libltdl/ltdl.h libltdl/Makefile.am  libltdl/README       libltdl/aclocal.m4    libltdl/configure    libltdl/ltdl.c        libltdl/stamp-h.in


SUBDIRS=libltdl src examples

# Install the pkg-config file:
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = @PACKAGE_TARNAME@.pc

bin_SCRIPTS=synfig-config

EXTRA_DIST=ChangeLog.old TODO config/subs.m4 bootstrap config/package doc/hw-gfx-mapping.txt win32build.sh macosxbuild.sh win32inst.nsi.in COPYING config/depcomp config/synfig.m4 config/cxx_macros.m4 @PACKAGE_TARNAME@.spec config/project.spec.in config/build.cfg synfig.kdevprj synfig-config.in config/ETL.m4 config/doxygen.cfg.in doxygen.cfg config/pkgconfig.pc.in config/pkgconfig.m4 config/libxml++.m4 @PACKAGE_TARNAME@.pc.in pkg-info/macosx/core-resources/ReadMe.txt pkg-info/macosx/core-resources/Welcome.txt pkg-info/macosx/core-resources/install.sh

CVS=cvs
GREP=grep
PRINTF=printf
SH=sh
DOXYGEN=doxygen

SVN=svn
TAG=@PACKAGE_TARNAME@_@VERSION_MAJ@_@VERSION_MIN@_@VERSION_REV@


tagstable:
	-$(SVN) delete $(SVN_REPOSITORY)/tags/stable -m "Stable Tag: Removing old tag"
	$(SVN) copy $(top_srcdir) $(SVN_REPOSITORY)/tags/stable -m "Stable Tag: Copying everything over"

tagrelease:
	$(SVN) copy $(top_srcdir) $(SVN_REPOSITORY)/tags/$(TAG) -m "Release $(TAG)"

stats:
	-@echo
	-@echo  -- Stats
	-@echo
	-@$(PRINTF) "Total lines: "
	-@wc -l $(shell find $(top_srcdir)/src -name '*.[ch]*') | $(GREP) total
	-@$(PRINTF) "Total size: "
	-@du -hcs $(shell find $(top_srcdir)/src -name '*.[ch]*') | $(GREP) total
	-@echo 

ChangeLog:
	-svn update
	svn2cl --include-rev || touch ChangeLog

#tagstable:
#	$(CVS) tag -F stable

#tagrelease:
#	$(CVS) tag -F @PACKAGE_TARNAME@_@VERSION_MAJ@_@VERSION_MIN@_@VERSION_REV@


listfixmes:
	-@echo
	-@echo  -- List of pending FIXMEs
	-@echo
	-@$(GREP) -e FIXME -e "\\fixme" -n $(shell find $(top_srcdir) -name '*.[ch]*')
	-@echo

listhacks:
	-@echo
	-@echo  -- List of pending HACKs
	-@echo
	-@$(GREP) -e HACK -e "\\hack" -n $(shell find $(top_srcdir) -name '*.[ch]*')
	-@echo

listtodos:
	-@echo
	-@echo  -- List of pending TODOs
	-@echo
	-@$(GREP) -e TODO -e "\\todo" -n $(shell find $(top_srcdir) -name '*.[ch]*')
	-@echo

listoptimizes:
	-@echo
	-@echo  -- List of pending optimizations
	-@echo
	-@$(GREP) -e OPTIMIZE -e "\\optimize" -n $(shell find $(top_srcdir) -name '*.[ch]*')
	-@echo

listwritemes:
	-@echo
	-@echo  -- List of pending WRITEMEs
	-@echo
	-@$(GREP) -e WRITEME -e "\\writeme" -n $(shell find $(top_srcdir) -name '*.[ch]*')
	-@echo

install-bin:
	make -C src/synfig install-libLTLIBRARIES

win32inst.nsi: $(srcdir)/win32inst.nsi.in
	./config.status win32inst.nsi

pkg-info/macosx/synfig-core.info: $(srcdir)/pkg-info/macosx/synfig-core.info.in
	./config.status pkg-info/macosx/synfig-core.info

RMDIR=rm -fr

if WIN32_PKG	
package: all win32inst.nsi
	cp -fu `which libHalf-4.dll` src/modules/mod_openexr/.libs/
	cp -fu `which libIlmImf-4.dll` src/modules/mod_openexr/.libs/
	cp -fu `which libIlmThread-4.dll` src/modules/mod_openexr/.libs/
	cp -fu `which libIex-4.dll` src/modules/mod_openexr/.libs/
	makensis win32inst.nsi
else
if MACOSX_PKG
package: all pkg-info/macosx/synfig-core.info
	[ -d pkg_root ] && $(RMDIR) pkg_root || true
	make install-strip prefix="`pwd`/pkg_root"
	$(srcdir)/config/package pkg_root pkg-info/macosx/synfig-core.info -r $(srcdir)/pkg-info/macosx/core-resources
endif
endif

run: check

$(srcdir)/configure:  $(srcdir)/configure.in $(ACLOCAL_M4) $(CONFIGURE_DEPENDENCIES)
	$(srcdir)/bootstrap

.doc_stamp: doxygen.cfg
	$(DOXYGEN) doxygen.cfg
	touch .doc_stamp

benchmark:
	synfig -b -t null -q --time 0 $(srcdir)/examples/*.sif

html: .doc_stamp

rtf: .doc_stamp

docs: html

.PHONY: stats tagstable tagrelease listfixmes listhacks check docs pdf html rtf
