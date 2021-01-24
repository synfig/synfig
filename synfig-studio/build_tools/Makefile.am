dist_noinst_SCRIPTS = autorevision.sh sif2ico.sh

if DEVELOPMENT_SNAPSHOT
all-local:
	sh $(srcdir)/autorevision.sh "$(top_srcdir)" "$(top_builddir)"
else
all-local:
	-rm -f $(top_builddir)/autorevision.h
	touch $(top_builddir)/autorevision.h
endif

.PHONY: all-local

clean-local:
	-rm -f $(top_builddir)/autorevision.h
