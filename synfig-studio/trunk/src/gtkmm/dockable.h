/* === S Y N F I G ========================================================= */
/*!	\file dockable.h
**	\brief Template Header
**
**	$Id: dockable.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SYNFIG_STUDIO_DOCKABLE_H
#define __SYNFIG_STUDIO_DOCKABLE_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/stockid.h>
#include <gtkmm/button.h>
#include "dialogsettings.h"
#include <synfig/string.h>
#include <gtkmm/table.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/label.h>
#include <gtkmm/frame.h>
#include <gtkmm/handlebox.h>
#include <gtkmm/box.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/toolbutton.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {
	
class DockManager;
class DockBook;
	
class Dockable : public Gtk::Table
{
	friend class DockManager;
	friend class DockBook;

	
	sigc::signal<void> signal_stock_id_changed_;
	SigC::Connection prev_widget_delete_connection;
protected:
	
//	DialogSettings dialog_settings;


private:

	Gtk::Toolbar *toolbar_;

	synfig::String name_;
	synfig::String local_name_;
	Gtk::Tooltips tooltips_;
	Gtk::Frame frame_;
	Gtk::Label title_label_;
	//Gtk::HBox button_box_;
	Gtk::HBox header_box_;
	//Gtk::VBox vbox_;
	
	//Gtk::HandleBox handle_box_;
	Gtk::ScrolledWindow *scrolled_;
	Gtk::Widget *prev_widget_;

	bool use_scrolled_;
	
	Gtk::StockID stock_id_;

	DockBook* parent_;

	bool dnd_success_;

public:
	//Gtk::VBox* get_vbox() { return &vbox_; }

	void set_toolbar(Gtk::Toolbar& toolbar);

	void set_use_scrolled(bool x) { use_scrolled_=x; }
	
	Dockable(const synfig::String& name,const synfig::String& local_name,Gtk::StockID stock_id_=Gtk::StockID(" "));
	~Dockable();

	sigc::signal<void>& signal_stock_id_changed() { return signal_stock_id_changed_; }

	const synfig::String& get_name()const { return name_; }
	const synfig::String& get_local_name()const { return local_name_; }

	const Gtk::StockID& get_stock_id()const { return stock_id_; }
	void set_stock_id(Gtk::StockID x) { stock_id_=x; signal_stock_id_changed()(); }
	
	void set_local_name(const synfig::String&);

	void clear();
	
	Gtk::Tooltips& get_tooltips() { return tooltips_; }
	
	//DialogSettings& settings() { return dialog_settings; }
	//const DialogSettings& settings()const { return dialog_settings; }
	
	void add(Gtk::Widget& x);
	
	Gtk::ToolButton* add_button(const Gtk::StockID& stock_id, const synfig::String& tooltip=synfig::String());	
	
	void detach();

	void present();
	
	void attach_dnd_to(Gtk::Widget& widget);

	bool clear_previous();
	virtual Gtk::Widget* create_tab_label();

private:
	
	void on_drag_data_get(const Glib::RefPtr<Gdk::DragContext>&, Gtk::SelectionData& selection_data, guint info, guint time);
	void on_drag_end(const Glib::RefPtr<Gdk::DragContext>&context);
	void on_drag_begin(const Glib::RefPtr<Gdk::DragContext>&context);
	void on_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int, int, const Gtk::SelectionData& selection_data, guint, guint time);

}; // END of studio::Dockable

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
