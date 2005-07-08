/* === S Y N F I G ========================================================= */
/*!	\file widget_canvaschooser.h
**	\brief Template Header
**
**	$Id: widget_canvaschooser.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SYNFIG_STUDIO_WIDGET_CANVASCHOOSER_H
#define __SYNFIG_STUDIO_WIDGET_CANVASCHOOSER_H

/* === H E A D E R S ======================================================= */

#include <synfig/canvas.h>
#include <gtkmm/optionmenu.h>


/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class Menu; };

namespace studio {

class Widget_CanvasChooser : public Gtk::OptionMenu
{
	Gtk::Menu *canvas_menu;
	synfig::Canvas::Handle parent_canvas;

	synfig::Canvas::Handle canvas;
	void set_value_(synfig::Canvas::Handle data);
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
