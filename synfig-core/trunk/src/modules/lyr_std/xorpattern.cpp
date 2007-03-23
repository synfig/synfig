/* === S Y N F I G ========================================================= */
/*!	\file xorpattern.cpp
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "xorpattern.h"

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>

#endif

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(XORPattern);
SYNFIG_LAYER_SET_NAME(XORPattern,"XORPattern");
SYNFIG_LAYER_SET_LOCAL_NAME(XORPattern,_("XOR Pattern"));
SYNFIG_LAYER_SET_CATEGORY(XORPattern,_("Other"));
SYNFIG_LAYER_SET_VERSION(XORPattern,"0.1");
SYNFIG_LAYER_SET_CVS_ID(XORPattern,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

XORPattern::XORPattern():
	pos(0.125,0.125),
	size(0.25,0.25)
{
}

bool
XORPattern::set_param(const String & param, const ValueBase &value)
{
	IMPORT(pos);
	IMPORT(size);
	return false;
}

ValueBase
XORPattern::get_param(const String & param)const
{
	EXPORT(pos);
	EXPORT(size);

	EXPORT_NAME();
	EXPORT_VERSION();

	return ValueBase();
}

Color
XORPattern::get_color(Context context, const Point &point)const
{
	unsigned int a=(unsigned int)floor((point[0]+pos[0])/size[0]), b=(unsigned int)floor((point[1]+pos[1])/size[1]);
	unsigned char rindex=(a^b);
	unsigned char gindex=(a^(~b))*4;
	unsigned char bindex=~(a^b)*2;

	return Color((Color::value_type)rindex/(Color::value_type)255.0,(Color::value_type)gindex/(Color::value_type)255.0,(Color::value_type)bindex/(Color::value_type)255.0,1.0);
}

Layer::Vocab
XORPattern::get_param_vocab()const
{
	Layer::Vocab ret;

	ret.push_back(ParamDesc("pos")
		.set_local_name(_("Offset"))
	);
	ret.push_back(ParamDesc("size")
		.set_local_name(_("Size"))
		.set_origin("pos")
	);

	return ret;
}
