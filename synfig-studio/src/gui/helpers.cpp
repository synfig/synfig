/* === S Y N F I G ========================================================= */
/*!	\file helpers.cpp
**	\brief Helpers File
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <gtk/gtk.h>
#include <glibmm/main.h>

#include <synfig/general.h>

#include "helpers.h"

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

static bool
is_old_gtk_adjustment() {
	static bool is_old = gtk_check_version(3, 18, 0) != NULL;
	return is_old;
}

/* === M E T H O D S ======================================================= */

AdjustmentGroup::AdjustmentGroup():
	lock() { }

AdjustmentGroup::~AdjustmentGroup()
{
	for(std::list<Item>::iterator i = items.begin(); i != items.end(); ++i) {
		i->connection_changed.disconnect();
		i->connection_value_changed.disconnect();
	}
	connection_timeout.disconnect();
}

void AdjustmentGroup::add(Glib::RefPtr<Gtk::Adjustment> adjustment)
{
	for(std::list<Item>::iterator i = items.begin(); i != items.end(); ++i)
		if (i->adjustment == adjustment) return;

	items.push_back(Item());
	Item &item = items.back();

	item.adjustment = adjustment;
	item.connection_changed = item.adjustment->signal_changed().connect(
		sigc::bind( sigc::mem_fun(this, &AdjustmentGroup::changed), adjustment ) );
	item.connection_value_changed = item.adjustment->signal_value_changed().connect(
		sigc::bind( sigc::mem_fun(this, &AdjustmentGroup::changed), adjustment ) );

	changed( adjustment );
}

void AdjustmentGroup::remove(Glib::RefPtr<Gtk::Adjustment> adjustment)
{
	bool found = false;
	for(std::list<Item>::iterator i = items.begin(); i != items.end(); )
		if (i->adjustment == adjustment) {
			i->connection_changed.disconnect();
			i->connection_value_changed.disconnect();
			i = items.erase(i);
			found = true;
		} else ++i;
	if (found) changed(adjustment);
}
  
void
AdjustmentGroup::changed(Glib::RefPtr<Gtk::Adjustment> adjustment)
{
	if (lock || items.empty()) return;

	double position = items.front().adjustment->get_value();

	double maxSize = 0;
	for(std::list<Item>::iterator i = items.begin(); i != items.end(); ++i) {
		if (i->adjustment == adjustment) {
			i->origSize = i->adjustment->get_upper()
			            - i->adjustment->get_page_size();
			position = i->adjustment->get_value();
		}
		maxSize = std::max(maxSize, i->origSize);
	}

	connection_timeout.disconnect();
	connection_timeout = Glib::signal_timeout().connect(
		sigc::bind_return(
			sigc::bind( sigc::mem_fun(this, &AdjustmentGroup::set), position, maxSize ),
			false ),
		0 );
}

void
AdjustmentGroup::set(double position, double size)
{
	BoolLock boollock(lock);
	connection_timeout.disconnect();
	for(std::list<Item>::iterator i = items.begin(); i != items.end(); ++i) {
		double value = i->adjustment->get_value();
		double page = i->adjustment->get_page_size();
		double upper = i->adjustment->get_upper();
		double newUpper = size + page;

		if (fabs(newUpper - upper) > 0.1)
			i->adjustment->set_upper(newUpper);
		if (fabs(position - value) > 0.1)
			i->adjustment->set_value(position);
	}
};


void
ConfigureAdjustment::emit_changed()
	{ if (is_old_gtk_adjustment()) adjustment->changed(); }

void
ConfigureAdjustment::emit_value_changed()
{ if (is_old_gtk_adjustment()) adjustment->value_changed(); }
