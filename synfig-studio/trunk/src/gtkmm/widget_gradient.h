/* === S I N F G =========================================================== */
/*!	\file widget_gradient.h
**	\brief Template Header
**
**	$Id: widget_gradient.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SINFG_STUDIO_WIDGET_GRADIENT_H
#define __SINFG_STUDIO_WIDGET_GRADIENT_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/drawingarea.h>
#include <sinfg/gradient.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

void render_gradient_to_window(const Glib::RefPtr<Gdk::Drawable>& window,const Gdk::Rectangle& ca,const sinfg::Gradient &gradient);

class Widget_Gradient : public Gtk::DrawingArea
{
	sigc::signal<void> signal_value_changed_;
	sigc::signal<void> signal_clicked_;

	sigc::signal<void,sinfg::Gradient::CPoint> signal_cpoint_selected_;

	sinfg::Gradient gradient_;
	
	bool editable_;
	
	bool changed_;
	
	sinfg::Gradient::CPoint	selected_cpoint;

	void popup_menu(float x);	

	void insert_cpoint(float x);	

	void remove_cpoint(float x);	
		
public:
	
	Widget_Gradient();
	
	~Widget_Gradient();

	sigc::signal<void>& signal_value_changed() { return signal_value_changed_; }
	sigc::signal<void>& signal_clicked() { return signal_clicked_; }

	sigc::signal<void,sinfg::Gradient::CPoint>& signal_cpoint_selected() { return signal_cpoint_selected_; }
	
	void set_value(const sinfg::Gradient& x);

	const sinfg::Gradient& get_value()const { return gradient_; }	
	
	void set_editable(bool x=true) { editable_=x; }
	
	bool get_editable()const { return editable_; }


	
	void set_selected_cpoint(const sinfg::Gradient::CPoint &x);

	const sinfg::Gradient::CPoint& get_selected_cpoint() { return selected_cpoint; }

	void update_cpoint(const sinfg::Gradient::CPoint &x);
	


	bool redraw(GdkEventExpose*bleh=NULL);

	bool on_event(GdkEvent *event);
}; // END of class BlackLevelSelector

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
