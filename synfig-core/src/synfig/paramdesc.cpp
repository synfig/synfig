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

#include "general.h"
#include <synfig/localization.h>
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

const ParamDesc ParamVocab::blank;

ParamDesc::ParamDesc(const ValueBase&, const String &a):
	name_           (a),
	local_name_     (a),
	scalar_         (1.0),
	exponential_    (false),
	critical_       (true),
	hidden_         (false),
	invisible_duck_ (false),
	is_distance_    (false),
	animation_only_ (false),
	static_         (false),
	interpolation_  (INTERPOLATION_UNDEFINED)
{
	if(a == "blend_method")
	{
		set_local_name(_("Blend Method"))
		.set_hint("enum")		// used shortcuts: A B C D E F G H I K L M N O P R S T U V Y; free: J Q W X Z
		.add_enum_value(Color::BLEND_COMPOSITE,      "composite",    _("Composite"))
		.add_enum_value(Color::BLEND_STRAIGHT,       "straight",     _("Straight"))
		.add_enum_value(Color::BLEND_ONTO,           "onto",         _("Onto"))
		.add_enum_value(Color::BLEND_STRAIGHT_ONTO,  "straightonto", _("Straight Onto"))
		.add_enum_value(Color::BLEND_BEHIND,         "behind",       _("Behind"))
		.add_enum_value(Color::BLEND_SCREEN,         "screen",       _("Screen"))
		.add_enum_value(Color::BLEND_OVERLAY,        "overlay",      _("Overlay"))
		.add_enum_value(Color::BLEND_HARD_LIGHT,     "hard_light",   _("Hard Light"))
		.add_enum_value(Color::BLEND_MULTIPLY,       "multiply",     _("Multiply"))
		.add_enum_value(Color::BLEND_DIVIDE,         "divide",       _("Divide"))
		.add_enum_value(Color::BLEND_ADD,            "add",          _("Add"))
		.add_enum_value(Color::BLEND_SUBTRACT,       "subtract",     _("Subtract"))
		.add_enum_value(Color::BLEND_DIFFERENCE,     "difference",   _("Difference"))
		.add_enum_value(Color::BLEND_BRIGHTEN,       "brighten",     _("Brighten"))
		.add_enum_value(Color::BLEND_DARKEN,         "darken",       _("Darken"))
		.add_enum_value(Color::BLEND_COLOR,          "color",        _("Color"))
		.add_enum_value(Color::BLEND_HUE,            "hue",          _("Hue"))
		.add_enum_value(Color::BLEND_SATURATION,     "saturation",   _("Saturation"))
		.add_enum_value(Color::BLEND_LUMINANCE,      "luminance",    _("Luminance"))
		.add_enum_value(Color::BLEND_ALPHA_OVER,     "alphaover",    _("Alpha Over"))
		.add_enum_value(Color::BLEND_ALPHA_BRIGHTEN, "alphabrighten",_("Alpha Brighten"))
		.add_enum_value(Color::BLEND_ALPHA_DARKEN,   "alphadarken",  _("Alpha Darken"))
		.add_enum_value(Color::BLEND_ALPHA,          "alpha",        _("Alpha"))
		; // end of enums
	}
}
