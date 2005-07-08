/* === S Y N F I G ========================================================= */
/*!	\file widget_filename.h
**	\brief Template Header
**
**	$Id: widget_filename.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SYNFIG_STUDIO_WIDGET_FILENAME_H
#define __SYNFIG_STUDIO_WIDGET_FILENAME_H

/* === H E A D E R S ======================================================= */

#include <sigc++/signal.h>
#include <sigc++/slot.h>
#include <gtkmm/box.h>
#include <gtkmm/entry.h>
#include <gtkmm/button.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class Entry; class Button; };

namespace studio {

class Widget_Filename : public Gtk::HBox
{
	Gtk::Entry *entry_filename;
	Gtk::Button *button_choose;

	void on_button_choose_pressed();

	sigc::signal<void> signal_value_changed_;

public:
	sigc::signal<void> &signal_value_changed() { return signal_value_changed_; }
	Glib::SignalProxy0<void> signal_activate() { return entry_filename->signal_activate(); }

	void on_value_changed();
	
	void set_value(const  std::string &data);
	std::string get_value() const;
	void set_has_frame(bool x);
	Widget_Filename();
	~Widget_Filename();
}; // END of class Widget_Filename

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
