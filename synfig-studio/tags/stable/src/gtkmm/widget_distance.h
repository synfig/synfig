/* === S I N F G =========================================================== */
/*!	\file widget_distance.h
**	\brief Template Header
**
**	$Id: widget_distance.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SINFG_STUDIO_WIDGET_DISTANCE_H
#define __SINFG_STUDIO_WIDGET_DISTANCE_H

/* === H E A D E R S ======================================================= */

#include <sigc++/signal.h>
#include <sigc++/slot.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/adjustment.h>
#include <sinfg/distance.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class Entry; class Button; };

namespace studio {

class Widget_Distance : public Gtk::SpinButton
{
	//sigc::signal<void> signal_value_changed_;
	
	mutable sinfg::Distance distance_;

	Gtk::Adjustment adjustment;
	
protected:
	
	int	on_input(double* new_value);
	bool on_output();

public:
	//sigc::signal<void> &signal_value_changed() { return signal_value_changed_; }
	
	void set_value(const sinfg::Distance &data);
	sinfg::Distance get_value()const;
	Widget_Distance();
	~Widget_Distance();
}; // END of class Widget_Distance

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
