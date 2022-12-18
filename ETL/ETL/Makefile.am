MAINTAINERCLEANFILES = \
	Makefile.in

CLEANFILES = \
	$(top_builddir)/ETL/etl_profile.h


etldir = $(etlincludedir)/ETL

etl_HEADERS = \
	ref_count \
	_ref_count.h \
	handle \
	stringf \
	_stringf.h \
	_handle.h \
	$(top_builddir)/ETL/etl_profile.h


$(top_builddir)/ETL/etl_profile.h:$(top_builddir)/ETL/etl_profile_.h
	sed "s/PACKAGE/ETL/g;" < $(top_builddir)/ETL/etl_profile_.h > $(top_builddir)/ETL/etl_profile.h

