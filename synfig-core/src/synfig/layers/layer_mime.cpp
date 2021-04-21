/* === S Y N F I G ========================================================= */
/*!	\file layer_mime.cpp
**	\brief Template File
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

#include <synfig/layer.h>
#include <synfig/time.h>
#include <synfig/string.h>
#include <synfig/vector.h>

#include <synfig/context.h>
#include <synfig/surface.h>
#include <synfig/renddesc.h>
#include <synfig/target.h>

#include <synfig/localization.h>
#include <synfig/paramdesc.h>

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
	param_list["Version"]=(const char*)"9";
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
