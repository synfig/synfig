/* === S Y N F I G ========================================================= */
/*!	\file paramdesc.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "paramdesc.h"
#include "value.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ParamDesc::ParamDesc(synfig::Color::BlendMethod, const String &a):
	name_			(a),
	local_name_		(a),
	scalar_			(1.0),
	critical_		(true),
	hidden_			(false),
	invisible_duck_	(false),
	is_distance_	(false),
	animation_only_	(false)
{
	set_local_name(_("Blend Method"))
		.set_hint("enum")		// used shortcuts: A B C D E F G H I K L M N O P R S T U V Y; free: J Q W X Z
		.add_enum_value(Color::BLEND_COMPOSITE,			"composite",		_("_Composite"		))
		.add_enum_value(Color::BLEND_STRAIGHT,			"straight",			_("_Straight"		))
		.add_enum_value(Color::BLEND_ONTO,				"onto",				_("_Onto"			))
		.add_enum_value(Color::BLEND_STRAIGHT_ONTO,		"straightonto",		_("S_traight Onto"	))
		.add_enum_value(Color::BLEND_BEHIND,			"behind",			_("_Behind"			))
		.add_enum_value(Color::BLEND_SCREEN,			"screen",			_("Sc_reen"			))
		.add_enum_value(Color::BLEND_OVERLAY,			"overlay",			_("Overla_y"		))
		.add_enum_value(Color::BLEND_HARD_LIGHT,		"hard_light",		_("_Hard Light"		))
		.add_enum_value(Color::BLEND_MULTIPLY,			"multiply",			_("_Multiply"		))
		.add_enum_value(Color::BLEND_DIVIDE,			"divide",			_("_Divide"			))
		.add_enum_value(Color::BLEND_ADD,				"add",				_("_Add"			))
		.add_enum_value(Color::BLEND_SUBTRACT,			"subtract",			_("S_ubtract"		))
		.add_enum_value(Color::BLEND_DIFFERENCE,		"difference",		_("Di_fference"		))
		.add_enum_value(Color::BLEND_BRIGHTEN,			"brighten",			_("Bri_ghten"		))
		.add_enum_value(Color::BLEND_DARKEN,			"darken",			_("Dar_ken"			))
		.add_enum_value(Color::BLEND_COLOR,				"color",			_("Co_lor"			))
		.add_enum_value(Color::BLEND_HUE,				"hue",				_("Hu_e"			))
		.add_enum_value(Color::BLEND_SATURATION,		"saturation",		_("Saturatio_n"		))
		.add_enum_value(Color::BLEND_LUMINANCE,			"luminance",		_("Lum_inance"		))
		// These are deprecated
		.add_enum_value(Color::BLEND_ALPHA_OVER,		"alphaover",		_("Alpha O_ver"		))
		.add_enum_value(Color::BLEND_ALPHA_BRIGHTEN,	"alphabrighten",	_("Al_pha Brighten"	))
		.add_enum_value(Color::BLEND_ALPHA_DARKEN,		"alphadarken",		_("Al_pha Darken"	))
		; // end of enums
}

ParamDesc::ParamDesc(const ValueBase&, const String &a):
	name_			(a),
	local_name_		(a),
	scalar_			(1.0),
	critical_		(true),
	hidden_			(false),
	invisible_duck_	(false),
		is_distance_	(false),
		animation_only_	(false)
{
}
