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

#include <gui/iconcontroller.h>
#include <gui/localization.h>

#include <synfig/paramdesc.h>

#endif

using namespace synfig;
using namespace studio;

Widget_Interpolation::Widget_Interpolation(Side side)
{
	const auto interpolation = ParamDesc("interpolation")
		.set_hint("enum")
		.add_enum_value(INTERPOLATION_CLAMPED,  "clamped",  _("Clamped"))
		.add_enum_value(INTERPOLATION_TCB,      "auto",     _("TCB"))
		.add_enum_value(INTERPOLATION_CONSTANT, "constant", _("Constant"))
		.add_enum_value(INTERPOLATION_HALT,     "ease",
						side == SIDE_BOTH? _("Ease In/Out") : (
						side == SIDE_BEFORE? _("Ease In") : _("Ease Out")))
		.add_enum_value(INTERPOLATION_LINEAR,   "linear",   _("Linear"));

	set_param_desc(interpolation);
	int i = 0;
	for (const auto& item : interpolation.get_enum_list()) {
		set_icon(i++, interpolation_icon_name(static_cast<synfig::Interpolation>(item.value)));
	}
}
