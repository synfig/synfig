MAINTAINERCLEANFILES = \
	Makefile.in

CLEANFILES = \
	$(top_builddir)/ETL/etl_profile.h


etldir = $(etlincludedir)/ETL

etl_HEADERS = \
	misc \
	_misc.h \
	ref_count \
	_ref_count.h \
	handle \
	hermite \
	stringf \
	_stringf.h \
	_hermite.h \
	_curve_func.h \
	_handle.h \
	$(top_builddir)/ETL/etl_profile.h \
	bezier \
	_bezier.h \
	pen \
	_pen.h \
	boxblur \
	_boxblur.h \
	gaussian \
	_gaussian.h \
	surface \
	_surface.h


$(top_builddir)/ETL/etl_profile.h:$(top_builddir)/ETL/etl_profile_.h
	sed "s/PACKAGE/ETL/g;" < $(top_builddir)/ETL/etl_profile_.h > $(top_builddir)/ETL/etl_profile.h

