/*! ========================================================================
** Sinfg
** Template File
** $Id: xorpattern.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
** This software and associated documentation
** are CONFIDENTIAL and PROPRIETARY property of
** the above-mentioned copyright holder.
**
** You may not copy, print, publish, or in any
** other way distribute this software without
** a prior written agreement with
** the copyright holder.
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

#include <sinfg/string.h>
#include <sinfg/time.h>
#include <sinfg/context.h>
#include <sinfg/paramdesc.h>
#include <sinfg/renddesc.h>
#include <sinfg/surface.h>
#include <sinfg/value.h>
#include <sinfg/valuenode.h>

#endif

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SINFG_LAYER_INIT(XORPattern);
SINFG_LAYER_SET_NAME(XORPattern,"XORPattern");
SINFG_LAYER_SET_LOCAL_NAME(XORPattern,_("XOR Pattern"));
SINFG_LAYER_SET_CATEGORY(XORPattern,_("Other"));
SINFG_LAYER_SET_VERSION(XORPattern,"0.1");
SINFG_LAYER_SET_CVS_ID(XORPattern,"$Id: xorpattern.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $");

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
