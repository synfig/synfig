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

#include <gtkmm/adjustment.h>
#include <gtkmm/colorselection.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/grid.h>
#include <gtkmm/spinbutton.h>
#include <gui/widgets/widget_color.h>
#include <gui/widgets/widget_colorslider.h>
#include <gui/widgets/widget_eyedropper.h>
#include <gui/widgets/widget_hsv_plane.h>
#include <synfig/color.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk {
	class Notebook;
};

namespace studio {

class Widget_ColorEdit : public Gtk::Bin
{
	sigc::signal<void> signal_activated_;
	sigc::signal<void> signal_value_changed_;

	Widget_ColorSlider *slider_R;
	Widget_ColorSlider *slider_G;
	Widget_ColorSlider *slider_B;

	Widget_ColorSlider *slider_A;
	Widget_ColorSlider *slider_vertical;

	Widget_HSV_Plane *hsv_plane;

	Widget_Color *widget_color;
	Widget_Eyedropper *widget_eyedropper;





	synfig::Color color;


protected:


	void on_button_eyedropper_clicked();
	void on_eyedropper_picked(const Gdk::RGBA& rgba);

public:

	sigc::signal<void>& signal_activated() { return signal_activated_; }

	sigc::signal<void>& signal_activate() { return signal_activated_; }

	sigc::signal<void>& signal_value_changed() { return signal_value_changed_; }
	

	void set_value(const synfig::Color &new_color);
	const synfig::Color &get_value();
	void set_has_frame(bool x);
	Widget_ColorEdit();

}; // END of class Widget_ColorEdit

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
