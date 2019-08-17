/* === S Y N F I G ========================================================= */
/*!	\file tool/main.cpp
**	\brief SYNFIG Tool
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <iostream>
#include <string>
#include <list>

//#include <boost/program_options/options_description.hpp>
//#include <boost/program_options/parsers.hpp>
//#include <boost/program_options/variables_map.hpp>
#include <boost/tokenizer.hpp>
#include <boost/token_functions.hpp>
//#include <boost/format.hpp>

#include <glibmm.h>

#include <synfig/localization.h>
#include <synfig/general.h>

#include <synfig/module.h>
#include <synfig/importer.h>
#include <synfig/cairoimporter.h>
#include <synfig/layer.h>
#include <synfig/canvas.h>
#include <synfig/target.h>
#include <synfig/targetparam.h>
#include <synfig/string.h>
#include <synfig/paramdesc.h>
#include <synfig/main.h>
#include <autorevision.h>
#include "definitions.h"
#include "progress.h"
#include "job.h"
#include "synfigtoolexception.h"
#include "optionsprocessor.h"
#include "joblistprocessor.h"
#include "printing_functions.h"

//#include "named_type.h"
#endif

//namespace po=boost::program_options;

std::string _appendAlphaToFilename(std::string input_filename)
{

	std::size_t found = input_filename.rfind(".");
	if (found == std::string::npos) return input_filename + "-alpha"; // extension not found, just add to the end
	
	return input_filename.substr(0, found) + "-alpha" + input_filename.substr(found);

    /*bfs::path filename(input_filename);
    bfs::path alpha_filename(filename.stem().string() + "-alpha" +
        filename.extension().string());
    return bfs::path(filename.parent_path() / alpha_filename).string();*/
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "");
	Glib::init(); // need to use Gio functions before app is started

	SynfigToolGeneralOptions::create_singleton_instance(argv[0]);

	std::string binary_path =
		SynfigToolGeneralOptions::instance()->get_binary_path();

#ifdef ENABLE_NLS
	/*bst::filesystem::path locale_path =
		binary_path.parent_path().parent_path();*/
	std::string locale_path = get_absolute_path(binary_path + "../../share/locale");
	//locale_path = locale_path/"share"/"locale";
	//bindtextdomain("synfig", Glib::locale_from_utf8(locale_path.string()).c_str() );
	bindtextdomain("synfig", Glib::locale_from_utf8(locale_path).c_str() );
	bind_textdomain_codeset("synfig", "UTF-8");
	textdomain("synfig");
#endif

	if(!SYNFIG_CHECK_VERSION())
	{
		std::cerr << _("FATAL: Synfig Version Mismatch") << std::endl;
		return SYNFIGTOOL_BADVERSION;
	}

	try
	{
		if(argc == 1)
		{
			throw (SynfigToolException(SYNFIGTOOL_MISSINGARGUMENT));
		}


/*		named_type<std::string>* target_arg_desc = new named_type<std::string>("module");
		named_type<int>* width_arg_desc = new named_type<int>("NUM");
		named_type<int>* height_arg_desc = new named_type<int>("NUM");
		named_type<int>* span_arg_desc = new named_type<int>("NUM");
		named_type<int>* antialias_arg_desc = new named_type<int>("1..30");
		named_type<int>* quality_arg_desc = new named_type<int>("0..10");
		named_type<float>* gamma_arg_desc = new named_type<float>("NUM (=2.2)");
		named_type<int>* threads_arg_desc = new named_type<int>("NUM");
		named_type<int>* verbosity_arg_desc = new named_type<int>("NUM");
		named_type<std::string>* canvas_arg_desc = new named_type<std::string>("canvas-id");
		named_type<std::string>* output_file_arg_desc = new named_type<std::string>("filename");
		named_type<std::string>* input_file_arg_desc = new named_type<std::string>("filename");
		named_type<float>* fps_arg_desc = new named_type<float>("NUM");
		named_type<std::string>* time_arg_desc = new named_type<std::string>("seconds");
		named_type<std::string>* begin_time_arg_desc = new named_type<std::string>("seconds");
		named_type<std::string>* start_time_arg_desc = new named_type<std::string>("seconds");
		named_type<std::string>* end_time_arg_desc = new named_type<std::string>("seconds");
		named_type<int>* dpi_arg_desc = new named_type<int>("NUM");
		named_type<int>* dpi_x_arg_desc = new named_type<int>("NUM");
		named_type<int>* dpi_y_arg_desc = new named_type<int>("NUM");
		named_type<std::string>* append_filename_arg_desc = new named_type<std::string>("filename");
		named_type<std::string>* sequence_separator_arg_desc = new named_type<std::string>("string");
		named_type<std::string>* canvas_info_fields_arg_desc = new named_type<std::string>("fields");
		named_type<std::string>* layer_info_field_arg_desc = new named_type<std::string>("layer-name");
		named_type<std::string>* video_codec_arg_desc = new named_type<std::string>("codec");
		named_type<int>* video_bitrate_arg_desc = new named_type<int>("bitrate");

        po::options_description po_settings(_("Settings"));
        po_settings.add_options()
			("target,t", target_arg_desc, _("Specify output target (Default: PNG)"))
            ("width,w", width_arg_desc, _("Set the image width in pixels (Use zero for file default)"))
            ("height,h", height_arg_desc, _("Set the image height in pixels (Use zero for file default)"))
            ("span,s", span_arg_desc, _("Set the diagonal size of image window (Span)"))
            ("antialias,a", antialias_arg_desc, _("Set antialias amount for parametric renderer."))
            ("quality,Q", quality_arg_desc->default_value(DEFAULT_QUALITY), etl::strprintf(_("Specify image quality for accelerated renderer (Default: %d)"), DEFAULT_QUALITY).c_str())
            ("gamma,g", gamma_arg_desc, _("Gamma"))
            ("threads,T", threads_arg_desc, _("Enable multithreaded renderer using the specified number of threads"))
            ("input-file,i", input_file_arg_desc, _("Specify input filename"))
            ("output-file,o", output_file_arg_desc, _("Specify output filename"))
            ("sequence-separator", sequence_separator_arg_desc, _("Output file sequence separator string (Use double quotes if you want to use spaces)"))
            ("canvas,c", canvas_arg_desc, _("Render the canvas with the given id instead of the root."))
            ("fps", fps_arg_desc, _("Set the frame rate"))
			("time", time_arg_desc, _("Render a single frame at <seconds>"))
			("begin-time", begin_time_arg_desc, _("Set the starting time"))
			("start-time", start_time_arg_desc, _("Set the starting time"))
			("end-time", end_time_arg_desc, _("Set the ending time"))
			("dpi", dpi_arg_desc, _("Set the physical resolution (Dots-per-inch)"))
			("dpi-x", dpi_x_arg_desc, _("Set the physical X resolution (Dots-per-inch)"))
			("dpi-y", dpi_y_arg_desc, _("Set the physical Y resolution (Dots-per-inch)"))
            ;

        po::options_description po_switchopts(_("Switch options"));
        po_switchopts.add_options()
            ("verbose,v", verbosity_arg_desc, _("Output verbosity level"))
            ("quiet,q", _("Quiet mode (No progress/time-remaining display)"))
            ("benchmarks,b", _("Print benchmarks"))
            ("extract-alpha,x", _("Extract alpha"))
            ;

        po::options_description po_misc(_("Misc options"));
        po_misc.add_options()
			("append", append_filename_arg_desc, _("Append layers in <filename> to composition"))
            ("canvas-info", canvas_info_fields_arg_desc, _("Print out specified details of the root canvas"))
            ("canvases", _("Print out the list of exported canvases in the composition"))
            ;

        po::options_description po_ffmpeg(_("FFMPEG target options"));
        po_ffmpeg.add_options()
			("video-codec", video_codec_arg_desc, _("Set the codec for the video. See --ffmpeg-video-codecs"))
            ("video-bitrate", video_bitrate_arg_desc, _("Set the bitrate for the output video"))
            ;

        po::options_description po_info(_("Synfig info options"));
        po_info.add_options()
			("help", _("Produce this help message"))
            ("importers", _("Print out the list of available importers"))
			("info", _("Print out misc build information"))
            ("layers", _("Print out the list of available layers"))
            ("layer-info", layer_info_field_arg_desc, _("Print out layer's description, parameter info, etc."))
            ("license", _("Print out license information"))
            ("modules", _("Print out the list of loaded modules"))
            ("targets", _("Print out the list of available targets"))
            ("target-video-codecs", _("Print out the list of available video codecs when encoding through FFMPEG"))
            ("valuenodes", _("Print out the list of available ValueNodes"))
            ("version", _("Print out version information"))
            ;

        po::options_description po_hidden("");
        po_hidden.add_options()
			("list-canvases", _("Print out the list of exported canvases in the composition"))
			;

#ifdef _DEBUG
        po::options_description po_debug(_("Synfig debug flags"));
        po_debug.add_options()
            ("guid-test", _("Test GUID generation"))
			("signal-test", _("Test signal implementation"))
            ;
#endif

		// The first positional option will be treated as the input file
		po::positional_options_description po_positional;
		po_positional.add("input-file", 1);

        // Declare an options description instance which will include
        // all the options
        po::options_description po_all("");
        po_all.add(po_settings).add(po_switchopts).add(po_misc).add(po_info).add(po_ffmpeg).add(po_hidden);

#ifdef _DEBUG
		po_all.add(po_debug);
#endif

        // Declare an options description instance which will be shown
        // to the user
        po::options_description po_visible("");
        po_visible.add(po_settings).add(po_switchopts).add(po_misc).add(po_ffmpeg);

#ifdef _DEBUG
		po_visible.add(po_debug);
#endif

		po_visible.add(po_info);

        po::variables_map vm;
        try{
            po::store(po::command_line_parser(argc, argv).options(po_all).
                    positional(po_positional).run(), vm);
        }catch(std::exception& e)
        {
            std::cout << std::endl << e.what() << std::endl;
            std::cout << _("Try 'synfig --help' for more information") << std::endl;
            return SYNFIGTOOL_UNKNOWNARGUMENT;
        }*/

        //OptionsProcessor op(vm, po_visible);
		SynfigCommandLineParser parser;
		parser.parse(argc, argv);

        // Switch options ---------------------------------------------
        parser.process_settings_options();

#ifdef _DEBUG
		// DEBUG options ----------------------------------------------
		parser.process_debug_options();
#endif

		// Trivial info options -----------------------------------------------
		parser.process_trivial_info_options();

		// Synfig Main initialization needs to be after verbose and
		// before any other where it's used
		Progress p(binary_path.c_str());
		//synfig::Main synfig_main(binary_path.parent_path().string(), &p);
		synfig::Main synfig_main(get_absolute_path(binary_path + "/.."), &p);

		// Info options -----------------------------------------------
		parser.process_info_options();

		std::list<Job> job_list;

		// Processing --------------------------------------------------
		Job job;
		job = parser.extract_job();
		job.desc = job.canvas->rend_desc() = parser.extract_renddesc(job.canvas->rend_desc());

		if (job.extract_alpha) {
			job.alpha_mode = synfig::TARGET_ALPHA_MODE_REDUCE;
			job_list.push_front(job);
			job.alpha_mode = synfig::TARGET_ALPHA_MODE_EXTRACT;
			job.outfilename = _appendAlphaToFilename(job.outfilename);
			job_list.push_front(job);
		} else {
			job_list.push_front(job);
		}

		process_job_list(job_list, parser.extract_targetparam());

		return SYNFIGTOOL_OK;

    }
    catch (SynfigToolException& e) {
    	exit_code code = e.get_exit_code();
    	if (code != SYNFIGTOOL_HELP && code != SYNFIGTOOL_OK)
    		std::cerr << e.get_message().c_str() << std::endl;
    	if (code == SYNFIGTOOL_MISSINGARGUMENT)
    		print_usage();

    	return code;
    }
    catch(std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}
