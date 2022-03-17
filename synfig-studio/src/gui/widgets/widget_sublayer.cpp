/* === S Y N F I G ========================================================= */
/*!	\file widget_sublayer.cpp
**	\brief Template File
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <gui/widgets/widget_sublayer.h>

#include <gui/localization.h>

#include <synfig/canvas.h>
#include <synfig/context.h>
#include <synfig/layers/layer_pastecanvas.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
Widget_Sublayer::set_value_desc(const synfigapp::ValueDesc &x)
{
	value_desc = x;

	remove_all();

	if(etl::handle<synfig::Layer_PasteCanvas> p = etl::handle<synfig::Layer_PasteCanvas>::cast_dynamic(value_desc.get_layer()))
	{
		synfig::Canvas::Handle canvas = p->get_sub_canvas();
		if(canvas)
		{
			// Fill the combo with the layers' descriptions
			append("", _("<empty>"));
			for(IndependentContext i = canvas->get_independent_context(); *i; ++i)
			{
				std::string desc = (*i)->get_description();
				append(desc, desc);
			}
		}
	}
	refresh();
}

void
Widget_Sublayer::refresh()
{
	set_active_id(value);
}

void
Widget_Sublayer::set_value(const std::string& data)
{
	value = data;
	refresh();
}

std::string
Widget_Sublayer::get_value() const
{
	return value;
}

void
Widget_Sublayer::on_changed()
{
	value = get_active_id();
}
