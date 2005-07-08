/* === S Y N F I G ========================================================= */
/*!	\file dockbook.h
**	\brief Template Header
**
**	$Id: dockbook.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SYNFIG_STUDIO_DOCKBOOK_H
#define __SYNFIG_STUDIO_DOCKBOOK_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/notebook.h>
#include <synfig/string.h>
#include <gtkmm/tooltips.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {
	
class DockManager;
class Dockable;
	
class DockBook : public Gtk::Notebook
{
	friend class DockManager;
	friend class Dockable;

	sigc::signal<void> signal_empty_;
	sigc::signal<void> signal_changed_;
	
	Gtk::Tooltips tooltips_;	
	
	bool deleting_;
	
protected:
public:
	DockBook();
	~DockBook();

	sigc::signal<void>& signal_empty() { return signal_empty_; }
	sigc::signal<void>& signal_changed() { return signal_changed_; }

	void add(Dockable& dockable, int position=-1);
	void remove(Dockable& dockable);

	void present();

	void clear();

	synfig::String get_local_contents()const;
	
	synfig::String get_contents()const;
	void set_contents(const synfig::String& x);

	void refresh_tabs_headers();
	
	void refresh_tab(Dockable*);

	bool tab_button_pressed(GdkEventButton* event, Dockable* dockable);
	void on_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int, int, const Gtk::SelectionData& selection_data, guint, guint time);
}; // END of studio::DockBook

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
