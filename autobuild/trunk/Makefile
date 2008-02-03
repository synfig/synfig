
VERSION=0.61.03

COREDIR=synfig-core
STUDIODIR=synfig-studio
ETLDIR=etl

OUTPUTDIR=output

SRCPKG=Synfig.mpkg
OUTPKG=$(OUTPUTDIR)/Synfig-$(VERSION).mpkg
PKGDIR=$(OUTPKG)/Contents/Packages

SSH_USER=darco@www.bridgetone.com
SSH_DIR=~/voria/files

all: src osx package

publish:
	cd $(OUTPUTDIR) && tar c *.gz *.zip *.exe *.bz2 | ssh $(SSH_USER) "cd $(SSH_DIR) && tar xv" 

osx-pkg:
	rm -fr $(OUTPKG)
	rm -fr $(OUTPKG).zip
	cp -R $(SRCPKG) $(OUTPKG)
	cp -r $(OUTPUTDIR)/*.pkg $(PKGDIR)
	cd $(OUTPUTDIR) && zip -r Synfig-$(VERSION).mpkg.zip *.mpkg
	
core-osx:
	[ -d $(OUTPUTDIR) ] || mkdir $(OUTPUTDIR)
	rm -fr $(COREDIR)
	svn export https://svn.voria.com/code/synfig-core/trunk $(COREDIR) || exit 2
	cd $(COREDIR) && ./macosxbuild.sh
	cp -r $(COREDIR)/macosxbuild/*.pkg $(OUTPUTDIR)

studio-osx:
	[ -d $(OUTPUTDIR) ] || mkdir $(OUTPUTDIR)
	rm -fr $(STUDIODIR)
	svn export https://svn.voria.com/code/synfig-studio/trunk $(STUDIODIR) || exit 2
	cd $(STUDIODIR) && ./macosxbuild.sh
	cp -r $(STUDIODIR)/macosxbuild/*.pkg $(OUTPUTDIR)

core-w32:
	[ -d $(OUTPUTDIR) ] || mkdir $(OUTPUTDIR)
	rm -fr $(COREDIR)
	svn export https://svn.voria.com/code/synfig-core/trunk $(COREDIR) || exit 2
	cd $(COREDIR) && ./win32build.sh
	cp -r $(COREDIR)/win32build/*.exe $(OUTPUTDIR)

studio-w32:
	[ -d $(OUTPUTDIR) ] || mkdir $(OUTPUTDIR)
	rm -fr $(STUDIODIR)
	svn export https://svn.voria.com/code/synfig-studio/trunk $(STUDIODIR) || exit 2
	cd $(STUDIODIR) && ./win32build.sh
	cp -r $(STUDIODIR)/win32build/*.exe $(OUTPUTDIR)

core-src:
	[ -d $(OUTPUTDIR) ] || mkdir $(OUTPUTDIR)
	rm -fr $(COREDIR)
	svn export https://svn.voria.com/code/synfig-core/trunk $(COREDIR) || exit 2
	cd $(COREDIR) && libtoolize --ltdl --copy --force && autoreconf --force --install && ./configure
	make -C $(COREDIR) dist
	cp -r $(COREDIR)/*.gz $(OUTPUTDIR)

studio-src:
	[ -d $(OUTPUTDIR) ] || mkdir $(OUTPUTDIR)
	rm -fr $(STUDIODIR)
	svn export https://svn.voria.com/code/synfig-studio/trunk $(STUDIODIR) || exit 2
	cd $(STUDIODIR) && autoreconf --force --install && ./configure
	make -C $(STUDIODIR) dist
	cp -r $(STUDIODIR)/*.gz $(OUTPUTDIR)

etl-src:
	[ -d $(OUTPUTDIR) ] || mkdir $(OUTPUTDIR)
	rm -fr $(ETLDIR)
	svn export https://svn.voria.com/code/ETL/trunk $(ETLDIR) || exit 2
	cd $(ETLDIR) && autoreconf --force --install && ./configure
	make -C $(ETLDIR) dist
	cp -r $(ETLDIR)/*.gz $(OUTPUTDIR)

src: etl-src core-src studio-src

osx: core-osx studio-osx

w32: core-w32 studio-w32

clean:
	rm -fr $(COREDIR)
	rm -fr $(STUDIODIR)
	rm -fr $(ETLDIR)
	rm -fr $(OUTPUTDIR)

