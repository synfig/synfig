/* === S I N F G =========================================================== */
/*!	\file widget_enum.h
**	\brief Template Header
**
**	$Id: widget_enum.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SINFG_STUDIO_WIDGET_ENUM_H
#define __SINFG_STUDIO_WIDGET_ENUM_H

/* === H E A D E R S ======================================================= */

#include <sinfg/layer.h>
#include <gtkmm/optionmenu.h>
#include <sinfg/paramdesc.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class Menu; };

namespace studio {

class Widget_Enum : public Gtk::OptionMenu
{
	Gtk::Menu *enum_menu;
	sinfg::ParamDesc param_desc;
	
	int value;
	void set_value_(int data);
public:

	Widget_Enum();
	~Widget_Enum();
	
	void set_param_desc(const sinfg::ParamDesc &x);
	void refresh();

	void set_value(int data);
	int get_value() const;
}; // END of class Widget_Enum

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
