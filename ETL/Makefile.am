# $Id$

MAINTAINERCLEANFILES = \
	COPYING \
	INSTALL \
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
	.DS_Store \
	include/etl_profile.h.in \
	doxygen.cfg


SUBDIRS = \
	ETL \
	test


# Install the pkg-config file:
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ETL.pc

#Install the -config/m4 alternative to pkg-config:
bin_SCRIPTS = ETL-config

aclocaldir = $(prefix)/share/aclocal
ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = \
	COPYING \
	m4/subs.m4 \
	config/depcomp \
	m4/cxx_macros.m4 \
	ETL-config.in \
	m4/ETL.m4 \
	doxygen.cfg.in \
	doxygen.cfg \
	ETL.pc.in


SVN=svn
TAG=@PACKAGE_TARNAME@_@VERSION_MAJ@_@VERSION_MIN@_@VERSION_REV@

GREP=grep

PRINTF=printf

SH=sh

DOXYGEN=doxygen

stats:
	-@echo
	-@echo  -- Stats
	-@echo
	-@$(PRINTF) "Total lines: "
	-@wc -l `(find $(top_srcdir)/ETL -name '*.[ch]*')` | $(GREP) total
	-@$(PRINTF) "Total size: "
	-@du -hcs `find $(top_srcdir)/ETL -name '*.[ch]*'` | $(GREP) total
	-@echo 

ChangeLog:
	../autobuild/git2cl > ChangeLog

listfixmes:
	-@echo
	-@echo  -- List of pending FIXMEs
	-@echo
	-@$(GREP) FIXME -n `(find $(top_srcdir) -name '*.[ch]*')`
	-@echo

listhacks:
	-@echo
	-@echo  -- List of pending HACKs
	-@echo
	-@$(GREP) HACK -n `(find $(top_srcdir) -name '*.[ch]*')`
	-@echo

run: check

.doc_stamp: doxygen.cfg
	$(DOXYGEN) doxygen.cfg
	touch .doc_stamp

html: .doc_stamp

rtf: .doc_stamp

docs: html

.PHONY: stats listfixmes listhacks check docs pdf html rtf
