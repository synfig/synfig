/* === S Y N F I G ========================================================= */
/*!	\file tool/joblistprocessor.cpp
**	\brief Synfig Tool Rendering Job List Processor Class
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2009-2012 Diego Barrios Romero
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <iostream>
#include <list>
#include <algorithm>
#include <errno.h>
#include <cstring>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include <ETL/stringf>
#include <ETL/clock>
#include <autorevision.h>
#include <synfig/general.h>
#include <synfig/canvas.h>
#include <synfig/target.h>
#include <synfig/layer.h>
#include <synfig/time.h>
#include <synfig/targetparam.h>
#include <synfig/target_scanline.h>
#include <synfig/paramdesc.h>
#include <synfig/module.h>
#include <synfig/importer.h>
#include <synfig/loadcanvas.h>
#include <synfig/savecanvas.h>
#include <synfig/filesystemnative.h>

#include "definitions.h"
#include "job.h"
#include "synfigtoolexception.h"
#include "printing_functions.h"
#include "renderprogress.h"
#include "joblistprocessor.h"

#endif

using namespace std;
using namespace synfig;
using namespace etl;

void process_job_list(list<Job>& job_list, const TargetParam& target_params) throw (SynfigToolException&)
{
	if(!job_list.size())
		throw (SynfigToolException(SYNFIGTOOL_BORED, _("Nothing to do!")));

	for(; job_list.size(); job_list.pop_front())
	{
		if (setup_job(job_list.front(), target_params))
			process_job(job_list.front());
	}
}

bool setup_job(Job& job, const TargetParam& target_parameters)
{
	VERBOSE_OUT(4) << _("Attempting to determine target/outfile...") << endl;

	// If the target type is not yet defined,
	// try to figure it out from the outfile.
	if(job.target_name.empty() && !job.outfilename.empty())
	{
		VERBOSE_OUT(3) << _("Target name undefined, attempting to figure it out")
					   << endl;
		string ext = filename_extension(job.outfilename);
		if (ext.length())
			ext = ext.substr(1);

		if(Target::ext_book().count(ext))
		{
			job.target_name = Target::ext_book()[ext];
			info("target name not specified - using %s", job.target_name.c_str());
		}
		else
		{
			string lower_ext(ext);

			for(unsigned int i = 0; i < ext.length(); i++)
				lower_ext[i] = tolower(ext[i]);

			if(Target::ext_book().count(lower_ext))
			{
				job.target_name=Target::ext_book()[lower_ext];
				info("target name not specified - using %s", job.target_name.c_str());
			}
			else
				job.target_name=ext;
		}
	}

	// If the target type is STILL not yet defined, then
	// set it to a some sort of default
	if(job.target_name.empty())
	{
		VERBOSE_OUT(2) << _("Defaulting to PNG target...") << endl;
		job.target_name = "png";
	}

	// If no output filename was provided, then create a output filename
	// based on the given input filename and the selected target.
	// (ie: change the extension)
	if(job.outfilename.empty())
	{
		job.outfilename =
			filename_sans_extension(job.filename) + '.';

		if(Target::book().count(job.target_name))
			job.outfilename +=
				Target::book()[job.target_name].filename;
		else
			job.outfilename += job.target_name;
	}

	VERBOSE_OUT(4) << "Target name = " << job.target_name.c_str() << endl;
	VERBOSE_OUT(4) << "Outfilename = " << job.outfilename.c_str() << endl;

	// Check permissions
	if (access(dirname(job.outfilename).c_str(), W_OK) == -1)
	{
		VERBOSE_OUT(1) << _("Unable to create ouput for \"") << job.filename.c_str()
						<< "\": " << strerror(errno) << endl
					   << _("Throwing out job...") << endl;
		return false;
	}

	VERBOSE_OUT(4) << _("Creating the target...") << endl;
	job.target =
		synfig::Target::create(job.target_name,
							   job.outfilename,
							   target_parameters);

	if(job.target_name == "sif")
		job.sifout=true;
	else
	{
		if(!job.target)
		{
			VERBOSE_OUT(1) << _("Unknown target for \"") << job.filename.c_str()
						   << "\": " << strerror(errno) << endl
						   << _("Throwing out job...") << endl;
			return false;
		}

		job.sifout=false;
	}

	// Set the Canvas on the Target
	if(job.target)
	{
		VERBOSE_OUT(4) << _("Setting the canvas on the target...") << endl;
		job.target->set_canvas(job.canvas);

		VERBOSE_OUT(4) << _("Setting the quality of the target...") << endl;
		job.target->set_quality(job.quality);
	}

	// Set the threads for the target
	if (job.target &&
		Target_Scanline::Handle::cast_dynamic(job.target))
		Target_Scanline::Handle::cast_dynamic(job.target)->set_threads(threads);

	return true;
}

void process_job (Job& job) throw (SynfigToolException&)
{
	VERBOSE_OUT(3) << job.filename.c_str() << " -- " << endl;
	VERBOSE_OUT(3) << '\t'
				   <<
		strprintf("w:%d, h:%d, a:%d, pxaspect:%f, imaspect:%f, span:%f",
			job.desc.get_w(),
			job.desc.get_h(),
			job.desc.get_antialias(),
			job.desc.get_pixel_aspect(),
			job.desc.get_image_aspect(),
			job.desc.get_span()
			).c_str()
		<< endl;

	VERBOSE_OUT(3) << '\t'
				   <<
		strprintf("tl:[%f,%f], br:[%f,%f], focus:[%f,%f]",
			job.desc.get_tl()[0],
			job.desc.get_tl()[1],
			job.desc.get_br()[0],
			job.desc.get_br()[1],
			job.desc.get_focus()[0],
			job.desc.get_focus()[1]
			).c_str()
			<< endl;

	RenderProgress p;
	p.task(job.filename + " ==> " + job.outfilename);

	if(job.sifout)
	{
		// todo: support containers
		if(!save_canvas(FileSystemNative::instance()->get_identifier(job.outfilename), job.canvas))
			throw (SynfigToolException(SYNFIGTOOL_RENDERFAILURE, _("Render Failure.")));
	}
	else
	{
		VERBOSE_OUT(1) << _("Rendering...") << endl;
		etl::clock timer;
		timer.reset();

		// Call the render member of the target
		if(!job.target->render(&p))
			throw (SynfigToolException(SYNFIGTOOL_RENDERFAILURE, _("Render Failure.")));

		if(print_benchmarks)
			cout << job.filename.c_str()
				 << _(": Rendered in ") << timer()
				 << _(" seconds.") << endl;
	}

	VERBOSE_OUT(1) << _("Done.") << endl;
}
