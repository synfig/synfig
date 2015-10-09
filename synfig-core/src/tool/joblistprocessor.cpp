/* === S Y N F I G ========================================================= */
/*!	\file tool/joblistprocessor.cpp
**	\brief Synfig Tool Rendering Job List Processor Class
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2009-2015 Diego Barrios Romero
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

#include <boost/filesystem.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/format.hpp>
#include <boost/chrono.hpp>

#include <autorevision.h>
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/canvas.h>
#include <synfig/target.h>
#include <synfig/layer.h>
#include <synfig/time.h>
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

using namespace synfig;
namespace bfs=boost::filesystem;

void process_job_list(std::list<Job>& job_list, const TargetParam& target_params)
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
	VERBOSE_OUT(4) << _("Attempting to determine target/outfile...") << std::endl;

	// If the target type is not yet defined,
	// try to figure it out from the outfile.
	if(job.target_name.empty() && !job.outfilename.empty())
	{
		VERBOSE_OUT(3) << _("Target name undefined, attempting to figure it out")
					   << std::endl;
		std::string ext = bfs::path(job.outfilename).extension().string();
		if (ext.length())
			ext = ext.substr(1);

		if(Target::ext_book().count(ext))
		{
			job.target_name = Target::ext_book()[ext];
			info("target name not specified - using %s", job.target_name.c_str());
		}
		else
		{
			std::string lower_ext;
			std::transform(ext.begin(), ext.end(), std::back_inserter(lower_ext), ::tolower);

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
		VERBOSE_OUT(2) << _("Defaulting to PNG target...") << std::endl;
		job.target_name = "png";
	}

	// If no output filename was provided, then create a output filename
	// based on the given input filename and the selected target.
	// (ie: change the extension)
	if(job.outfilename.empty())
	{
        std::string new_extension;
		if(Target::book().count(job.target_name))
			new_extension = Target::book()[job.target_name].filename;
		else
			new_extension = job.target_name;

        job.outfilename = bfs::path(job.filename).replace_extension(new_extension).string();
	}

	VERBOSE_OUT(4) << "Target name = " << job.target_name.c_str() << std::endl;
	VERBOSE_OUT(4) << "Outfilename = " << job.outfilename.c_str() << std::endl;

	// Check permissions
	if (access(bfs::canonical(bfs::path(job.outfilename).parent_path()).string().c_str(), W_OK) == -1)
	{
	    const std::string message =
            (boost::format(_("Unable to create output for \"%s\": %s"))
                           % job.filename % strerror(errno)).str();
		synfig::error(message.c_str());
		synfig::error(_("Throwing out job..."));
		return false;
	}

	VERBOSE_OUT(4) << _("Creating the target...") << std::endl;
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
		    const std::string message =
                (boost::format(_("Unknown target for \"%s\": %s"))
                               % job.filename % strerror(errno)).str();
		    synfig::error(message.c_str());
                    synfig::error(_("Throwing out job..."));
                    return false;
		}

		job.sifout=false;
	}

	// Set the Canvas on the Target
	if(job.target)
	{
		VERBOSE_OUT(4) << _("Setting the canvas on the target...") << std::endl;
		job.target->set_canvas(job.canvas);

		VERBOSE_OUT(4) << _("Setting the quality of the target...") << std::endl;
		job.target->set_quality(job.quality);

		if (job.alpha_mode!=TARGET_ALPHA_MODE_KEEP)
		{
			VERBOSE_OUT(4) << _("Setting the alpha mode of the target...") << std::endl;
			job.target->set_alpha_mode(job.alpha_mode);
		}
	}

	// Set the threads for the target
	if (job.target && Target_Scanline::Handle::cast_dynamic(job.target))
		Target_Scanline::Handle::cast_dynamic(job.target)->set_threads(SynfigToolGeneralOptions::instance()->get_threads());

	return true;
}

void process_job (Job& job)
{
	VERBOSE_OUT(3) << job.filename.c_str() << " -- " << std::endl;
	VERBOSE_OUT(3) << '\t'
				   << boost::format("w:%d, h:%d, a:%d, pxaspect:%f, imaspect:%f, span:%f")
                                    % job.desc.get_w()
                                    % job.desc.get_h()
                                    % job.desc.get_antialias()
                                    % job.desc.get_pixel_aspect()
                                    % job.desc.get_image_aspect()
                                    % job.desc.get_span()
                    << std::endl;

	VERBOSE_OUT(3) << '\t'
				   << boost::format("tl:[%f,%f], br:[%f,%f], focus:[%f,%f]")
                                    % job.desc.get_tl()[0]
                                    % job.desc.get_tl()[1]
                                    % job.desc.get_br()[0]
                                    % job.desc.get_br()[1]
                                    % job.desc.get_focus()[0]
                                    % job.desc.get_focus()[1]
                    << std::endl;

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
		VERBOSE_OUT(1) << _("Rendering...") << std::endl;
		boost::chrono::system_clock::time_point start_timepoint =
            boost::chrono::system_clock::now();

		// Call the render member of the target
		if(!job.target->render(&p))
			throw (SynfigToolException(SYNFIGTOOL_RENDERFAILURE, _("Render Failure.")));

		if(SynfigToolGeneralOptions::instance()->should_print_benchmarks())
        {
            boost::chrono::duration<double> duration =
                boost::chrono::system_clock::now() - start_timepoint;

            std::cout << job.filename.c_str()
                      << _(": Rendered in ")
                      << duration.count()
                      << _(" seconds.") << std::endl;
        }
	}

	VERBOSE_OUT(1) << _("Done.") << std::endl;
}

