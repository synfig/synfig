/* === S I N F G =========================================================== */
/*!	\file cellrenderer_value.cpp
**	\brief Template File
**
**	$Id: cellrenderer_value.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SINFG_GTKMM_CELLRENDERER_VALUE_H
#define __SINFG_GTKMM_CELLRENDERER_VALUE_H

/* === H E A D E R S ======================================================= */

#include <gtk/gtk.h>
#include <gtkmm/ruler.h>
#include <gtkmm/arrow.h>
#include <gtkmm/image.h>
#include <gdkmm/pixbufloader.h>
#include <gtkmm/viewport.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/table.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <atkmm/stateset.h>
#include <gtkmm/paned.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/box.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/cellrenderer.h>
#include <gtkmm/checkbutton.h>

#include <gtkmm/colorselection.h>
#include <gtkmm/optionmenu.h>

//#include <sinfg/sinfg.h>
#include <sinfg/paramdesc.h>
#include <sinfg/value.h>


/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_Color;
class Widget_CanvasChooser;
class Widget_Enum;
class Widget_Filename;
class Widget_Vector;
class Widget_Time;

class ValueBase_Entry;

class CellRenderer_ValueBase : public Gtk::CellRendererText
{
	sigc::signal<void, const Glib::ustring&> signal_secondary_click_;
	sigc::signal<void, const Glib::ustring&, sinfg::ValueBase> signal_edited_;

	Glib::Property<sinfg::ValueBase> property_value_;
	Glib::Property<etl::handle<sinfg::Canvas> > property_canvas_;
	Glib::Property<sinfg::ParamDesc> property_param_desc_;

	void string_edited_(const Glib::ustring&,const Glib::ustring&);

	void gradient_edited(sinfg::Gradient gradient, Glib::ustring path);
	void color_edited(sinfg::Color color, Glib::ustring path);
	
public:
	sigc::signal<void, const Glib::ustring&> &signal_secondary_click()
	{return signal_secondary_click_; }

	sigc::signal<void, const Glib::ustring&, sinfg::ValueBase> &signal_edited()
	{return signal_edited_; }

	Glib::PropertyProxy<sinfg::ValueBase> property_value() { return property_value_.get_proxy();}
	Glib::PropertyProxy<etl::handle<sinfg::Canvas> > property_canvas() { return property_canvas_.get_proxy();}
	Glib::PropertyProxy<sinfg::ParamDesc> property_param_desc() { return property_param_desc_.get_proxy(); }
	Glib::PropertyProxy<bool> property_inconsistant() { return property_foreground_set(); }

	etl::handle<sinfg::Canvas> get_canvas()const { return property_canvas_; }
	sinfg::ParamDesc get_param_desc()const { return property_param_desc_; }
	
	CellRenderer_ValueBase();
	~CellRenderer_ValueBase();

	ValueBase_Entry *value_entry;

	void on_value_editing_done();

	virtual void
	render_vfunc(
		const Glib::RefPtr<Gdk::Drawable>& window,
		Gtk::Widget& widget,
		const Gdk::Rectangle& background_area,
		const Gdk::Rectangle& ca,
		const Gdk::Rectangle& expose_area,
		Gtk::CellRendererState flags);
	
	virtual Gtk::CellEditable* start_editing_vfunc(GdkEvent* event,
                                                 Gtk::Widget& widget,
                                                 const Glib::ustring& path,
                                                 const Gdk::Rectangle& background_area,
                                                 const Gdk::Rectangle& cell_area,
                                                 Gtk::CellRendererState flags);

}; // END of class CellRenderer_ValueBase

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
