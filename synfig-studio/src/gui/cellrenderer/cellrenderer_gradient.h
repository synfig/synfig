/* === S Y N F I G ========================================================= */
/*!	\file cellrenderer_gradient.h
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

#ifndef __SYNFIG_STUDIO_CELLRENDERER_GRADIENT_H
#define __SYNFIG_STUDIO_CELLRENDERER_GRADIENT_H

/* === H E A D E R S ======================================================= */

#include <glibmm/property.h>
#include <glibmm/propertyproxy.h>

#include <gtkmm/cellrenderertext.h>

#include <synfig/gradient.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class Entry; class Button; };

namespace studio {

class CellRenderer_Gradient : public Gtk::CellRendererText
{
	sigc::signal<void, const Glib::ustring&> signal_secondary_click_;
	sigc::signal<void, const Glib::ustring&, synfig::Gradient> signal_edited_;

	Glib::Property<synfig::Gradient> property_gradient_;

public:
	sigc::signal<void, const Glib::ustring&, synfig::Gradient> &signal_edited()
	{return signal_edited_; }

	Glib::PropertyProxy<synfig::Gradient> property_gradient() { return property_gradient_.get_proxy();}

	CellRenderer_Gradient();
	~CellRenderer_Gradient();

protected:

	virtual void
	render_vfunc(
		const ::Cairo::RefPtr< ::Cairo::Context>& cr,
		Gtk::Widget& widget,
		const Gdk::Rectangle& background_area,
		const Gdk::Rectangle& cell_area,
		Gtk::CellRendererState flags);

	virtual Gtk::CellEditable* start_editing_vfunc(GdkEvent* event,
                                                 Gtk::Widget& widget,
                                                 const Glib::ustring& path,
                                                 const Gdk::Rectangle& background_area,
                                                 const Gdk::Rectangle& cell_area,
                                                 Gtk::CellRendererState flags);

}; // END of class CellRenderer_Gradient

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
