/* === S Y N F I G ========================================================= */
/*!	\file helpers.h
**	\brief Helpers Header
**
**	$Id$
**
**	\legal
**	......... ... 2018 Ivan Mahonin
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_HELPERS_H
#define __SYNFIG_HELPERS_H

/* === H E A D E R S ======================================================= */

#include <cassert>

#include <gtkmm/adjustment.h>

#include <ETL/handle>
#include <synfig/real.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {


class BoolLock {
private:
	bool &lock;
public:
	explicit BoolLock(bool &lock): lock(lock) { assert(!lock); lock = true; }
	~BoolLock() { lock = false; }
};


//! Temoraly freezes all notifications from Glib object
//! All notifications will raised immediately when FreezeNotify is destroyed
//! see: Glib::ObjectBase::freeze_notify(), Glib::ObjectBase::thaw_notify()
class FreezeNotify {
private:
	Glib::ObjectBase *obj;
	Glib::RefPtr<Glib::ObjectBase> obj_ref;

	// disable copying
	FreezeNotify(const FreezeNotify&): obj() { }
	FreezeNotify& operator=(const FreezeNotify&) { return *this; }
public:
	explicit FreezeNotify(Glib::ObjectBase &obj):
		obj(&obj)
		{ if (this->obj) this->obj->freeze_notify(); }
	explicit FreezeNotify(Glib::ObjectBase *obj):
		obj(obj)
		{ if (this->obj) this->obj->freeze_notify(); }
	explicit FreezeNotify(const Glib::RefPtr<Glib::ObjectBase> &obj_ref):
		obj(), obj_ref(obj_ref)
		{ if (this->obj_ref) this->obj_ref->freeze_notify(); }
	~FreezeNotify() {
		if (obj) obj->thaw_notify();
		if (obj_ref) obj_ref->thaw_notify();
	}
};


class AdjustmentGroup: public etl::shared_object {
public:
	typedef etl::handle<AdjustmentGroup> Handle;

	struct Item {
		Glib::RefPtr<Gtk::Adjustment> adjustment;
		double origSize;
		sigc::connection connection_changed;
		sigc::connection connection_value_changed;

		Item(): origSize() { }
	};
	typedef std::list<Item> List;

private:
	List items;
	bool lock;
	sigc::connection connection_timeout;

	void changed(Glib::RefPtr<Gtk::Adjustment> adjustment);
	void set(double position, double size);

public:
	AdjustmentGroup();
	~AdjustmentGroup();

	void add(Glib::RefPtr<Gtk::Adjustment> adjustment);
	void remove(Glib::RefPtr<Gtk::Adjustment> adjustment);
};


class ConfigureAdjustment {
public:
	Glib::RefPtr<Gtk::Adjustment> adjustment;
	double value;
	double lower;
	double upper;
	double step_increment;
	double page_increment;
	double page_size;

	double precision;

private:
	// for compatibility with old gtk
	void emit_changed();
	void emit_value_changed();

public:
	explicit ConfigureAdjustment(const Glib::RefPtr<Gtk::Adjustment> &adjustment = Glib::RefPtr<Gtk::Adjustment>()):
		adjustment(adjustment),
		value(),
		lower(),
		upper(),
		step_increment(),
		page_increment(),
		page_size(),
		precision()
		{ reset(); }

	~ConfigureAdjustment()
		{ cancel(); }

	bool is_equal(double a, double b)
		{ return fabs(b-a) <= precision; }

	ConfigureAdjustment& reset(const Glib::RefPtr<Gtk::Adjustment> &adjustment, double precision) {
		set_adjustment(adjustment);
		if (this->adjustment) {
			value          = this->adjustment->get_value();
			lower          = this->adjustment->get_lower();
			upper          = this->adjustment->get_upper();
			step_increment = this->adjustment->get_step_increment();
			page_increment = this->adjustment->get_page_increment();
			page_size      = this->adjustment->get_page_size();
		} else {
			value          = 0.0;
			lower          = 0.0;
			upper          = 0.0;
			step_increment = 0.0;
			page_increment = 0.0;
			page_size      = 0.0;
		}
		this->precision = precision;
		return *this;
	}
	ConfigureAdjustment& reset(double precision)
		{ return reset(adjustment, precision); }
	ConfigureAdjustment& reset()
		{ return reset(precision); }

	ConfigureAdjustment& set_adjustment(const Glib::RefPtr<Gtk::Adjustment> &x)
		{ if (&x != &adjustment) adjustment = x; return *this; }
	ConfigureAdjustment& set_precision(double x)
		{ precision = x; return *this; }
	ConfigureAdjustment& set_precision_high()
		{ precision = synfig::real_high_precision<double>(); return *this; }
	ConfigureAdjustment& set_precision_normal()
		{ precision = synfig::real_precision<double>(); return *this; }
	ConfigureAdjustment& set_precision_low()
		{ precision = synfig::real_low_precision<double>(); return *this; }

	ConfigureAdjustment& set_value(double x)
		{ value = x; return *this; }
	ConfigureAdjustment& set_lower(double x)
		{ lower = x; return *this; }
	ConfigureAdjustment& set_upper(double x)
		{ upper = x; return *this; }
	ConfigureAdjustment& set_step_increment(double x)
		{ step_increment = x; return *this; }
	ConfigureAdjustment& set_page_increment(double x)
		{ page_increment = x; return *this; }
	ConfigureAdjustment& set_page_size(double x)
		{ page_size = x; return *this; }

	ConfigureAdjustment& adj_value(double x)
		{ value += x; return *this; }
	ConfigureAdjustment& adj_value_step(double x)
		{ value += x*step_increment; return *this; }
	ConfigureAdjustment& adj_value_page(double x)
		{ value += x*page_increment; return *this; }

	void finish() {
		assert(adjustment);
		if (!adjustment) return;
		double value = std::max(lower, std::min(upper - page_size, this->value));
		if ( !is_equal(lower,          adjustment->get_lower())
		  || !is_equal(upper,          adjustment->get_upper())
		  || !is_equal(step_increment, adjustment->get_step_increment())
		  || !is_equal(page_increment, adjustment->get_page_increment())
		  || !is_equal(page_size,      adjustment->get_page_size()) )
		{
			adjustment->configure(value, lower, upper, step_increment, page_increment, page_size);
			emit_changed();
		} else
		if (!is_equal(value, adjustment->get_value())) {
			adjustment->set_value(value);
			emit_value_changed();
		}
		adjustment.reset();
	}

	void cancel()
		{ adjustment.reset(); }
};

inline void configure_adjustment(
	const Glib::RefPtr<Gtk::Adjustment> &adjustment,
	double value,
	double lower,
	double upper,
	double step_increment,
	double page_increment,
	double page_size,
	double precision = 0.0 )
{
	ConfigureAdjustment(adjustment)
		.set_value(value)
		.set_lower(lower)
		.set_upper(upper)
		.set_step_increment(step_increment)
		.set_page_increment(page_increment)
		.set_page_size(page_size)
		.set_precision(precision)
		.finish();
}


}; // END of namespace studio

/* === E N D =============================================================== */

#endif
