/* === S Y N F I G ========================================================= */
/*!	\file tool/main.cpp
**	\brief SYNFIG Tool
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <iostream>
#include <ETL/stringf>
#include <list>
#include <cstring>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/tokenizer.hpp>
#include <boost/token_functions.hpp>

#include <glibmm.h>

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

#include "named_type.h"
#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace boost;
namespace po=boost::program_options;

/* === G L O B A L S ================================================ */

String binary_path;
int verbosity = 0;
bool be_quiet = false;
bool print_benchmarks = false;
int threads = 1;

//! Allowed video codecs
/*! \warning This variable is linked to allowed_video_codecs_description,
 *  if you change this you must change the other acordingly.
 *  \warning These codecs are linked to the filename extensions for
 *  mod_ffmpeg. If you change this you must change the others acordingly.
 */
const char* allowed_video_codecs[] =
{
	"flv", "h263p", "huffyuv", "libtheora", "libx264",
	"mjpeg", "mpeg1video", "mpeg2video", "mpeg4", "msmpeg4",
	"msmpeg4v1", "msmpeg4v2", "wmv1", "wmv2", NULL
};

//! Allowed video codecs description.
/*! \warning This variable is linked to allowed_video_codecs,
 *  if you change this you must change the other acordingly.
 */
const char* allowed_video_codecs_description[] =
{
	"Flash Video (FLV) / Sorenson Spark / Sorenson H.263.",
	"H.263+ / H.263-1998 / H.263 version 2.",
	"Huffyuv / HuffYUV.",
	"libtheora Theora.",
	"libx264 H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10.",
	"MJPEG (Motion JPEG).",
	"raw MPEG-1 video.",
	"raw MPEG-2 video.",
	"MPEG-4 part 2. (XviD/DivX)",
	"MPEG-4 part 2 Microsoft variant version 3.",
	"MPEG-4 part 2 Microsoft variant version 1.",
	"MPEG-4 part 2 Microsoft variant version 2.",
	"Windows Media Video 7.",
	"Windows Media Video 8.",
	NULL
};

/* === M E T H O D S ================================================ */

int main(int ac, char* av[])
{
	setlocale(LC_ALL, "");
	
	binary_path = synfig::get_binary_path(String(av[0]));

#ifdef ENABLE_NLS
	String locale_dir;
	locale_dir = etl::dirname(etl::dirname(binary_path))+ETL_DIRECTORY_SEPARATOR+"share"+ETL_DIRECTORY_SEPARATOR+"locale";
#ifdef WIN32
	locale_dir = Glib::locale_from_utf8(locale_dir);
#endif
	bindtextdomain("synfig", locale_dir.c_str() );
	bind_textdomain_codeset("synfig", "UTF-8");
	textdomain("synfig");
#endif

	if(!SYNFIG_CHECK_VERSION())
	{
		cerr << _("FATAL: Synfig Version Mismatch") << endl;
		return SYNFIGTOOL_BADVERSION;
	}

	try {
		if(ac==1)
			throw (SynfigToolException(SYNFIGTOOL_MISSINGARGUMENT));


		named_type<string>* target_arg_desc = new named_type<string>("module");
		named_type<int>* width_arg_desc = new named_type<int>("NUM");
		named_type<int>* height_arg_desc = new named_type<int>("NUM");
		named_type<int>* span_arg_desc = new named_type<int>("NUM");
		named_type<int>* antialias_arg_desc = new named_type<int>("1..30");
		named_type<int>* quality_arg_desc = new named_type<int>("0..10");
		named_type<float>* gamma_arg_desc = new named_type<float>("NUM (=2.2)");
		named_type<int>* threads_arg_desc = new named_type<int>("NUM");
		named_type<int>* verbosity_arg_desc = new named_type<int>("NUM");
		named_type<string>* canvas_arg_desc = new named_type<string>("canvas-id");
		named_type<string>* output_file_arg_desc = new named_type<string>("filename");
		named_type<string>* input_file_arg_desc = new named_type<string>("filename");
		named_type<float>* fps_arg_desc = new named_type<float>("NUM");
		named_type<string>* time_arg_desc = new named_type<string>("seconds");
		named_type<string>* begin_time_arg_desc = new named_type<string>("seconds");
		named_type<string>* start_time_arg_desc = new named_type<string>("seconds");
		named_type<string>* end_time_arg_desc = new named_type<string>("seconds");
		named_type<int>* dpi_arg_desc = new named_type<int>("NUM");
		named_type<int>* dpi_x_arg_desc = new named_type<int>("NUM");
		named_type<int>* dpi_y_arg_desc = new named_type<int>("NUM");
		named_type<string>* append_filename_arg_desc = new named_type<string>("filename");
		named_type<string>* sequence_separator_arg_desc = new named_type<string>("string");
		named_type<string>* canvas_info_fields_arg_desc = new named_type<string>("fields");
		named_type<string>* layer_info_field_arg_desc = new named_type<string>("layer-name");
		named_type<string>* video_codec_arg_desc = new named_type<string>("codec");
		named_type<int>* video_bitrate_arg_desc = new named_type<int>("bitrate");

        po::options_description po_settings(_("Settings"));
        po_settings.add_options()
			("target,t", target_arg_desc, _("Specify output target (Default: PNG)"))
            ("width,w", width_arg_desc, _("Set the image width in pixels (Use zero for file default)"))
            ("height,h", height_arg_desc, _("Set the image height in pixels (Use zero for file default)"))
            ("span,s", span_arg_desc, _("Set the diagonal size of image window (Span)"))
            ("antialias,a", antialias_arg_desc, _("Set antialias amount for parametric renderer."))
            ("quality,Q", quality_arg_desc->default_value(DEFAULT_QUALITY), strprintf(_("Specify image quality for accelerated renderer (Default: %d)"), DEFAULT_QUALITY).c_str())
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
        po::store(po::command_line_parser(ac, av).options(po_all).
				  positional(po_positional).run(), vm);

        OptionsProcessor op(vm, po_visible);

        // Switch options ---------------------------------------------
        op.process_settings_options();

#ifdef _DEBUG
		// DEBUG options ----------------------------------------------
		op.process_debug_options();
#endif

		// TODO: Optional load of main only if needed. i.e. not needed to display help
		// Synfig Main initialization needs to be after verbose and
		// before any other where it's used
		Progress p(binary_path.c_str());
		synfig::Main synfig_main(etl::dirname(binary_path), &p);

        // Info options -----------------------------------------------
        op.process_info_options();

		list<Job> job_list;

		// Processing --------------------------------------------------
		Job job;
		job = op.extract_job();
		job.desc = job.canvas->rend_desc() = op.extract_renddesc(job.canvas->rend_desc());

		job_list.push_front(job);

		process_job_list(job_list, op.extract_targetparam());

		return SYNFIGTOOL_OK;

    }
    catch (SynfigToolException& e) {
    	exit_code code = e.get_exit_code();
    	if (code != SYNFIGTOOL_HELP && code != SYNFIGTOOL_OK)
    		cerr << e.get_message().c_str() << endl;
    	if (code == SYNFIGTOOL_MISSINGARGUMENT)
    		print_usage();

    	return code;
    }
    catch(std::exception& e) {
        cout << e.what() << "\n";
    }
}
