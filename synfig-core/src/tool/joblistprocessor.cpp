/* === S Y N F I G ========================================================= */
/*!	\file tool/joblistprocessor.cpp
**	\brief Synfig Tool Rendering Job List Processor Class
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2009-2015 Diego Barrios Romero
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <iostream>
#include <algorithm>
#include <errno.h>
#include <cstring>

#include <chrono>

#include <autorevision.h>
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/target.h>
#include <synfig/target_scanline.h>
#include <synfig/target_tile.h>
#include <synfig/savecanvas.h>
#include <synfig/filesystemnative.h>

#include "definitions.h"
#include "synfigtoolexception.h"
#include "renderprogress.h"
#include "joblistprocessor.h"

#include <giomm/file.h>
#include <glib/gstdio.h>

#endif

// MSVC
#ifndef W_OK
#define W_OK 2
#endif

using namespace synfig;

void process_job_list(std::list<Job>& job_list, const TargetParam& target_params)
{
	if (job_list.empty())
		throw (SynfigToolException(SYNFIGTOOL_BORED, _("Nothing to do!")));

	for(; !job_list.empty(); job_list.pop_front())
	{
		if (setup_job(job_list.front(), target_params))
			process_job(job_list.front());
	}
}

std::string get_extension(const std::string &filename)
{
	std::size_t found = filename.rfind('.');
	if (found == std::string::npos) return ""; // extension not found

	return filename.substr(found);
}

std::string replace_extension(const std::string &filename, const std::string &new_extension)
{
	std::size_t found = filename.rfind('.');
	if (found == std::string::npos) return filename + "." + new_extension; // extension not found
	
	return filename.substr(0, found) + "." + new_extension;
}

std::string get_absolute_path(std::string relative_path) {
  Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(relative_path);
  return file->get_path();
}

void try_to_determine_target_from_outfile(Job& job)
{
	VERBOSE_OUT(3) << _("Target name undefined, attempting to figure it out")
				   << std::endl;
	std::string ext = get_extension(job.outfilename);
	if (ext.length())
		ext = ext.substr(1);

	if(Target::ext_book().count(ext))
	{
		job.target_name = Target::ext_book()[ext];
		info("target name not specified - using %s", job.target_name.c_str());
	}
	else
	{
		std::string lower_ext(ext);
		strtolower(lower_ext);

		if(Target::ext_book().count(lower_ext))
		{
			job.target_name=Target::ext_book()[lower_ext];
			info("target name not specified - using %s", job.target_name.c_str());
		}
		else
			job.target_name=ext;
	}
}

void set_default_target(Job& job)
{
	if(job.target_name.empty())
	{
		VERBOSE_OUT(2) << _("Defaulting to PNG target...") << std::endl;
		job.target_name = "png";
	}
}

bool setup_job(Job& job, const TargetParam& target_parameters)
{
	VERBOSE_OUT(4) << _("Attempting to determine target/outfile...") << std::endl;

	// If the target type is not yet defined,
	// try to figure it out from the outfile.
	if (job.target_name.empty() && !job.outfilename.empty()) {
		try_to_determine_target_from_outfile(job);
	}

	// If the target type is STILL not yet defined, then
	// set it to a some sort of default
	set_default_target(job);

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

        //job.outfilename = bfs::path(job.filename).replace_extension(new_extension).string();
		job.outfilename = replace_extension(job.filename, new_extension);
	}

	VERBOSE_OUT(4) << "Target name = " << job.target_name.c_str() << std::endl;
	VERBOSE_OUT(4) << "Outfilename = " << job.outfilename.c_str() << std::endl;

	// Check permissions
	//if (access(bfs::canonical(bfs::path(job.outfilename).parent_path()).string().c_str(), W_OK) == -1)
	// az: fixme
	if (g_access(get_absolute_path(job.outfilename + "/../").c_str(), W_OK) == -1)
	{
	    /*const std::string message =
            (boost::format(_("Unable to create output for \"%s\": %s"))
                           % job.filename % strerror(errno)).str();
		synfig::error(message.c_str());*/
		synfig::error(_("Unable to create output for \"%s\": %s"), job.filename.c_str(), strerror(errno));
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
		    /*const std::string message =
                (boost::format(_("Unknown target for \"%s\": %s"))
                               % job.filename % strerror(errno)).str();
		    synfig::error(message.c_str());*/

			synfig::error(_("Unknown target for \"%s\": %s"), job.filename.c_str(), strerror(errno));
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

	// Set the threads and render engine for the target
	if (job.target)
	{
		if(auto scanline_target = Target_Scanline::Handle::cast_dynamic(job.target))
		{
			scanline_target->set_threads(SynfigToolGeneralOptions::instance()->get_threads());
			scanline_target->set_engine(job.render_engine);
		} else if(auto tile_target = Target_Tile::Handle::cast_dynamic(job.target))
		{
			tile_target->set_threads(SynfigToolGeneralOptions::instance()->get_threads());
			tile_target->set_engine(job.render_engine);
		}
	}

	return true;
}

void print_job_info(const Job& job) {
	VERBOSE_OUT(3) << job.filename.c_str() << " -- " << std::endl;
	synfig::info("\tw: %d, h: %d, a: %d, pxaspect: %f, imaspect: %f, span: %f",
				 job.desc.get_w(), job.desc.get_h(), job.desc.get_antialias(),
				 job.desc.get_pixel_aspect(), job.desc.get_image_aspect(), job.desc.get_span());

	synfig::info("\ttl: [%f,%f], br: [%f,%f], focus: [%f,%f]",
				 job.desc.get_tl()[0], job.desc.get_tl()[1],
				 job.desc.get_br()[0], job.desc.get_br()[1],
				 job.desc.get_focus()[0], job.desc.get_focus()[1]
	);
}

void save_canvas_to_file(const std::string& filename, const synfig::Canvas::Handle& canvas) {
	// todo: support containers
	if(!synfig::save_canvas(FileSystemNative::instance()->get_identifier(filename), canvas)) {
		throw (SynfigToolException(SYNFIGTOOL_RENDERFAILURE, _("Render Failure.")));
	}
}

void render_job(const Job& job, RenderProgress& progress, bool should_print_benchmarks, int repeats) {
	double total_duration = 0.f;

	for(int i = 0; i < repeats; i++)
	{
		std::chrono::steady_clock::time_point start_timepoint =
				std::chrono::steady_clock::now();

		// Call the render member of the target
		if(!job.target->render(&progress)) {
			throw (SynfigToolException(SYNFIGTOOL_RENDERFAILURE, _("Render Failure.")));
		}

		if(should_print_benchmarks)	{
			std::chrono::duration<double, std::milli> duration =
					std::chrono::steady_clock::now() - start_timepoint;

			total_duration += duration.count();
		}
	}

	if(should_print_benchmarks)
	{
		std::cout << job.filename.c_str()
				  << _(": Rendered ")
				  << repeats
				  << _( " times in ")
				  << total_duration
				  << _(" ms.")
				  << _(" Average time per render: ")
				  << total_duration / repeats
				  << _(" ms.") << std::endl;
	}
}

void process_job (Job& job)
{
	print_job_info(job);

	RenderProgress p;
	p.task(job.filename + " ==> " + job.outfilename);

	if(job.sifout)
	{
		save_canvas_to_file(job.outfilename, job.canvas);
	}
	else
	{
		VERBOSE_OUT(1) << _("Rendering...") << std::endl;

		bool should_print_benchmarks = SynfigToolGeneralOptions::instance()->should_print_benchmarks();
		int repeats = SynfigToolGeneralOptions::instance()->get_repeats();

		render_job(job, p, should_print_benchmarks, repeats);
	}

	VERBOSE_OUT(1) << _("Done.") << std::endl;
}

