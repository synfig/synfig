/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_canvaschooser.h
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

#ifndef __SYNFIG_STUDIO_WIDGET_CANVASCHOOSER_H
#define __SYNFIG_STUDIO_WIDGET_CANVASCHOOSER_H

/* === H E A D E R S ======================================================= */

#include <vector>

#include <synfig/canvas.h>
#include <gtkmm/comboboxtext.h>


/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class Menu; };

namespace studio {

class Widget_CanvasChooser : public Gtk::ComboBoxText
{
	Gtk::Menu *canvas_menu;
	synfig::Canvas::Handle parent_canvas;
	std::vector<synfig::Canvas::Handle> canvases;

	synfig::Canvas::Handle canvas;
	void set_value_(synfig::Canvas::Handle data);

protected:
	virtual void on_changed();

public:
	Widget_CanvasChooser();
	~Widget_CanvasChooser();

	void set_parent_canvas(synfig::Canvas::Handle x);
	void set_value(synfig::Canvas::Handle data);
	const synfig::Canvas::Handle &get_value();

private:
	void chooser_menu();
}; // END of class Widget_CanvasChooser

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
