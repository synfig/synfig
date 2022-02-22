/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_interpolation.h
**	\brief Widget for interpolation selection
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2022      Rodolfo R. Gomes
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "widget_interpolation.h"

#include <gui/localization.h>

#include <synfig/paramdesc.h>

#endif

using namespace synfig;
using namespace studio;

Widget_Interpolation::Widget_Interpolation(Side side)
{
	set_param_desc(
		ParamDesc("interpolation")
			.set_hint("enum")
			.add_enum_value(INTERPOLATION_CLAMPED,"clamped",_("Clamped"))
			.add_enum_value(INTERPOLATION_TCB,"auto",_("TCB"))
			.add_enum_value(INTERPOLATION_CONSTANT,"constant",_("Constant"))
			.add_enum_value(INTERPOLATION_HALT,"ease",
							side == SIDE_BOTH? _("Ease In/Out") : (
							side == SIDE_BEFORE? _("Ease In") :
												 _("Ease Out")))
			.add_enum_value(INTERPOLATION_LINEAR,"linear",_("Linear"))
	);
	set_icons();
}

void Widget_Interpolation::set_icons()
{
	const unsigned int num_interpolations = 5;
	// Must follow the same order of ParamDesc "interpolation" enum:
	const char* interpolation_icons[num_interpolations] = {
		"synfig-interpolation_type_clamped",
		"synfig-interpolation_type_tcb",
		"synfig-interpolation_type_const",
		"synfig-interpolation_type_ease",
		"synfig-interpolation_type_linear"
	};
	for (auto i = 0; i < num_interpolations; ++i)
		set_icon(i, Gtk::Button().render_icon_pixbuf(Gtk::StockID(interpolation_icons[i]),Gtk::ICON_SIZE_MENU));
}
