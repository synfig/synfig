/* === S I N F G =========================================================== */
/*!	\file widget_color.h
**	\brief Template Header
**
**	$Id: widget_color.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SINFG_STUDIO_WIDGET_COLOR_H
#define __SINFG_STUDIO_WIDGET_COLOR_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/box.h>
#include <gtkmm/table.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/drawingarea.h>
#include <sinfg/color.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {


Gdk::Color colorconv_sinfg2gdk(const sinfg::Color &c);
	
void render_color_to_window(const Glib::RefPtr<Gdk::Drawable>& window,const Gdk::Rectangle& ca,const sinfg::Color &color);

class Widget_Color : public Gtk::DrawingArea
{
	sinfg::Color color;
	
	sigc::signal<void> signal_activate_;
	sigc::signal<void> signal_secondary_;

protected:

public:
	sigc::signal<void>& signal_activate() { return signal_activate_; }
	sigc::signal<void>& signal_clicked() { return signal_activate_; }
	sigc::signal<void>& signal_secondary() { return signal_secondary_; }
	
	void set_value(const sinfg::Color &data);
	const sinfg::Color &get_value();
	Widget_Color();
	~Widget_Color();
private:
	bool redraw(GdkEventExpose*bleh);
	bool on_event(GdkEvent *event);

}; // END of class Widget_Color
	
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
