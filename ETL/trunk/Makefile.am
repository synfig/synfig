# $Id$

MAINTAINERCLEANFILES=config/config.guess config/config.sub config/ltmain.sh config/install-sh config/mkinstalldirs config/aclocal.m4 config/missing config/texinfo.tex config/depcomp aclocal.m4 config.h.in configure stamp-h.in Makefile.in config.log config.status .doc_stamp .DS_Store @PACKAGE@.spec include/etl_profile.h.in ETL.pc.in doxygen.cfg

SUBDIRS=ETL test

# Install the pkg-config file:
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ETL.pc

#Install the -config/m4 alternative to pkg-config:
bin_SCRIPTS=ETL-config
aclocaldir = $(prefix)/share/aclocal


EXTRA_DIST=COPYING bootstrap config/subs.m4 config/depcomp config/cxx_macros.m4 ETL.pbproj/project.pbxproj ETL.pbproj/etl_profile.h ETL.pbproj/frameworkfix.cpp @PACKAGE_TARNAME@.spec config/project.spec.in config/build.cfg ETL-config.in config/ETL.m4 config/doxygen.cfg.in doxygen.cfg config/pkgconfig.pc.in @PACKAGE_TARNAME@.pc.in

CVS=cvs
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
	-@wc -l $(shell find $(top_srcdir)/ETL -name '*.[ch]*') | $(GREP) total
	-@$(PRINTF) "Total size: "
	-@du -hcs $(shell find $(top_srcdir)/ETL -name '*.[ch]*') | $(GREP) total
	-@echo 

tagstable:
	-$(SVN) delete $(SVN_REPOSITORY)/tags/stable -m "Stabe Tag: Removing old tag"
	$(SVN) mkdir $(SVN_REPOSITORY)/tags/stable -m "Stable Tag: Creating new directory"
	$(SVN) copy $(top_srcdir) $(SVN_REPOSITORY)/tags/stable -m "Stable Tag: Copying everyhting over"

tagrelease:
	$(SVN) mkdir $(SVN_REPOSITORY)/tags/$(TAG) -m "Release Tag: Making directory"
	$(SVN) copy $(top_srcdir) $(SVN_REPOSITORY)/tags/$(TAG) -m "Release $(TAG)"


listfixmes:
	-@echo
	-@echo  -- List of pending FIXMEs
	-@echo
	-@$(GREP) FIXME -n $(shell find $(top_srcdir) -name '*.[ch]*')
	-@echo

listhacks:
	-@echo
	-@echo  -- List of pending HACKs
	-@echo
	-@$(GREP) HACK -n $(shell find $(top_srcdir) -name '*.[ch]*')
	-@echo

run: check

$(srcdir)/configure:  $(srcdir)/configure.in $(ACLOCAL_M4) $(CONFIGURE_DEPENDENCIES)
	$(srcdir)/bootstrap

.doc_stamp: doxygen.cfg
	$(DOXYGEN) doxygen.cfg
	touch .doc_stamp

html: .doc_stamp

rtf: .doc_stamp

docs: html

.PHONY: stats tagstable tagrelease listfixmes listhacks check docs pdf html rtf
