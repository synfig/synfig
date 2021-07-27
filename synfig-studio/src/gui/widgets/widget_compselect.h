/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_compselect.h
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

#ifndef __SYNFIG_STUDIO_WIDGET_COMPSELECT_H
#define __SYNFIG_STUDIO_WIDGET_COMPSELECT_H

/* === H E A D E R S ======================================================= */

#include <ETL/handle>
#include <gtkmm/comboboxtext.h>
#include <gui/instance.h>
#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_CompSelect : public Gtk::ComboBoxText
{
	std::vector< std::shared_ptr<studio::Instance> > instances;
	std::shared_ptr<studio::Instance>	selected_instance;

	void set_selected_instance_(std::shared_ptr<studio::Instance> x);

	void new_instance(std::shared_ptr<studio::Instance> x);

	void delete_instance(std::shared_ptr<studio::Instance> x);

	void set_selected_instance(std::shared_ptr<studio::Instance> x);

	void set_selected_instance_signal(std::shared_ptr<studio::Instance> x);

protected:
	virtual void on_changed();

public:
	Widget_CompSelect();
	~Widget_CompSelect();

	std::shared_ptr<studio::Instance> get_selected_instance() { return selected_instance; }

	void refresh();
}; // END of class Widget_CompSelect

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
