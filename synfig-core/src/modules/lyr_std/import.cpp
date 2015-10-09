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
#include <synfig/filesystemnative.h>

#include <synfig/rendering/software/surfacesw.h>

#endif

using namespace synfig;
using namespace std;
using namespace etl;

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

	String filename=param_filename.get(String());
	IMPORT_VALUE_PLUS_BEGIN(param_filename)
	{
		if(!get_canvas())
		{
			filename=value.get(filename);
			importer=0;
			cimporter=0;
			surface.clear();
			csurface.set_cairo_surface(NULL);
			param_filename.set(filename);
			return true;
		}

		String newfilename=value.get(string());
		String filename_with_path;

		// Get rid of any %20 crap
		{
			String::size_type n;
			while((n=newfilename.find("%20"))!=String::npos)
				newfilename.replace(n,3," ");
		}

		//if(get_canvas()->get_file_path()==dirname(newfilename))
		//{
		//	synfig::info("Image seems to be in local directory. Adjusting path...");
		//	newfilename=basename(newfilename);
		//}

#ifndef WIN32
		if(is_absolute_path(newfilename))
		{
			string curpath(cleanup_path(absolute_path(get_canvas()->get_file_path())));
			while(basename(curpath)==".")curpath=dirname(curpath);

			newfilename=relative_path(curpath,newfilename);
			synfig::info("basename(curpath)=%s, Path adjusted to %s",basename(curpath).c_str(),newfilename.c_str());
		}
#endif

		// TODO: "images" and "container:" literals
		String newfilename_orig = newfilename;
		if (newfilename_orig.substr(0, String("#").size()) == "#")
			newfilename_orig = "#images/" + newfilename_orig.substr(String("#").size());

		if(filename.empty())
			filename=newfilename;

		if(newfilename.empty())
		{
			filename=newfilename;
			importer=0;
			cimporter=0;
			surface.clear();
			csurface.set_cairo_surface(NULL);
			param_filename.set(filename);
			return true;
		}

		switch (get_method())
		{
		
		case SOFTWARE:
			{
				// If we are already loaded, don't reload
				if(filename==newfilename && importer)
				{
					synfig::warning(strprintf(_("Filename seems to already be set to \"%s\" (%s)"),filename.c_str(),newfilename.c_str()));
					return true;
				}

				assert(get_canvas());

				FileSystem::Handle file_system = get_canvas()->get_identifier().file_system;
				if (!file_system) file_system = FileSystemNative::instance();

				// todo: literal "container:"
				if(is_absolute_path(newfilename_orig)
				|| newfilename_orig.substr(0, std::string("#").size())=="#")
					filename_with_path=newfilename_orig;
				else
					filename_with_path=absolute_path(get_canvas()->get_file_path()+ETL_DIRECTORY_SEPARATOR+newfilename_orig);

				handle<Importer> newimporter;

				newimporter=Importer::open(file_system->get_identifier(filename_with_path));

				if(!newimporter)
				{
					newimporter=Importer::open(file_system->get_identifier(get_canvas()->get_file_path()+ETL_DIRECTORY_SEPARATOR+basename(newfilename_orig)));
					if(!newimporter)
					{
						synfig::error(strprintf("Unable to create an importer object with file \"%s\"",filename_with_path.c_str()));
						importer=0;
						filename=newfilename;
						abs_filename=filename_with_path;
						surface.clear();
						param_filename.set(filename);
						return false;
					}
				}

				surface.clear();
				if(!newimporter->get_frame(surface,get_canvas()->rend_desc(),Time(0),trimmed,width,height,top,left))
				{
					synfig::warning(strprintf("Unable to get frame from \"%s\"",filename_with_path.c_str()));
				}

				rendering_surface = new rendering::SurfaceSW();
				rendering_surface->assign(surface[0], surface.get_w(), surface.get_h());

				importer=newimporter;
				filename=newfilename;
				abs_filename=filename_with_path;
				param_filename.set(filename);

				return true;
			}
		case OPENGL:
			{
				return false;
			}
		case CAIRO:
			{
				
				if(filename==newfilename && cimporter)
				{
					synfig::warning(strprintf(_("Filename seems to already be set to \"%s\" (%s)"),filename.c_str(),newfilename.c_str()));
					return true;
				}
				assert(get_canvas());
				 
				FileSystem::Handle file_system = get_canvas()->get_identifier().file_system;
				if (!file_system) file_system = FileSystemNative::instance();

				// todo: literal "container:"
				if(is_absolute_path(newfilename_orig)
				|| newfilename_orig.substr(0, std::string("#").size())=="#")
					filename_with_path=newfilename_orig;
				else
					filename_with_path=absolute_path(get_canvas()->get_file_path()+ETL_DIRECTORY_SEPARATOR+newfilename_orig);
				 
				handle<CairoImporter> newimporter;
				 
				newimporter=CairoImporter::open(file_system->get_identifier(filename_with_path));
				 
				if(!newimporter)
				{
					newimporter=CairoImporter::open(file_system->get_identifier(get_canvas()->get_file_path()+ETL_DIRECTORY_SEPARATOR+basename(newfilename_orig)));
					if(!newimporter)
					{
						synfig::error(strprintf("Unable to create an importer object with file \"%s\"",filename_with_path.c_str()));
						cimporter=0;
						filename=newfilename;
						abs_filename=filename_with_path;
						csurface.set_cairo_surface(NULL);
						param_filename.set(filename);
						return false;
					}
				}
				 
				cairo_surface_t* cs;
				if(!newimporter->get_frame(cs, get_canvas()->rend_desc(), Time(0), trimmed, width, height, top, left))
				{
					synfig::warning(strprintf("Unable to get frame from \"%s\"",filename_with_path.c_str()));
				}
				set_cairo_surface(cs);
				cairo_surface_destroy(cs);
				 
				cimporter=newimporter;
				filename=newfilename;
				abs_filename=filename_with_path;
				param_filename.set(filename);

				return true;
				
				//return false;
			}
		}
	}
	IMPORT_VALUE_PLUS_END
	} catch(...) { set_amount(0); return false; }

	return Layer_Bitmap::set_param(param,value);
}

ValueBase
Import::get_param(const String & param)const
{
	EXPORT_VALUE(param_time_offset);

	if(get_canvas())
	{
		if(param=="filename")
		{
			ValueBase ret(type_string);
			// This line is needed to copy the internals of ValueBase from param_filename
			ret=param_filename;
			
			// todo: literal "container:" and "images"
			if(ret.get(String()).substr(0, std::string("#").size())!="#") {
				string curpath(cleanup_path(absolute_path(get_canvas()->get_file_path())));
				ret=relative_path(curpath,abs_filename);
			} else
			if(ret.get(String()).substr(0, std::string("#images/").size())=="#images/") {
				ret = "#" + ret.get(String()).substr(std::string("#images/").size());
			}

			return ret;
		}
	}
	else
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
Import::set_time(IndependentContext context, Time time)const
{
	Time time_offset=param_time_offset.get(Time());
	switch (get_method())
	{
	case SOFTWARE:
		if(get_amount() && importer &&
		   importer->is_animated())
			importer->get_frame(surface,get_canvas()->rend_desc(),time+time_offset,trimmed,width,height,top,left);
		break;
	case OPENGL:
		break;
	case CAIRO:
		{

			if(get_amount() && cimporter &&
			   cimporter->is_animated())
			{
				cairo_surface_t* cs;
				cimporter->get_frame(cs, get_canvas()->rend_desc(), time+time_offset, trimmed, width, height, top, left);
				if(cs)
				{
					csurface.set_cairo_surface(cs);
					csurface.map_cairo_image();
					cairo_surface_destroy(cs);
				}
			}
			break;

		}
	
	}
	context.set_time(time);
}

void
Import::set_time(IndependentContext context, Time time, const Point &pos)const
{
	Time time_offset=param_time_offset.get(Time());
	switch (get_method())
	{
		case SOFTWARE:
			if(get_amount() && importer &&
			   importer->is_animated())
				importer->get_frame(surface,get_canvas()->rend_desc(),time+time_offset,trimmed,width,height,top,left);
			break;
		case OPENGL:
			break;
		case CAIRO:
		{

			if(get_amount() && cimporter &&
			   cimporter->is_animated())
			{
				cairo_surface_t* cs;
				cimporter->get_frame(cs, get_canvas()->rend_desc(), time+time_offset, trimmed, width, height, top, left);
				if(cs)
				{
					csurface.set_cairo_surface(cs);
					csurface.map_cairo_image();
					cairo_surface_destroy(cs);
				}
			}
			break;

		}
			
	}
	context.set_time(time,pos);
}

void
Import::set_render_method(Context context, RenderMethod x)
{
	if(get_method() != x) // if the method is different
	{
		Layer_Bitmap::set_render_method(context, x); // set the method (and pass to the other layers)
		importer=0; // invalidate the importer
		cimporter=0;
		set_param("filename", param_filename); // this will update the importer to the new type
	}
	else
		context.set_render_method(x); // pass it down.
}
