# $Id$

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = \
	CD_concept
	
brushesdir = ${datadir}/synfig/brushes
brushes_DATA = \
	CD_concept

CD_concept: mkdirs
	cp -rf CD_concept ${brushesdir}
	
mkdirs:
	mkdir -p ${brushesdir}

.PHONY: CD_concept
