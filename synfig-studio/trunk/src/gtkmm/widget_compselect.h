/* === S I N F G =========================================================== */
/*!	\file widget_compselect.h
**	\brief Template Header
**
**	$Id: widget_compselect.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SINFG_STUDIO_WIDGET_COMPSELECT_H
#define __SINFG_STUDIO_WIDGET_COMPSELECT_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/optionmenu.h>
#include <gtkmm/menu.h>
#include "app.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class Menu; };

namespace studio {

class Widget_CompSelect : public Gtk::OptionMenu
{
	Gtk::Menu	instance_list_menu;
	

	etl::loose_handle<studio::Instance>	selected_instance;
	void set_selected_instance_(etl::handle<studio::Instance> x);
	
	void new_instance(etl::handle<studio::Instance> x);

	void delete_instance(etl::handle<studio::Instance> x);

	void set_selected_instance(etl::loose_handle<studio::Instance> x);

	void set_selected_instance_signal(etl::handle<studio::Instance> x);
		
public:

	Widget_CompSelect();
	~Widget_CompSelect();
	
	etl::loose_handle<studio::Instance> get_selected_instance() { return selected_instance; }

	void refresh();
}; // END of class Widget_CompSelect

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
