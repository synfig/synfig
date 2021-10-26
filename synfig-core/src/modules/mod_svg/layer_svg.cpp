/* === S Y N F I G ========================================================= */
/*!	\file layer_svg.cpp
**	\brief Implementation of the Svg layer
**
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2009 Carlos A. Sosa Navarro
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

#include <synfig/localization.h>

#include <synfig/canvasfilenaming.h>
#include <synfig/string.h>
#include <synfig/paramdesc.h>
#include <synfig/value.h>

#include "layer_svg.h"

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(svg_layer);
SYNFIG_LAYER_SET_NAME(svg_layer,"svg_layer");
SYNFIG_LAYER_SET_LOCAL_NAME(svg_layer,N_("Import Svg"));
SYNFIG_LAYER_SET_CATEGORY(svg_layer,CATEGORY_DO_NOT_USE);//Hide this layer in the menu
SYNFIG_LAYER_SET_VERSION(svg_layer,"0.1");

/* === P R O C E D U R E S ================================================= */

svg_layer::svg_layer():
	Layer_Group(),
	filename("none")
{
}

bool
svg_layer::set_param(const String & param, const ValueBase &value)
{
	if(param=="filename"){
		Canvas::Handle canvas;
		//if ext of filename == "svg" then
		filename = value.get(String());
		canvas=open_svg(CanvasFileNaming::make_full_filename(get_canvas()->get_file_name(), filename),errors,warnings);
		//else other parsers maybe
		if(canvas)
			canvas->set_inline(get_canvas());
		set_sub_canvas(canvas);
		return true;
	}
	return Layer_Group::set_param(param,value);
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

	return Layer_Group::get_param(param);
}

Layer::Vocab
svg_layer::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Group::get_param_vocab());

	ret.push_back(ParamDesc("filename")
		.set_local_name(_("Filename"))
		.set_hint("filename")
	);
	return ret;
}

