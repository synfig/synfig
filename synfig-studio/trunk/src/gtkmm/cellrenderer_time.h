/* === S I N F G =========================================================== */
/*!	\file cellrenderer_time.h
**	\brief Template Header
**
**	$Id: cellrenderer_time.h,v 1.1.1.1 2005/01/07 03:34:35 darco Exp $
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

#ifndef __SINFG_STUDIO_CELLRENDERER_TIME_H
#define __SINFG_STUDIO_CELLRENDERER_TIME_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/entry.h>
#include <gtkmm/cellrenderertext.h>

#include <sigc++/signal.h>
#include <sigc++/slot.h>

#include <sinfg/time.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class Entry; class Button; };

namespace studio {

class CellRenderer_Time : public Gtk::CellRendererText
{
	sigc::signal<void, const Glib::ustring&> signal_secondary_click_;
	sigc::signal<void, const Glib::ustring&, sinfg::Time> signal_edited_;

	Glib::Property<sinfg::Time> property_time_;
	Glib::Property<sinfg::Time> property_fps_;

	void string_edited_(const Glib::ustring&,const Glib::ustring&);

	void on_value_editing_done();

public:
	sigc::signal<void, const Glib::ustring&, sinfg::Time> &signal_edited()
	{return signal_edited_; }

	Glib::PropertyProxy<sinfg::Time> property_time() { return property_time_.get_proxy();}
	Glib::PropertyProxy<sinfg::Time> property_fps() { return property_fps_.get_proxy();}
	
	CellRenderer_Time();
	~CellRenderer_Time();

protected:
	
	virtual void
	render_vfunc(
		const Glib::RefPtr<Gdk::Drawable>& window,
		Gtk::Widget& widget,
		const Gdk::Rectangle& background_area,
		const Gdk::Rectangle& ca,
		const Gdk::Rectangle& expose_area,
		Gtk::CellRendererState flags);

	virtual Gtk::CellEditable* start_editing_vfunc(GdkEvent* event,
                                                 Gtk::Widget& widget,
                                                 const Glib::ustring& path,
                                                 const Gdk::Rectangle& background_area,
                                                 const Gdk::Rectangle& cell_area,
                                                 Gtk::CellRendererState flags);

}; // END of class CellRenderer_Time

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
