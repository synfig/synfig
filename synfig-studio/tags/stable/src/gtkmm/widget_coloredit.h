/* === S I N F G =========================================================== */
/*!	\file widget_coloredit.h
**	\brief Template Header
**
**	$Id: widget_coloredit.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SINFG_STUDIO_WIDGET_COLOREDIT_H
#define __SINFG_STUDIO_WIDGET_COLOREDIT_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/box.h>
#include <gtkmm/table.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/drawingarea.h>
#include <sinfg/color.h>
#include "widget_color.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk {
	class Notebook;
};

namespace studio {

class ColorSlider : public Gtk::DrawingArea
{
public:
	enum Type
	{
		TYPE_R,
		TYPE_G,
		TYPE_B,
		TYPE_Y,
		TYPE_U,
		TYPE_V,
		TYPE_HUE,
		TYPE_SAT,
		TYPE_A,
		
		TYPE_END
	};
	
private:

	sigc::signal<void,Type,float> signal_slider_moved_;
	sigc::signal<void> signal_activated_;

	Type type;
	sinfg::Color color_;

public:

	sigc::signal<void,Type,float>& signal_slider_moved() { return signal_slider_moved_; }
	sigc::signal<void>& signal_activated() { return signal_activated_; }

	Type
	get_type()const { return type; }

	const sinfg::Color&
	get_color()const { return color_; }


	ColorSlider(const Type &x=TYPE_Y);

	void
	set_type(Type x);

	void
	set_color(sinfg::Color x);

	static void adjust_color(Type type, sinfg::Color &color, float amount);

private:
	typedef void (*slider_color_func)(sinfg::Color &,float);

	static void slider_color_TYPE_R(sinfg::Color &color, float amount);
	static void slider_color_TYPE_G(sinfg::Color &color, float amount);
	static void slider_color_TYPE_B(sinfg::Color &color, float amount);
	static void slider_color_TYPE_Y(sinfg::Color &color, float amount);
	static void slider_color_TYPE_U(sinfg::Color &color, float amount);
	static void slider_color_TYPE_V(sinfg::Color &color, float amount);
	static void slider_color_TYPE_HUE(sinfg::Color &color, float amount);
	static void slider_color_TYPE_SAT(sinfg::Color &color, float amount);
	static void slider_color_TYPE_A(sinfg::Color &color, float amount);
	

	bool
	redraw(GdkEventExpose*bleh);
	bool on_event(GdkEvent *event);
}; // END of class ColorSlider

	
class Widget_ColorEdit : public Gtk::Table
{
	sigc::signal<void> signal_activated_;
	sigc::signal<void> signal_value_changed_;

	ColorSlider *slider_R;
	ColorSlider *slider_G;
	ColorSlider *slider_B;
	ColorSlider *slider_A;
	ColorSlider *slider_Y;
	ColorSlider *slider_U;
	ColorSlider *slider_V;
	ColorSlider *slider_SAT;
	ColorSlider *slider_HUE;
	
	Widget_Color widget_color;
	
	bool hold_signals;
	
	bool clamp_;
	
	Gtk::SpinButton *spinbutton_R;
	Gtk::SpinButton *spinbutton_G;
	Gtk::SpinButton *spinbutton_B;
	Gtk::SpinButton *spinbutton_A;

	Gtk::Adjustment R_adjustment;
	Gtk::Adjustment G_adjustment;
	Gtk::Adjustment B_adjustment;
	Gtk::Adjustment A_adjustment;
	
	sinfg::Color color;

	Gtk::Notebook* notebook;

protected:
	
	void on_value_changed();

public:

	sigc::signal<void>& signal_activated() { return signal_activated_; }

	sigc::signal<void>& signal_activate() { return signal_activated_; }

	void on_slider_moved(ColorSlider::Type type, float amount);

	//Glib::SignalProxy0<void> signal_activate() { return spinbutton_A->signal_activate(); }
	
	sigc::signal<void>& signal_value_changed() { return signal_value_changed_; }
	
	void activated() { signal_activated_(); }
	void activate() { signal_activated_(); }
	void set_value(const sinfg::Color &data);
	const sinfg::Color &get_value();
	sinfg::Color get_value_raw();
	void set_has_frame(bool x);
	void set_digits(int x);
	Widget_ColorEdit();
	~Widget_ColorEdit();
}; // END of class Widget_ColorEdit
	
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
