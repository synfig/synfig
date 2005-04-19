/* === S Y N F I G ========================================================= */
/*!	\file layer_mime.cpp
**	\brief Template File
**
**	$Id: layer_mime.cpp,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#include "layer_mime.h"

#include "layer.h"
#include "time.h"
#include "string.h"
#include "vector.h"

#include "context.h"
#include "time.h"
#include "color.h"
#include "surface.h"
#include "renddesc.h"
#include "target.h"

#include "general.h"
#include "paramdesc.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Layer_Mime::Layer_Mime(String x):name(x)
{
	// Throw a bogus default version onto the parameter list.
	param_list["Version"]="9";
}

String
Layer_Mime::get_version()const
{
	return get_param("Version").get(String());
}

bool
Layer_Mime::set_version(const String &ver)
{
	return set_param("Version",ver);
}

String
Layer_Mime::get_local_name()const
{
	return _("[MIME]")+get_name();
}

bool
Layer_Mime::set_param(const String &param, const ValueBase &value)
{
	// Don't try to set the name
	if(param=="name" || param=="Name" || param=="name__")
		return false;

	// Otherwise, remember this parameter's value
	param_list[param]=value;
	return true;
}

ValueBase
Layer_Mime::get_param(const String &param)const
{
	// If they are requesting the name of
	// the layer, just return it
	if(param=="name" || param=="Name" || param=="name__")
		return ValueBase(name);

	// Otherwise, return the stored parameter value
	map<string,ValueBase>::const_iterator iter=param_list.find(param);
	if(iter!=param_list.end())
		return iter->second;
	return ValueBase();
}

Color
Layer_Mime::get_color(Context context, const Point &pos)const
{
	// A Layer_Mime layer should do nothing at all.
	return context.get_color(pos);
}

bool
Layer_Mime::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	// A Layer_Mime layer should do nothing at all.
	return context.accelerated_render(surface,quality,renddesc,cb);
}

Layer::Vocab
Layer_Mime::get_param_vocab()const
{
	Layer::Vocab ret;
	map<string,ValueBase>::const_iterator iter;

	// Construct the vocabulary from the stored
	// parameters
	for(iter=param_list.begin();iter!=param_list.end();iter++)
	{
		// Make sure that we don't add the version
		// into the vocabulary
		if(iter->first!="Version")
			ret.push_back(ParamDesc(iter->first));
	}
	
	// ... and return it
	return ret;
}
