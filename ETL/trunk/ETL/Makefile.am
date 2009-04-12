# $Id$

MAINTAINERCLEANFILES = \
	Makefile.in

CLEANFILES = \
	$(top_builddir)/ETL/etl_profile.h


etldir = $(includedir)/ETL

etl_HEADERS = \
	value \
	_value.h \
	rect \
	_rect.h \
	misc \
	_misc.h \
	ref_count \
	_ref_count.h \
	angle \
	fastangle \
	handle \
	ipc \
	thread \
	fixed \
	random \
	clock \
	hermite \
	calculus \
	stringf \
	trivial \
	spline \
	_stringf.h \
	_bspline.h \
	_calculus.h \
	_mutex_null.h \
	_hermite.h \
	_curve_func.h \
	_clock_base.h \
	_clock_system.h \
	_clock_gettimeofday.h \
	_random.h \
	_angle.h \
	_fastangle.h \
	_curve.h \
	_handle.h \
	_thread.h \
	_mutex_pthreads.h \
	_mutex_win32.h \
	_condition.h \
	_rwlock.h \
	smach \
	_smach.h \
	_trivial.h \
	_fixed.h \
	etl_config.h \
	$(top_builddir)/ETL/etl_profile.h \
	_fastangle_tables.h \
	bezier \
	_bezier.h \
	_bezier_angle.h \
	_status.h \
	status \
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
	_clock_win32hpcount.h \
	mutex \
	_mutex_pthreads_simple.h


$(top_builddir)/ETL/etl_profile.h:$(top_builddir)/ETL/etl_profile_.h
	sed "s/PACKAGE/ETL/g;" < $(top_builddir)/ETL/etl_profile_.h > $(top_builddir)/ETL/etl_profile.h


# FIXME: figure out why this is in SVN, but isn't installed
EXTRA_DIST = \
	_bit_rotate.h
