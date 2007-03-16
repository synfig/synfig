#Author: Jaime Irving Davila
#Date: 23/09/2001
#Modified on 15/04/2002
#Given to the public domain

#Variables useful for the customization of the makefile

#Name of the document (without the sgml extension)

DOC = synfig-studio

#Name of the file used for the index (without the sgml extension)
INDEX = index

#Name of the compiler of the sgml, could be jade, openjade, ..
COMPILER = openjade

#Location of the stylesheet for the html output

DBSTYLESHEET = /usr/share/sgml/docbook/stylesheet/dsssl/ldp/ldp.dsl

#Options for generating multiple HTML output
MULTIPLEHTMLOPT = -t sgml -ihtml -d${DBSTYLESHEET}\#html 

#Name of the directory for the source sgml
SGMLDIR = en

#Name of the directory for the "others" files
OTHERSDIR = ${SGMLDIR}/others

#Name of the directory for the png files
PNGDIR = png

RESULTDIR = result/${SGMLDIR}

#Name of the directory for the output of the multiple html
MULTIPLEHTMLDIR = ${RESULTDIR}/out-htmls

#Name of the directory for the output the ps file
PSDIR = ${RESULTDIR}/out-ps

#Name of the script which generates the ps file, in RedHat it's db2ps
SCRIPTPS = db2ps

#Name of the directory for the temp and tmp-output the pdf file
PDFDIR = ${RESULTDIR}/tmp-pdf
#Name of the script which generates the ps file, in RedHat it's db2ps
SCRIPTPDF = db2pdf

#Variables needed by the rules, should NOT be changed
#Variable for including the sgml source file
VPATH = ${SGMLDIR}

#Variable for knowing if there exists the sgml index file, in case it
#NOT exists its value is empty, otherwise it's the name of the index
ISINDEX = ${findstring ${INDEX}, ${wildcard ${SGMLDIR}/*.sgml}}

#Variable for knowing if the tag init will be needed
ifneq (${ISINDEX}, ${INDEX})
	USEINIT = init
endif

#Variable for storing the name of all the sgml source files directory
SRCFILES = $(patsubst ${SGMLDIR}/${INDEX}.sgml,,$(wildcard ${SGMLDIR}/*))

#Variable for storing the name of all the files in the "others" directory
OTHERFILES = $(wildcard ${OTHERSDIR}/*)

#Variable for storing the names of the "others" files placed in the

#Variable for storing the names of the "others" files placed in the
#multiple-html directory
OTHERMULTIPLE = $(patsubst ${SGMLDIR}/others/%, ${MULTIPLEHTMLDIR}/%, ${OTHERFILES} )

#Variable for storing the names of the png's pictures
PNGFILES = $(wildcard ${PNGDIR}/*.png)

#Variable for storing the names of the png's files placed on the

#Variable for storing the names of the png's files placed on the
#single-html directory
PNGMULTIPLE = $(patsubst ${PNGDIR}/%, ${MULTIPLEHTMLDIR}/%, ${PNGFILES})

#Variable for storing the names of the eps files
FIGEPS = $(patsubst ${PNGDIR}/%.png, ${PSDIR}/%.eps, ${PNGFILES})

PATH_LANG = ${CURDIR}/${SGMLDIR}

all: multiple-html ps pdf

.PHONY: all clean init cpfilessingle multiple-html cpfilesmultiple distsource ps

#Rules needed for generating multiple html files

multiple-html: ${MULTIPLEHTMLDIR}/${DOC}.html cpfilesmultiple

${MULTIPLEHTMLDIR}/${DOC}.html: ${MULTIPLEHTMLDIR} ${INDEX}.sgml.m ${SGMLDIR}/${DOC}.tmp.sgml
	#cp -f ${SGMLDIR}/${INDEX}.sgml.m ${SGMLDIR}/${INDEX}.sgml
	${COMPILER} ${MULTIPLEHTMLOPT} ${SGMLDIR}/${DOC}.tmp.sgml
	mv *.html ${MULTIPLEHTMLDIR}

${SGMLDIR}/${DOC}.tmp.sgml: ${SGMLDIR}/${DOC}.sgml
	sed "s:##PATHLANG##:${PATH_LANG}:" ${SGMLDIR}/${DOC}.sgml > ${SGMLDIR}/${DOC}.tmp.sgml

HTML.index.m: ${DOC}.sgml ${SRCFILES} ${USEINIT} ${SGMLDIR}/${DOC}.tmp.sgml
	${COMPILER} ${MULTIPLEHTMLOPT} -V html-index ${SGMLDIR}/${DOC}.tmp.sgml > /dev/null
	mv HTML.index ${SGMLDIR}/HTML.index.m
	rm *.html

${INDEX}.sgml.m: HTML.index.m
	perl -S collateindex.pl -o ${SGMLDIR}/${INDEX}.sgml.m ${SGMLDIR}/HTML.index.m

${MULTIPLEHTMLDIR}:
	mkdir -p ${MULTIPLEHTMLDIR}

cpfilesmultiple: ${PNGMULTIPLE} ${OTHERMULTIPLE}

${MULTIPLEHTMLDIR}/%.png: ${PNGDIR}/%.png
	cp -rf $< $@

${MULTIPLEHTMLDIR}/%: ${OTHERSDIR}/%
	cp -rf $< $@

#Rules needed for generating the ps file

ps: ${PSDIR}/${DOC}.ps

${PSDIR}/${DOC}.ps: ${PSDIR} ${FIGEPS} ${INDEX}.sgml.m
	cp -rf ${SGMLDIR}/*.sgml ${PSDIR}
	cd ${PSDIR}; ${SCRIPTPS} ${DOC}.tmp.sgml ; mv ${DOC}.tmp.ps ${DOC}.ps
	cd ..

${PSDIR}/%.eps: ${PNGDIR}/%.png ${PSDIR}
	convert $< $@

${PSDIR}:
	mkdir -p ${PSDIR}

#Rules needed for generating the pdf file

pdf: ${PDFDIR}/${DOC}.pdf

${PDFDIR}/${DOC}.pdf: ${PDFDIR} ${INDEX}.sgml.m
	cp -rf ${PNGDIR}/* ${PDFDIR}
	cp -rf ${SGMLDIR}/*.sgml ${PDFDIR}
	cp -rf ${SGMLDIR}/* ${PDFDIR}
	mv ${PDFDIR}/${DOC}.tmp.sgml ${PDFDIR}/${DOC}.sgml
	cd ${PDFDIR};${SCRIPTPDF} ${DOC}.sgml
	cd ..

${PDFDIR}:
	mkdir -p ${PDFDIR}

#Some additional and standard tags
init:
	perl -S collateindex.pl -N -o ${SGMLDIR}/${INDEX}.sgml

distsource: clean
	tar cvfz ${DOC}.tar.gz *

clean:
	rm -rf ${MULTIPLEHTMLDIR} ${SGMLDIR}/*.index.* ${SGMLDIR}/${INDEX}.sgml* nil ${PSDIR} ${PDFDIR}



