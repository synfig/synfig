# $Id$

MAINTAINERCLEANFILES = \
	Makefile.in

CLEANFILES = \
	$(top_builddir)/ETL/etl_profile.h


etldir = $(etlincludedir)/ETL

etl_HEADERS = \
	rect \
	_rect.h \
	misc \
	_misc.h \
	ref_count \
	_ref_count.h \
	angle \
	handle \
	fixed \
	clock \
	hermite \
	calculus \
	stringf \
	trivial \
	spline \
	_stringf.h \
	_bspline.h \
	_calculus.h \
	_hermite.h \
	_curve_func.h \
	_clock_base.h \
	_clock_system.h \
	_clock_gettimeofday.h \
	_angle.h \
	_handle.h \
	_trivial.h \
	_fixed.h \
	etl_config.h \
	$(top_builddir)/ETL/etl_profile.h \
	bezier \
	_bezier.h \
	_bezier_angle.h \
	pen \
	_pen.h \
	boxblur \
	_boxblur.h \
	gaussian \
	_gaussian.h \
	surface \
	_surface.h \
	smart_ptr \
	_smart_ptr.h \
	_clock_win32hpcount.h


$(top_builddir)/ETL/etl_profile.h:$(top_builddir)/ETL/etl_profile_.h
	sed "s/PACKAGE/ETL/g;" < $(top_builddir)/ETL/etl_profile_.h > $(top_builddir)/ETL/etl_profile.h

