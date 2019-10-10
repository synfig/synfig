/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_coloredit.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_STUDIO_WIDGET_COLOREDIT_H
#define __SYNFIG_STUDIO_WIDGET_COLOREDIT_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/box.h>
#include <gtkmm/table.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/colorselection.h>
#include <synfig/color.h>
#include "widgets/widget_color.h"

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
	synfig::Color color_, orig_color;

public:

	sigc::signal<void,Type,float>& signal_slider_moved() { return signal_slider_moved_; }
	sigc::signal<void>& signal_activated() { return signal_activated_; }

	Type
	get_type()const { return type; }

	const synfig::Color&
	get_color()const { return color_; }


	ColorSlider(const Type &x=TYPE_Y);

	void
	set_type(Type x);

	void
	set_color(synfig::Color x);

	static void adjust_color(Type type, synfig::Color &color, float amount);

private:
	typedef void (*slider_color_func)(synfig::Color &,float);

	static void slider_color_TYPE_R(synfig::Color &color, float amount);
	static void slider_color_TYPE_G(synfig::Color &color, float amount);
	static void slider_color_TYPE_B(synfig::Color &color, float amount);
	static void slider_color_TYPE_Y(synfig::Color &color, float amount);
	static void slider_color_TYPE_U(synfig::Color &color, float amount);
	static void slider_color_TYPE_V(synfig::Color &color, float amount);
	static void slider_color_TYPE_HUE(synfig::Color &color, float amount);
	static void slider_color_TYPE_SAT(synfig::Color &color, float amount);
	static void slider_color_TYPE_A(synfig::Color &color, float amount);

	bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);
	bool on_event(GdkEvent *event);

    ///@brief Draw face to face contrasted arrows
	void draw_arrow(
		const Cairo::RefPtr<Cairo::Context> &cr,
		double x, double y,
		double width, double height,
		int size,
		bool fill);
}; // END of class ColorSlider


class Widget_ColorEdit : public Gtk::Table
{
	sigc::signal<void> signal_activated_;
	sigc::signal<void> signal_value_changed_;

	ColorSlider *slider_R;
	ColorSlider *slider_G;
	ColorSlider *slider_B;
	Gtk::Label *hex_color_label;
	Gtk::Entry *hex_color;

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

	Gtk::ColorSelection * hvsColorWidget;

	Glib::RefPtr<Gtk::Adjustment> R_adjustment;
	Glib::RefPtr<Gtk::Adjustment> G_adjustment;
	Glib::RefPtr<Gtk::Adjustment> B_adjustment;
	Glib::RefPtr<Gtk::Adjustment> A_adjustment;

	synfig::Color color;

	Gtk::Notebook* notebook;

	static synfig::Gamma hvs_gamma;
	static synfig::Gamma hvs_gamma_in;

protected:

	void on_value_changed();

public:

	sigc::signal<void>& signal_activated() { return signal_activated_; }

	sigc::signal<void>& signal_activate() { return signal_activated_; }

	void on_slider_moved(ColorSlider::Type type, float amount);
	void on_hex_edited();
	bool on_hex_focus_out(GdkEventFocus* event);

	//Glib::SignalProxy0<void> signal_activate() { return spinbutton_A->signal_activate(); }

	sigc::signal<void>& signal_value_changed() { return signal_value_changed_; }
	
	void on_color_changed();

	void activated() { signal_activated_(); }
	void activate() { signal_activated_(); }
	void set_value(const synfig::Color &data);
	const synfig::Color &get_value();
	synfig::Color get_value_raw();
	void set_has_frame(bool x);
	void set_digits(int x);
	Widget_ColorEdit();
	~Widget_ColorEdit();

private:
	bool colorHVSChanged; //Spike. Look more in the code.
	///@brief Sets color to the widget
	void setHVSColor(synfig::Color color);
	///@brief The function adds slider into the row table with label. 
	void SliderRow(int i,ColorSlider * n, char * l, Pango::AttrList & attr_list, Gtk::Table* table);
	///@brief The function adds spin button into the table.
	void AttachSpinButton(int i, Gtk::SpinButton * n, Gtk::Table * table);

}; // END of class Widget_ColorEdit

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
