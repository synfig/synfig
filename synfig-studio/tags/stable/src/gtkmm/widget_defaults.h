/* === S I N F G =========================================================== */
/*!	\file widget_defaults.h
**	\brief Template Header
**
**	$Id: widget_defaults.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SINFG_STUDIO_WIDGET_DEFAULTS_H
#define __SINFG_STUDIO_WIDGET_DEFAULTS_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/drawingarea.h>
#include <gtkmm/table.h>
#include <sinfg/gradient.h>
#include "widget_gradient.h"
#include <gtkmm/tooltips.h>

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
	Widget_Color *widget_fg_color;
	Widget_Color *widget_bg_color;
	Widget_Distance *widget_bline_width;
	Widget_Gradient *widget_gradient;
	Widget_Enum	*widget_blend_method;
	Widget_Enum	*widget_interpolation;
	Gtk::HScale *widget_opacity;
	
	void fg_color_refresh();
	void bg_color_refresh();
	void gradient_refresh();
	void bline_width_refresh();
	void interpolation_refresh();
	
	void on_bline_width_changed();
	void on_fg_color_clicked();
	void on_bg_color_clicked();
	void on_swap_color_clicked();
	void on_reset_color_clicked();
	void on_gradient_clicked();
	void on_interpolation_changed();

	void blend_method_refresh();
	void on_blend_method_changed();

	void opacity_refresh();
	void on_opacity_changed();

	Gtk::Tooltips tooltips_;
	
public:
	
	Widget_Defaults();
	
	~Widget_Defaults();

//	bool redraw(GdkEventExpose*bleh=NULL);

//	bool on_event(GdkEvent *event);
}; // END of class BlackLevelSelector

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
