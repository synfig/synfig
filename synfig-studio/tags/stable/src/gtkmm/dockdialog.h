/* === S I N F G =========================================================== */
/*!	\file dockdialog.h
**	\brief Template Header
**
**	$Id: dockdialog.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SINFG_STUDIO_DOCK_DIALOG_H
#define __SINFG_STUDIO_DOCK_DIALOG_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/stockid.h>
#include <gtkmm/button.h>
#include "dialogsettings.h"
#include <sinfg/string.h>
#include <gtkmm/dialog.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/label.h>
#include <gtkmm/frame.h>
#include <gtkmm/handlebox.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/accelgroup.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class Box; class Paned;  };
namespace studio {
	
class DockManager;
class DockBook;
class Dockable;
class Widget_CompSelect;
class CanvasView;
	
class DockDialog : public Gtk::Window
{
	friend class DockManager;
	friend class DockBook;
	friend class Dockable;
	SigC::Connection empty_sig;

	bool composition_selector_;
	
	bool is_deleting;
	
	bool is_horizontal;
	
private:
	std::list<DockBook*> dock_book_list;

	std::vector<Gtk::Paned*>	pannels_;
	std::vector<int>			dock_book_sizes_;


	DockBook* last_dock_book;

	Widget_CompSelect* widget_comp_select;
	Gtk::Box *box;

	int id_;

	void on_hide();

	void refresh();

	void refresh_title();

	void set_id(int x) { id_=x; }

	void refresh_accel_group();

	void drop_on_append(const Glib::RefPtr<Gdk::DragContext>& context, int, int, const Gtk::SelectionData& selection_data, guint, guint time);
	void drop_on_prepend(const Glib::RefPtr<Gdk::DragContext>& context, int, int, const Gtk::SelectionData& selection_data, guint, guint time);
	
public:

	const std::vector<int>&	get_dock_book_sizes()const { return dock_book_sizes_;}
	void set_dock_book_sizes(const std::vector<int>&);
	void rebuild_sizes();
	
	bool close();
	
	int get_id()const { return id_; }
	
	DockBook* append_dock_book();
	DockBook* prepend_dock_book();
	void erase_dock_book(DockBook*);
	
	void set_composition_selector(bool x);
	bool get_composition_selector()const { return composition_selector_; }
	
	DockDialog();
	~DockDialog();

	DockBook& get_dock_book();
	const DockBook& get_dock_book()const;

	sinfg::String get_contents()const;
	void set_contents(const sinfg::String& x);
}; // END of studio::DockDialog

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
