/* === S Y N F I G ========================================================= */
/*!	\file dockable.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include <synfig/general.h>

#include "app.h"

#include "docks/dockable.h"
#include "docks/dockmanager.h"
#include "docks/dockbook.h"
#include "docks/dockdialog.h"

#if GTK_CHECK_VERSION (3,20,0)
#include <gdkmm/seat.h>
#else
#include <gdkmm/devicemanager.h>
#endif

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#ifndef IMAGE_EXT
#	define IMAGE_EXT	"png"
#endif

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Dockable::Dockable(const synfig::String& name, const synfig::String& local_name, Gtk::StockID stock_id):
	name_(name),
	local_name_(local_name),
	stock_id_(stock_id),
	use_scrolled(true),
	container(),
	toolbar_container(),
	dnd_success_()
{
	clear();
	set_size_request(175, 120);
	show();
}

Dockable::~Dockable()
{ }

bool
Dockable::get_use_scrolled() const
	{ return use_scrolled; }

void
Dockable::set_use_scrolled(bool x) {
	use_scrolled = x;
	if (!container) return;
	Gtk::PolicyType policy = use_scrolled ? Gtk::POLICY_AUTOMATIC : Gtk::POLICY_NEVER;
	container->set_policy(policy, policy);
}

void
Dockable::on_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int, int, const Gtk::SelectionData& selection_data, guint, guint time)
{
	if (selection_data.get_length() >= 0
	 && selection_data.get_format() == 8
	 && selection_data.get_data_type() == "SYNFIG_DOCK")
	{
		Dockable& dockable(**reinterpret_cast<Dockable**>(const_cast<guint8*>(selection_data.get_data())));
		DockBook *parent = dynamic_cast<DockBook*>(get_parent());
		DockBook *dockable_parent = dynamic_cast<DockBook*>(dockable.get_parent());
		if (parent) {
			if (dockable_parent != parent)
				parent->add(dockable, parent->page_num(*this));
			else
				parent->reorder_child(dockable, parent->page_num(*this));
			dockable.present();
			context->drag_finish(true, false, time);
			App::dock_manager->update_window_titles();
			return;
		}
	}
	context->drag_finish(false, false, time);
}

void
Dockable::on_drag_end(const Glib::RefPtr<Gdk::DragContext>&/*context*/)
{
	if (!dnd_success_) {
		detach_to_pointer();
	}
	App::dock_manager->set_dock_area_visibility(false, nullptr);
}

void
Dockable::on_drag_begin(const Glib::RefPtr<Gdk::DragContext>&/*context*/)
{
	dnd_success_ = false;
	App::dock_manager->set_dock_area_visibility(true, dynamic_cast<DockBook*>(get_parent()));
}

void
Dockable::on_drag_data_get(const Glib::RefPtr<Gdk::DragContext>&, Gtk::SelectionData& selection_data, guint /*info*/, guint /*time*/)
{
	Dockable* tmp(this);
	dnd_success_ = true;
	selection_data.set(8, reinterpret_cast<const guchar*>(&tmp), sizeof(Dockable**));
}

void
Dockable::set_local_name(const synfig::String& local_name)
{
	local_name_ = local_name;
	signal_stock_id_changed()();
}

void
Dockable::attach_dnd_to(Gtk::Widget& widget)
{
	std::vector<Gtk::TargetEntry> listTargets;
	listTargets.push_back( Gtk::TargetEntry("SYNFIG_DOCK") );
	Gtk::StockItem stock_item;

	widget.drag_source_set(listTargets);
	if (Gtk::Stock::lookup(get_stock_id(), stock_item))
		widget.drag_source_set_icon(get_stock_id());
	widget.drag_dest_set(listTargets);
	widget.signal_drag_data_get().connect( sigc::mem_fun(*this, &Dockable::on_drag_data_get ));
	widget.signal_drag_end().connect( sigc::mem_fun(*this, &Dockable::on_drag_end ));
	widget.signal_drag_begin().connect( sigc::mem_fun(*this, &Dockable::on_drag_begin ));
	widget.signal_drag_data_received().connect( sigc::mem_fun(*this, &Dockable::on_drag_data_received ));
}

void Dockable::detach()
{
	DockManager::remove_widget_recursive(*this);
	present();
}

void Dockable::detach_to_pointer()
{
	Glib::RefPtr<Gdk::Device> mouse_device;
#if GTK_CHECK_VERSION (3,20,0)
	Glib::RefPtr<Gdk::Seat> seat = get_display()->get_default_seat();
	mouse_device = seat->get_pointer();
#else
	Glib::RefPtr<Gdk::DeviceManager> dev_manager = get_display()->get_device_manager();
	dev_manager->get_client_pointer();
#endif
	int x, y;
	mouse_device->get_position(x, y);

	detach();

	get_window()->move(x, y);
}

void
Dockable::add(Gtk::Widget& x)
{
	reset_container();
	x.set_hexpand();
	x.set_vexpand();
	x.show();
	container->add(x);
}

void
Dockable::set_toolbar(Gtk::Toolbar& toolbar)
{
	reset_toolbar();
	toolbar.set_icon_size(Gtk::IconSize(1) /*GTK::ICON_SIZE_MENU*/);
	toolbar.set_toolbar_style(Gtk::TOOLBAR_ICONS);
	toolbar.set_hexpand(true);
	toolbar.set_vexpand(false);
	toolbar.show();
	toolbar_container->add(toolbar);
}

Gtk::ToolButton*
Dockable::add_button(const Gtk::StockID& stock_id, const synfig::String& tooltip)
{
	if (!toolbar_container) reset_toolbar();
	Gtk::Toolbar *toolbar = dynamic_cast<Gtk::Toolbar*>(toolbar_container->get_child());
	if (!toolbar) {
		toolbar = manage(new Gtk::Toolbar());
		set_toolbar(*toolbar);
	}

	Gtk::ToolButton* ret(manage(new Gtk::ToolButton(stock_id)));
	ret->set_tooltip_text(tooltip);
	ret->show();
	toolbar->set_has_tooltip();
	toolbar->append(*ret);
	return ret;
}

void
Dockable::reset_container()
{
	if (container) delete container;
	container = manage(new Gtk::ScrolledWindow);
	container->set_shadow_type(Gtk::SHADOW_NONE);
	container->set_hexpand();
	container->set_vexpand();
	container->show();
	set_use_scrolled(use_scrolled);
	attach(*container, 0, 0, 1, 1);
	
	// to avoid GTK warning:
	//   Allocating size to widget without calling gtk_widget_get_preferred_width/height().
	//   How does the code know the size to allocate?
	// related with combination of Grid, ScrolledWindow and TreeView
	//App::process_all_events();
	// Update:
	// Seems bug in other place, process_all_events() here produces
	// a concurrent event processing and collissions
}

void
Dockable::reset_toolbar()
{
	if (toolbar_container) delete toolbar_container;
	toolbar_container = manage(new Gtk::EventBox);
	toolbar_container->set_hexpand();
	toolbar_container->show();
	attach(*toolbar_container, 0, 1, 1, 1);
}

void
Dockable::clear()
{
	reset_container();
	reset_toolbar();
}

void
Dockable::present()
{
	DockBook *parent = dynamic_cast<DockBook*>(get_parent());
	if (parent) {
		parent->set_current_page(parent->page_num(*this));
		parent->present();
	} else {
		show();

		DockBook* book = manage(new DockBook());
		book->add(*this);
		book->show();
		
		DockDialog* dock_dialog(new DockDialog());
		dock_dialog->add(*book);
		dock_dialog->present();
	}
	App::dock_manager->update_window_titles();
}

Gtk::Widget*
Dockable::create_tab_label()
{
	Gtk::EventBox *event_box = manage(new Gtk::EventBox());
	attach_dnd_to(*event_box);

	// Check to make sure the icon is valid
	Gtk::StockItem stock_item;
	if (Gtk::Stock::lookup(get_stock_id(), stock_item)) {
		// add icon
		Gtk::IconSize iconsize = Gtk::IconSize::from_name("synfig-small_icon_16x16");
		Gtk::Image* icon(manage(new Gtk::Image(get_stock_id(), iconsize)));
		icon->show();
		event_box->set_tooltip_text(get_local_name());
		event_box->add(*icon);
	} else {
		// bad icon, add label
		Gtk::Label* label = manage(new Gtk::Label(get_local_name()));
		label->show();
		event_box->add(*label);
	}

	return event_box;
}

void Dockable::write_layout_string(std::string& /*params*/) const
{
}

void Dockable::read_layout_string(const std::string& /*params*/) const
{
}
