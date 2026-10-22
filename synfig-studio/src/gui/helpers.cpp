/* === S Y N F I G ========================================================= */
/*!	\file helpers.cpp
**	\brief Helpers File
**
**	\legal
**	......... ... 2018 Ivan Mahonin
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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

#include "helpers.h"

#include <glibmm/main.h>
#include <gtkmm/action.h>
#include <gtkmm/bin.h>
#include <gtkmm/widget.h>
#include <gtkmm/accelmap.h>
#include <gtkmm/tooltip.h>
#include <gtk/gtk.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

static bool
is_old_gtk_adjustment() {
	static bool is_old = gtk_check_version(3, 18, 0) != nullptr;
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

void
studio::setup_tooltip_with_accel(Gtk::Widget* widget, const std::function<std::string()>& get_base_tooltip, const std::string& accel_path)
{
	if (!widget) return;
	auto handler = [get_base_tooltip, accel_path](int, int, bool, const Glib::RefPtr<Gtk::Tooltip>& tooltip) -> bool {
		std::string text = get_base_tooltip ? get_base_tooltip() : "";
		Gtk::AccelKey key;
		if (!accel_path.empty() && Gtk::AccelMap::lookup_entry(accel_path, key) && key.get_key() != 0) {
			gchar* accel_text = gtk_accelerator_get_label(key.get_key(), (GdkModifierType)key.get_mod());
			if (accel_text) {
				if (*accel_text) {
					text += " (";
					text += accel_text;
					text += ")";
				}
				g_free(accel_text);
			}
		}
		if (!text.empty()) {
			tooltip->set_text(text);
			return true;
		}
		return false;
	};

	widget->property_has_tooltip() = true;
	widget->signal_query_tooltip().connect(handler);

	if (Gtk::Bin* bin = dynamic_cast<Gtk::Bin*>(widget)) {
		if (Gtk::Widget* child = bin->get_child()) {
			child->property_has_tooltip() = true;
			child->signal_query_tooltip().connect(handler);
		}
	}
}

void
studio::setup_tooltip_with_accel(Gtk::Widget* widget, const std::string& base_tooltip, const std::string& accel_path)
{
	setup_tooltip_with_accel(widget, [base_tooltip]() { return base_tooltip; }, accel_path);
}

void
studio::setup_tooltip_with_accel(Gtk::Widget* widget, const Glib::RefPtr<Gtk::Action>& action)
{
	if (!widget || !action) return;
	std::string accel_path = action->get_accel_path();
	if (accel_path.empty()) {
		const std::string name = action->get_name();
		Gtk::AccelKey key;
		const std::string candidates[] = {
			"<Actions>/action_group_dock_history/" + name,
			"<Actions>/canvasview/" + name,
			"<Actions>/mainwindow/" + name,
			"<Actions>/action_group_layer_action_manager/" + name,
			"<Actions>/action_group_state_manager/" + name
		};
		for (const auto& c : candidates) {
			if (Gtk::AccelMap::lookup_entry(c, key) && key.get_key() != 0) {
				accel_path = c;
				break;
			}
		}
	}
	setup_tooltip_with_accel(widget, [action]() { return action->get_tooltip(); }, accel_path);
}
