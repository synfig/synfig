/* === S I N F G =========================================================== */
/*!	\file paramdesc.cpp
**	\brief Template File
**
**	$Id: paramdesc.cpp,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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
using namespace sinfg;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ParamDesc::ParamDesc(sinfg::Color::BlendMethod, const String &a):
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
	.set_hint("enum")
	.add_enum_value(Color::BLEND_COMPOSITE,"composite",_("Composite"))
	.add_enum_value(Color::BLEND_STRAIGHT,"straight",_("Straight"))
	.add_enum_value(Color::BLEND_ONTO,"onto",_("Onto"))
	.add_enum_value(Color::BLEND_STRAIGHT_ONTO,"straightonto",_("StraightOnto"))
	.add_enum_value(Color::BLEND_BEHIND,"behind",_("Behind"))
	.add_enum_value(Color::BLEND_SCREEN,"screen",_("Screen"))
	.add_enum_value(Color::BLEND_OVERLAY,"overlay",_("Overlay"))
	.add_enum_value(Color::BLEND_HARD_LIGHT,"hard_light",_("Hard Light"))
	.add_enum_value(Color::BLEND_MULTIPLY,"multiply",_("Multiply"))
	.add_enum_value(Color::BLEND_DIVIDE,"divide",_("Divide"))
	.add_enum_value(Color::BLEND_ADD,"add",_("Add"))
	.add_enum_value(Color::BLEND_SUBTRACT,"subtract",_("Subtract"))
	.add_enum_value(Color::BLEND_DIFFERENCE,"difference",_("Difference"))
	.add_enum_value(Color::BLEND_BRIGHTEN,"brighten",_("Brighten"))
	.add_enum_value(Color::BLEND_DARKEN,"darken",_("Darken"))
	.add_enum_value(Color::BLEND_COLOR,"color",_("Color"))
	.add_enum_value(Color::BLEND_HUE,"hue",_("Hue"))
	.add_enum_value(Color::BLEND_SATURATION,"saturation",_("Saturation"))
	.add_enum_value(Color::BLEND_LUMINANCE,"luminance",_("Luminance"))
// These are deprecated
	.add_enum_value(Color::BLEND_ALPHA_OVER,"alphaover",_("Alpha Over"))
//	.add_enum_value(Color::BLEND_ALPHA_BRIGHTEN,"alphabrighten",_("Alpha Brighten"))
//	.add_enum_value(Color::BLEND_ALPHA_DARKEN,"alphadarken",_("Alpha Darken"))
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
