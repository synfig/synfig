/* === S Y N F I G ========================================================= */
/*!	\file dockdialog.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "docks/dockdialog.h"

#include <gtkmm/textview.h>

#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/docks/dockmanager.h>
#include <gui/exception_guard.h>
#include <gui/localization.h>
#include <gui/mainwindow.h>

#include <synfig/general.h>
#include <synfig/uniqueid.h>

#include <synfigapp/main.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

DockDialog::DockDialog():
	Gtk::Window(Gtk::WINDOW_TOPLEVEL)
{
	is_deleting=false;

	// Give ourselves an ID that is most likely unique
	set_id(synfig::UniqueID().get_uid()^reinterpret_cast<intptr_t>(this));

	set_role(strprintf("dock_dialog_%d",get_id()));
	{
#ifdef __APPLE__
		const int default_hint = Gdk::WINDOW_TYPE_HINT_NORMAL;
#else
		const int default_hint = Gdk::WINDOW_TYPE_HINT_UTILITY;
#endif
		set_type_hint(Gdk::WindowTypeHint(synfigapp::Main::settings().get_value("pref.dock_dialog_hints", default_hint)));
	}
	set_keep_above(false);

	//! \todo can we set dialog windows transient for all normal windows, not just the toolbox?
	//! paragraph 3 of http://standards.freedesktop.org/wm-spec/1.3/ar01s07.html suggests we can
	// this seems to have bad effects on KDE, so leave it disabled by default
	if(getenv("SYNFIG_TRANSIENT_DIALOGS"))
		set_transient_for(*App::main_window);

	// Set up the window
	//set_type_hint(Gdk::WINDOW_TYPE_HINT_UTILITY);
	set_title(_("Dock Panel"));

	set_deletable(true);

	// Register with the dock manager
	App::dock_manager->dock_dialog_list_.push_back(this);

	App::signal_present_all().connect(sigc::mem_fun0(*this,&DockDialog::present));
}

DockDialog::~DockDialog()
{
	empty_sig.disconnect();

	is_deleting=true;

	// Remove us from the dock manager
	DockManager::containers_to_remove_.erase(this);
	if(App::dock_manager)try{
		std::list<DockDialog*>::iterator iter;
		for(iter=App::dock_manager->dock_dialog_list_.begin();iter!=App::dock_manager->dock_dialog_list_.end();++iter)
			if(*iter==this)
			{
				App::dock_manager->dock_dialog_list_.erase(iter);
				break;
			}
	}
	catch(...)
	{
		synfig::warning("DockDialog::~DockDialog(): Exception thrown when trying to remove from dock manager...?");
	}
}

bool
DockDialog::on_delete_event(GdkEventAny * /* event */)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	for (std::list<Dockable*>::iterator i = App::dock_manager->dockable_list_.begin(); i != App::dock_manager->dockable_list_.end(); ++i) {
		if ((*i)->get_parent_window() == get_window())
		{
			CanvasView *canvas_view = dynamic_cast<CanvasView*>(*i);
			if (canvas_view)
				canvas_view->close_view();
			else
				DockManager::remove_widget_recursive(**i);
		}
	}
	delete this;
	return true;
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}

bool DockDialog::on_key_press_event(GdkEventKey* key_event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	Gtk::Widget * widget = get_focus();
	if (widget && (dynamic_cast<Gtk::Editable*>(widget) || dynamic_cast<Gtk::TextView*>(widget) || dynamic_cast<Gtk::DrawingArea*>(widget))) {
		bool handled = gtk_window_propagate_key_event(this->gobj(), key_event);
		if (handled)
			return true;
	}
	return Gtk::Window::on_key_press_event(key_event);
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}

bool
DockDialog::close()
{
	DEBUG_LOG("SYNFIG_DEBUG_DESTRUCTORS",
		"DockDialog::close(): Deleted");

	empty_sig.disconnect();
	//get_dock_book().clear();
	delete this;
	return false;
}
