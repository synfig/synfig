/* === S I N F G =========================================================== */
/*!	\file dialog_gradient.h
**	\brief Template Header
**
**	$Id: dialog_gradient.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SINFG_STUDIO_DIALOG_GRADIENT_H
#define __SINFG_STUDIO_DIALOG_GRADIENT_H

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

#include "widget_gradient.h"
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

class Widget_Gradient;
class Widget_ColorEdit;
	
class Dialog_Gradient : public Gtk::Dialog
{
	DialogSettings dialog_settings;
	
	Gtk::SpinButton *spinbutton_pos;

	Gtk::Adjustment adjustment_pos;


	sigc::signal<void,sinfg::Gradient> signal_edited_;

	sigc::connection value_changed_connection;
	
	void on_ok_pressed();
	void on_apply_pressed();
	void on_grab_pressed();
	
	void on_cpoint_selected(sinfg::Gradient::CPoint x);
	void on_values_adjusted();

	Widget_Gradient* widget_gradient;
	Widget_ColorEdit* widget_color;

	void on_changed();
	
public:

	sigc::signal<void,sinfg::Gradient>& signal_edited() { return signal_edited_; }
	
	void set_gradient(const sinfg::Gradient& x);

	const sinfg::Gradient& get_gradient()const { return widget_gradient->get_value(); }
		
	void reset();

	
	Dialog_Gradient();
	~Dialog_Gradient();

	void edit(const sinfgapp::ValueDesc &x, etl::handle<sinfgapp::CanvasInterface> canvas_interface, sinfg::Time x=0);
}; // END of Dialog_Gradient

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
