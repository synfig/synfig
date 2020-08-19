/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_interpolationenum.cpp
**	\brief Widget for interpolation selection
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**            (c) 2020 Rodolfo Ribeiro Gomes
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "widget_interpolationenum.h"

#include <synfig/paramdesc.h>

#include <gui/localization.h>

#include <gtkmm/icontheme.h>

#endif

using namespace synfig;
using namespace studio;

Widget_InterpolationEnum::Widget_InterpolationEnum(Side side)
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

	Gtk::IconTheme::get_default()->signal_changed().connect(sigc::mem_fun(*this, &Widget_InterpolationEnum::set_icons));
	// GTK docs suggests to do so instead of above line, but it receive several calls (on focus, on "blur", theme changes, etc)
	// signal_style_updated().connect(sigc::mem_fun(*this, &Widget_InterpolationEnum::set_icons));
}

void Widget_InterpolationEnum::set_icons()
{
	int w, icon_height;
	Gtk::IconSize::lookup(Gtk::ICON_SIZE_MENU, w, icon_height);
	set_icon(0, Gtk::IconTheme::get_default()->load_icon("interpolation_type_clamped_icon", icon_height));
	set_icon(1, Gtk::IconTheme::get_default()->load_icon("interpolation_type_tcb_icon", icon_height));
	set_icon(2, Gtk::IconTheme::get_default()->load_icon("interpolation_type_const_icon", icon_height));
	set_icon(3, Gtk::IconTheme::get_default()->load_icon("interpolation_type_ease_icon", icon_height));
	set_icon(4, Gtk::IconTheme::get_default()->load_icon("interpolation_type_linear_icon", icon_height));
}
