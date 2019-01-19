/* === S Y N F I G ========================================================= */
/*!	\file import.cpp
**	\brief Implementation of the "Import Image" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	Copyright (c) 2011-2013 Carlos López
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

#include "import.h"

#include <synfig/localization.h>
#include <synfig/general.h>

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/canvas.h>
#include <synfig/canvasfilenaming.h>
#include <synfig/filesystem.h>

#include <synfig/rendering/software/surfacesw.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace modules;
using namespace lyr_std;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Import);
SYNFIG_LAYER_SET_NAME(Import,"import");
SYNFIG_LAYER_SET_LOCAL_NAME(Import,N_("Import Image"));
SYNFIG_LAYER_SET_CATEGORY(Import,N_("Other"));
SYNFIG_LAYER_SET_VERSION(Import,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Import,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Import::Import():
	param_filename(ValueBase(String())),
	param_time_offset(ValueBase(Time(0)))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

Import::~Import()
{
}

void
Import::on_canvas_set()
{
	if(get_canvas())set_param("filename",param_filename);
}

bool
Import::set_param(const String & param, const ValueBase &value)
{
	try{
	IMPORT_VALUE(param_time_offset);

	IMPORT_VALUE_PLUS_BEGIN(param_filename)
	{
		if(!get_canvas() || !get_canvas()->get_file_system())
		{
			importer.reset();
			cimporter.reset();
			rendering_surface.reset();
			param_filename.set(value.get(String()));
			return true;
		}

		if (is_surface_modified())
		{
			error("Unable to load new file, already opened file is not saved");
			return false;
		}

		String filename = value.get(String());
		String fixed_filename = filename;

		// TODO: find source of this sreening of unicode characters
		// Get rid of any %20 crap
		for(String::size_type n; (n = fixed_filename.find("%20")) != String::npos;)
			fixed_filename.replace(n,3," ");

		String full_filename = CanvasFileNaming::make_full_filename(get_canvas()->get_file_name(), fixed_filename);
		if (full_filename.empty())
		{
			importer.reset();
			cimporter.reset();
			rendering_surface.reset();
			param_filename.set(filename);
			return true;
		}

		String independent_filename = CanvasFileNaming::make_canvas_independent_filename(get_canvas()->get_file_name(), full_filename);

		// If we are already loaded, don't reload
		// here we need something to force reload if file is changed
		if(this->independent_filename==independent_filename && importer)
		{
			param_filename.set(filename);
			return true;
		}

		this->independent_filename = independent_filename;

		handle<Importer> newimporter;
		newimporter = Importer::open(get_canvas()->get_file_system()->get_identifier(full_filename));

		if (!newimporter)
		{
			String local_filename = CanvasFileNaming::make_local_filename(get_canvas()->get_file_name(), full_filename);
			newimporter = Importer::open(get_canvas()->get_file_system()->get_identifier(local_filename));
			if(!newimporter)
			{
				error(strprintf("Unable to create an importer object with file \"%s\"", independent_filename.c_str()));
				importer.reset();
				cimporter.reset();
				param_filename.set(filename);
				rendering_surface.reset();
				return true;
			}
		}

		Time time_offset = param_time_offset.get(Time());
		Time time = (get_time_mark() == Time::end()) ? time_offset : get_time_mark() + time_offset;
		if (!newimporter->is_animated())
			time = Time(0);

		rendering_surface = new rendering::SurfaceResource(
			newimporter->get_frame(get_canvas()->rend_desc(), time) );
		importer=newimporter;
		param_filename.set(filename);

		return true;
	}
	IMPORT_VALUE_PLUS_END
	} catch(...) { return false; }

	return Layer_Bitmap::set_param(param,value);
}

ValueBase
Import::get_param(const String & param)const
{
	EXPORT_VALUE(param_time_offset);
	EXPORT_VALUE(param_filename);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Bitmap::get_param(param);
}

Layer::Vocab
Import::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Bitmap::get_param_vocab());

	ret.push_back(ParamDesc("filename")
		.set_local_name(_("Filename"))
		.set_description(_("File to import"))
		.set_hint("filename")
	);
	ret.push_back(ParamDesc("time_offset")
		.set_local_name(_("Time Offset"))
		.set_description(_("Time Offset to apply to the imported file"))
	);

	return ret;
}

void
Import::set_time_vfunc(IndependentContext context, Time time)const
{
	//Time time_offset = param_time_offset.get(Time());
	//if(get_amount() && importer && importer->is_animated())
	//	rendering_surface = new rendering::SurfaceResource(
	//		importer->get_frame(get_canvas()->rend_desc(), time+time_offset) );
	context.set_time(time);
}

void
Import::load_resources_vfunc(IndependentContext context, Time time)const
{
	Time time_offset=param_time_offset.get(Time());
	if(get_amount() && importer && importer->is_animated())
		rendering_surface = new rendering::SurfaceResource(
			importer->get_frame(get_canvas()->rend_desc(), time+time_offset) );
	context.load_resources(time);
}
