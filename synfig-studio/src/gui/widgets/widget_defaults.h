/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_defaults.h
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

#ifndef __SYNFIG_STUDIO_WIDGET_DEFAULTS_H
#define __SYNFIG_STUDIO_WIDGET_DEFAULTS_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/drawingarea.h>
#include <gtkmm/table.h>
#include <synfig/gradient.h>
#include "widgets/widget_gradient.h"
#include <gtkmm/tooltip.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class HScale; }

namespace studio {

class Widget_Brush;
class Widget_Color;
class Widget_Distance;
class Widget_Enum;

class Widget_Defaults : public Gtk::Table
{
	Widget_Brush *widget_brush;
	Widget_Color *widget_otln_color;
	Widget_Color *widget_fill_color;
	Widget_Distance *widget_bline_width;
	Widget_Gradient *widget_gradient;
	Widget_Enum	*widget_blend_method;
	Widget_Enum	*widget_interpolation;
	Gtk::HScale *widget_opacity;

	void otln_color_refresh();
	void fill_color_refresh();
	void gradient_refresh();
	void bline_width_refresh();
	void interpolation_refresh();

	void on_bline_width_changed();
	void on_otln_color_clicked();
	void on_fill_color_clicked();
	void on_swap_color_clicked();
	void on_reset_color_clicked();
	void on_gradient_clicked();
	void on_interpolation_changed();

	void blend_method_refresh();
	void on_blend_method_changed();

	void opacity_refresh();
	void on_opacity_changed();

public:

	Widget_Defaults();

	~Widget_Defaults();

//	bool redraw(GdkEventExpose*bleh=NULL);

//	bool on_event(GdkEvent *event);
}; // END of class BlackLevelSelector

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
