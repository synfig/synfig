/*! ========================================================================
** Sinfg
** Template Header File
** $Id: toolbox.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
** This software and associated documentation
** are CONFIDENTIAL and PROPRIETARY property of
** the above-mentioned copyright holder.
**
** You may not copy, print, publish, or in any
** other way distribute this software without
** a prior written agreement with
** the copyright holder.
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SINFG_GTKMM_TOOLBOX_H
#define __SINFG_GTKMM_TOOLBOX_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/window.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/button.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/menu.h>
#include <gtkmm/table.h>
#include <sinfg/string.h>
#include "smach.h"
#include <map>
#include "dialogsettings.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Dockable;
class StateManager;
	
class Toolbox : public Gtk::Window
{
	friend class studio::StateManager;
	
	DialogSettings dialog_settings;

	Gtk::Tooltips tooltips;
	Gtk::Button *button_undo;
	Gtk::Button *button_redo;

	Gtk::Button *button_eyedrop;
	Gtk::Button *button_rotoscope;
	Gtk::Button *button_rotoscope_bline;
	Gtk::Button *button_rotoscope_polygon;
	
	Gtk::Table *tool_table;

	std::map<sinfg::String,Gtk::ToggleButton *> state_button_map;
	
	Gtk::Menu	*recent_files_menu;

	Gtk::Menu	*dock_dialogs;

	bool changing_state_;
	
	void on_recent_files_changed();	
	void on_drop_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, const Gtk::SelectionData& selection_data, guint info, guint time);

	void change_state_(const Smach::state_base *state);
	
public:

	void change_state(const sinfg::String& statename);

	void update_undo_redo();

	void refresh() { update_undo_redo(); }

	void set_active_state(const sinfg::String& statename);

	void add_state(const Smach::state_base *state);

	
	void dockable_registered(Dockable* x);
	
	Toolbox();
	virtual ~Toolbox();
	
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
