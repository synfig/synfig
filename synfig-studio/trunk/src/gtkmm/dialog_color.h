/* === S I N F G =========================================================== */
/*!	\file dialog_color.h
**	\brief Template Header
**
**	$Id: dialog_color.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SINFG_STUDIO_DIALOG_COLOR_H
#define __SINFG_STUDIO_DIALOG_COLOR_H

/* === H E A D E R S ======================================================= */

#include <gtk/gtk.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/table.h>
#include <gtkmm/button.h>
#include <gtkmm/dialog.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/optionmenu.h>
#include <gtkmm/checkbutton.h>

#include <sinfg/gamma.h>
#include <sinfg/time.h>

#include "widget_coloredit.h"

#include <sinfgapp/value_desc.h>
#include <sinfg/time.h>

#include "dialogsettings.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class Menu; class SpinButton; class Adjustment; };

namespace sinfgapp {
class CanvasInterface;
};

namespace studio {

class Widget_Color;
	
class Dialog_Color : public Gtk::Dialog
{
	DialogSettings dialog_settings;
	
	sigc::signal<void,sinfg::Color> signal_edited_;
	//sigc::signal<void,sinfg::Color> signal_apply_;

	bool on_close_pressed();
	void on_apply_pressed();
	void on_color_changed();

	Widget_ColorEdit* widget_color;

	bool busy_;
	
public:
	bool busy()const { return busy_; }
	
	sigc::signal<void,sinfg::Color>& signal_edited() { return signal_edited_; }
	
	//sigc::signal<void,sinfg::Color>& signal_apply() { return signal_apply_; }
	
	void set_color(const sinfg::Color& x) { widget_color->set_value(x); }

	sinfg::Color get_color()const { return widget_color->get_value(); }
		
	void reset();

	
	Dialog_Color();
	~Dialog_Color();

//	void edit(const sinfgapp::ValueDesc &x, etl::handle<sinfgapp::CanvasInterface> canvas_interface, sinfg::Time x=0);
}; // END of Dialog_Color

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
