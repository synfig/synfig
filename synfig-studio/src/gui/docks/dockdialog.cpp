/* === S Y N F I G ========================================================= */
/*!	\file dockdialog.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "app.h"

#include "docks/dockdialog.h"
#include "docks/dockbook.h"
#include "docks/dockmanager.h"
#include "mainwindow.h"
#include "canvasview.h"
#include "widgets/widget_compselect.h"
#include <synfig/general.h>
#include <synfig/uniqueid.h>
#include <gtkmm/table.h>
#include "canvasview.h"
#include <gtkmm/paned.h>
#include <gtkmm/box.h>
#include <synfigapp/main.h>

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#define GRAB_HINT_DATA(y,default)	{ \
		String x; \
		if(synfigapp::Main::settings().get_value(String("pref.")+y+"_hints",x)) \
		{ \
			set_type_hint((Gdk::WindowTypeHint)atoi(x.c_str()));	\
		} else {\
			set_type_hint(default); \
		} \
	}

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
	GRAB_HINT_DATA(
		"dock_dialog",
#ifdef __APPLE__
		Gdk::WINDOW_TYPE_HINT_NORMAL
#else
		Gdk::WINDOW_TYPE_HINT_UTILITY
#endif
	);
	set_keep_above(false);

	//! \todo can we set dialog windows transient for all normal windows, not just the toolbox?
	//! paragraph 3 of http://standards.freedesktop.org/wm-spec/1.3/ar01s07.html suggests we can
	// this seems to have bad effects on KDE, so leave it disabled by default
	if(getenv("SYNFIG_TRANSIENT_DIALOGS"))
		set_transient_for(*App::main_window);

	// Set up the window
	//set_type_hint(Gdk::WINDOW_TYPE_HINT_UTILITY);
	set_title(_("Dock Panel"));

	// Register with the dock manager
	App::dock_manager->dock_dialog_list_.push_back(this);

	add_accel_group(App::ui_manager()->get_accel_group());
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
	for(std::list<Dockable*>::iterator i = App::dock_manager->dockable_list_.begin(); i != App::dock_manager->dockable_list_.end(); i++)
	{
		if ((*i)->get_parent_window() == get_window())
		{
			CanvasView *canvas_view = dynamic_cast<CanvasView*>(*i);
			if (canvas_view)
				canvas_view->close_view();
			else
				DockManager::remove_widget_recursive(**i);
		}
	}
	return false;
}

bool
DockDialog::close()
{
	if (getenv("SYNFIG_DEBUG_DESTRUCTORS"))
		synfig::info("DockDialog::close(): Deleted");

	empty_sig.disconnect();
	//get_dock_book().clear();
	delete this;
	return false;
}
