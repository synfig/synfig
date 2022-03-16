/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_sublayer.h
**	\brief Template Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2015 Max May
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_STUDIO_WIDGET_SUBLAYER_H
#define __SYNFIG_STUDIO_WIDGET_SUBLAYER_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/comboboxtext.h>
#include <synfigapp/value_desc.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {
class Widget_Sublayer : public Gtk::ComboBoxText
{
	synfigapp::ValueDesc value_desc;
	std::string value;

public:

	Widget_Sublayer() = default;
	~Widget_Sublayer() = default;

	void set_value_desc(const synfigapp::ValueDesc &x);

	void set_value(const std::string &data);
	std::string get_value() const;

protected:
	void refresh();
	void on_changed() override;
}; // END of class Widget_Sublayer
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
