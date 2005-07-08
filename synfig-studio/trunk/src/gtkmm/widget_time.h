/* === S Y N F I G ========================================================= */
/*!	\file widget_time.h
**	\brief Template Header
**
**	$Id: widget_time.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SYNFIG_STUDIO_WIDGET_TIME_H
#define __SYNFIG_STUDIO_WIDGET_TIME_H

/* === H E A D E R S ======================================================= */

#include <sigc++/signal.h>
#include <sigc++/slot.h>
#include <gtkmm/entry.h>
#include <synfig/time.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class Entry; class Button; };

namespace studio {

class Widget_Time : public Gtk::Entry
{


	sigc::signal<void> signal_value_changed_;
	
	float fps_;
	
	synfig::Time time_;
	
protected:
	bool on_focus_out_event(GdkEventFocus* event);

	bool on_focus_in_event(GdkEventFocus* event);
	
	//void on_activate();
	
	void refresh_text();

	void refresh_value();
	
	bool on_event(GdkEvent* event);

public:
	sigc::signal<void> &signal_value_changed() { return signal_value_changed_; }


	
	void set_value(const synfig::Time &data);
	synfig::Time get_value()const;
	void set_fps(float x);
	Widget_Time();
	~Widget_Time();
}; // END of class Widget_Time

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
