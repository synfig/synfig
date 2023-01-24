/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_link.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2014 Jérôme Blanchi
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/general.h>

#include "widgets/widget_link.h"

#endif

/* === U S I N G =========================================================== */

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */



namespace studio {

Widget_Link::Widget_Link(const std::string &tlt_inactive, const std::string &tlt_active)
{
	set_relief(Gtk::RELIEF_NONE);

	tooltip_inactive_ = tlt_inactive;
	tooltip_active_ = tlt_active;
	Widget_Link::on_toggled();
}

Widget_Link::~Widget_Link() = default;

void Widget_Link::on_toggled()
{
	if(get_active())
	{
		set_image_from_icon_name("utils_chain_link_on_icon", Gtk::BuiltinIconSize::ICON_SIZE_BUTTON);
		set_tooltip_text(tooltip_active_);
	}
	else
	{
		set_image_from_icon_name("utils_chain_link_off_icon", Gtk::BuiltinIconSize::ICON_SIZE_BUTTON);
		set_tooltip_text(tooltip_inactive_);
	}
}

}
