/* === S I N F G =========================================================== */
/*!	\file widget_value.cpp
**	\brief Template File
**
**	$Id: widget_value.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SINFG_GTKMM_VALUE_H
#define __SINFG_GTKMM_VALUE_H

/* === H E A D E R S ======================================================= */

//#include <gtk/gtk.h>
//#include <gtkmm/ruler.h>
//#include <gtkmm/arrow.h>
//#include <gtkmm/image.h>
//#include <gdkmm/pixbufloader.h>
//#include <gtkmm/viewport.h>
#include <gtkmm/adjustment.h>
//#include <gtkmm/scrolledwindow.h>
#include <gtkmm/table.h>
//#include <gtkmm/statusbar.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
//#include <atkmm/stateset.h>
//#include <gtkmm/paned.h>
//#include <gtkmm/treeview.h>
//#include <gtkmm/treestore.h>
#include <gtkmm/box.h>
#include <gtkmm/spinbutton.h>
//#include <gtkmm/cellrenderer.h>
#include <gtkmm/checkbutton.h>

//#include <gtkmm/colorselection.h>
#include <gtkmm/optionmenu.h>

//#include <sinfg/sinfg.h>
#include <sinfg/paramdesc.h>
#include <sinfg/value.h>
#include <sinfg/canvas.h>


/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_Color;
class Widget_ColorEdit;
class Widget_CanvasChooser;
class Widget_Enum;
class Widget_Filename;
class Widget_Vector;
class Widget_Time;
class Widget_Distance;
	
class Widget_ValueBase : public Gtk::HBox
{
	Gtk::Label *label;
	sinfg::ValueBase value;

	Widget_Vector *vector_widget;
	Gtk::SpinButton *real_widget;
	Gtk::Adjustment real_adjustment;
	Gtk::SpinButton *integer_widget;
	Gtk::Adjustment integer_adjustment;
	Gtk::SpinButton *angle_widget;
	Gtk::Adjustment angle_adjustment;

	Gtk::CheckButton *bool_widget;
	//Gtk::ColorSelection *color_widget;
	Widget_ColorEdit *color_widget;
	Widget_CanvasChooser *canvas_widget;
	Widget_Enum *enum_widget;
	Widget_Filename *filename_widget;
	Widget_Time *time_widget;
	Gtk::Entry *string_widget;
	Widget_Distance *distance_widget;
	
//	std::string hint;
	
	sinfg::ParamDesc param_desc;
	etl::handle<sinfg::Canvas> canvas;
	sigc::signal<void> signal_value_changed_;
	sigc::signal<void> signal_activate_;

public:
	sigc::signal<void> &signal_value_changed() { return signal_value_changed_; }
	
	void activate();
	
	sigc::signal<void>& signal_activate() { return signal_activate_; }

	void set_value(const sinfg::ValueBase &data);
	const sinfg::ValueBase &get_value();

	void on_grab_focus();
	
	void set_param_desc(const sinfg::ParamDesc &x) { param_desc=x; }
	const sinfg::ParamDesc &get_param_desc() { return param_desc; }

	void set_sensitive(bool x);
	
	//void set_hint(std::string x) { hint=x; }
//	std::string get_hint() { return hint; }

	void set_canvas(etl::handle<sinfg::Canvas> x) { canvas=x; assert(canvas); }
	void inside_cellrenderer();
	Widget_ValueBase();
	~Widget_ValueBase();
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
