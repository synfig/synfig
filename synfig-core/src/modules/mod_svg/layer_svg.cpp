/* === S Y N F I G ========================================================= */
/*!	\file layer_svg.cpp
**	\brief Implementation of the Svg layer
**
**	$Id:$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2009 Carlos A. Sosa Navarro
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

#include <synfig/string.h>
#include <synfig/paramdesc.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>

#include "layer_svg.h"

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(svg_layer);
SYNFIG_LAYER_SET_NAME(svg_layer,"svg_layer");
SYNFIG_LAYER_SET_LOCAL_NAME(svg_layer,N_("Import Svg"));
SYNFIG_LAYER_SET_CATEGORY(svg_layer,CATEGORY_DO_NOT_USE);//Hide this layer in the menu
SYNFIG_LAYER_SET_VERSION(svg_layer,"0.1");
SYNFIG_LAYER_SET_CVS_ID(svg_layer,"$Id: layer_svg.cpp 2240 2008-11-22 15:35:33Z dooglus $");

/* === P R O C E D U R E S ================================================= */

svg_layer::svg_layer():
	Layer_PasteCanvas(),
	filename("none")
{
}

bool
svg_layer::set_param(const String & param, const ValueBase &value)
{
	if(param=="filename"){
		Canvas::Handle canvas;
		//if ext of filename == "svg" then
		canvas=open_svg(value.get(String()),errors,warnings);
		//else other parsers maybe
		if(canvas){
			canvas->set_inline(get_canvas());
			set_sub_canvas(canvas);
			if(param=="filename" && value.same_type_as(filename))
			{
				value.put(&filename);
				return true;
			}
		}
	}
	return Layer_PasteCanvas::set_param(param,value);
}

ValueBase
svg_layer::get_param(const String &param)const
{
	if(param=="filename")
	{
		ValueBase ret(filename);
		return ret;
	}
	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_PasteCanvas::get_param(param);
}

Layer::Vocab
svg_layer::get_param_vocab()const
{
	Layer::Vocab ret(Layer_PasteCanvas::get_param_vocab());

	ret.push_back(ParamDesc("filename")
		.set_local_name(_("Filename"))
	);
	return ret;
}

