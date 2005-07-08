/* === S Y N F I G ========================================================= */
/*!	\file dialog_history.h
**	\brief Template Header
**
**	$Id: dock_history.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_STUDIO_DIALOG_HISTORY_H
#define __SYNFIG_STUDIO_DIALOG_HISTORY_H

/* === H E A D E R S ======================================================= */

#include "dockable.h"
#include <gtkmm/treeview.h>
#include "instance.h"
#include <gtkmm/actiongroup.h>
#include "dock_canvasspecific.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Dock_History : public Dock_CanvasSpecific
{	
	Glib::RefPtr<Gtk::ActionGroup> action_group;
	Gtk::TreeView *action_tree;

	etl::loose_handle<studio::Instance>	selected_instance;
	void set_selected_instance_(etl::handle<studio::Instance> x);


	void set_selected_instance(etl::loose_handle<studio::Instance> x);

	void set_selected_instance_signal(etl::handle<studio::Instance> x);

	void delete_instance(etl::handle<studio::Instance> x);

	Gtk::Widget* create_action_tree();

public:

	etl::loose_handle<studio::Instance> get_selected_instance() { return selected_instance; }

	void clear_undo();
	void clear_redo();

	bool on_action_event(GdkEvent *event);
	void on_action_toggle(const Glib::ustring& path);

	void update_undo_redo();
	
	Dock_History();
	~Dock_History();
protected:
	virtual void init_instance_vfunc(etl::loose_handle<Instance> instance);

}; // END of Dock_History

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
