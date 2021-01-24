# $Id$

MAINTAINERCLEANFILES = \
	COPYING \
	INSTALL \
	synfig.pc \
	synfig-$(VERSION).tar.gz \
	doxygen.cfg \
	config/config.guess \
	config/config.sub \
	config/ltmain.sh \
	config/ltmain.sh.orig \
	config/ltmain.sh.rej \
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
	.DS_Store \
	include/etl_profile.h.in \
	libltdl/COPYING.LIB \
	libltdl/Makefile.in \
	libltdl/acinclude.m4 \
	libltdl/config-h.in \
	libltdl/configure.in \
	libltdl/ltdl.h \
	libltdl/Makefile.am \
	libltdl/README \
	libltdl/aclocal.m4 \
	libltdl/configure \
	libltdl/ltdl.c \
	libltdl/stamp-h.in


SUBDIRS = \
	libltdl \
	build_tools \
	src \
	examples \
	po \
	test


# Install the pkg-config file:
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = synfig.pc

bin_SCRIPTS = synfig-config

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = \
	ChangeLog.old \
	TODO \
	m4/subs.m4 \
	config/package \
	doc/hw-gfx-mapping.txt \
	macosxbuild.sh \
	COPYING \
	config/depcomp \
	m4/synfig.m4 \
	m4/cxx_macros.m4 \
	synfig-config.in \
	m4/ETL.m4 \
	doxygen.cfg.in \
	doxygen.cfg \
	synfig.pc.in \
	m4/pkgconfig.m4 \
	m4/libxml++.m4 \
	@PACKAGE_TARNAME@.pc.in \
	pkg-info/macosx/core-resources/ReadMe.txt \
	pkg-info/macosx/core-resources/Welcome.txt \
	pkg-info/macosx/core-resources/install.sh


GREP=grep
PRINTF=printf
SH=sh
DOXYGEN=doxygen

SVN=svn
TAG=@PACKAGE_TARNAME@_@VERSION_MAJ@_@VERSION_MIN@_@VERSION_REV@

stats:
	-@echo
	-@echo  -- Stats
	-@echo
	-@$(PRINTF) "Total lines: "
	-@wc -l `shell find $(top_srcdir)/src -name '*.[ch]*'` | $(GREP) total
	-@$(PRINTF) "Total size: "
	-@du -hcs `shell find $(top_srcdir)/src -name '*.[ch]*'` | $(GREP) total
	-@echo 

ChangeLog:
	cd $(top_srcdir) && ../autobuild/git2cl > ChangeLog

listfixmes:
	-@echo
	-@echo  -- List of pending FIXMEs
	-@echo
	-@$(GREP) -e FIXME -e "\\fixme" -n `shell find $(top_srcdir) -name '*.[ch]*'`
	-@echo

listhacks:
	-@echo
	-@echo  -- List of pending HACKs
	-@echo
	-@$(GREP) -e HACK -e "\\hack" -n `shell find $(top_srcdir) -name '*.[ch]*'`
	-@echo

listtodos:
	-@echo
	-@echo  -- List of pending TODOs
	-@echo
	-@$(GREP) -e TODO -e "\\todo" -n `shell find $(top_srcdir) -name '*.[ch]*'`
	-@echo

listoptimizes:
	-@echo
	-@echo  -- List of pending optimizations
	-@echo
	-@$(GREP) -e OPTIMIZE -e "\\optimize" -n `shell find $(top_srcdir) -name '*.[ch]*'`
	-@echo

listwritemes:
	-@echo
	-@echo  -- List of pending WRITEMEs
	-@echo
	-@$(GREP) -e WRITEME -e "\\writeme" -n `shell find $(top_srcdir) -name '*.[ch]*'`
	-@echo

install-bin:
	make -C src/synfig install-libLTLIBRARIES

RMDIR=rm -fr

if MACOSX_PKG
package: all pkg-info/macosx/synfig-core.info
	[ -d pkg_root ] && $(RMDIR) pkg_root || true
	make install-strip prefix="`pwd`/pkg_root"
	$(srcdir)/config/package pkg_root pkg-info/macosx/synfig-core.info -r $(srcdir)/pkg-info/macosx/core-resources
endif

run: check

.doc_stamp: doxygen.cfg
	$(DOXYGEN) doxygen.cfg
	touch .doc_stamp

benchmark:
	synfig -b -t null -q --time 0 $(srcdir)/examples/*.sif

html: .doc_stamp

rtf: .doc_stamp

docs: html

.PHONY: stats listfixmes listhacks check docs pdf html rtf
