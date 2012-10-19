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
#include <ETL/clock>
#include <algorithm>
#include <cstring>
#include <errno.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/tokenizer.hpp>
#include <boost/token_functions.hpp>

#include <synfig/loadcanvas.h>
#include <synfig/savecanvas.h>
#include <synfig/target_scanline.h>
#include <synfig/module.h>
#include <synfig/importer.h>
#include <synfig/layer.h>
#include <synfig/canvas.h>
#include <synfig/target.h>
#include <synfig/targetparam.h>
#include <synfig/time.h>
#include <synfig/string.h>
#include <synfig/paramdesc.h>
#include <synfig/main.h>
#include <synfig/guid.h>
#include <autorevision.h>
#include "definitions.h"
#include "progress.h"
#include "renderprogress.h"
#include "job.h"
#include "synfigtoolexception.h"
#include "optionsprocessor.h"
#include "printing_functions.h"

#include "named_type.h"
#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace boost;
namespace po=boost::program_options;

/* === G L O B A L S ================================================ */

const char *progname;
int verbosity=0;
bool be_quiet=false;
bool print_benchmarks=false;

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

#ifdef _DEBUG

void guid_test()
{
	cout << "GUID Test" << endl;
	for(int i = 20; i; i--)
		cout << synfig::GUID().get_string() << ' '
			 << synfig::GUID().get_string() << endl;
}

void signal_test_func()
{
	cout << "**SIGNAL CALLED**" << endl;
}

void signal_test()
{
	sigc::signal<void> sig;
	sigc::connection conn;
	cout << "Signal Test" << endl;
	conn = sig.connect(sigc::ptr_fun(signal_test_func));
	cout << "Next line should exclaim signal called." << endl;
	sig();
	conn.disconnect();
	cout << "Next line should NOT exclaim signal called." << endl;
	sig();
	cout << "done."<<endl;
}

#endif

void extract_canvas_info(Job& job, string values)
{
	job.canvas_info = true;
	string value;

	std::string::size_type pos;
	while (!values.empty())
	{
		pos = values.find_first_of(',');
		if (pos == std::string::npos)
		{
			value = values;
			values = "";
		}
		else
		{
			value = values.substr(0, pos);
			values = values.substr(pos+1);
		}
		if (value == "all")
		{
			job.canvas_info_all = true;
			return;
		}

		if (value == "time_start")			job.canvas_info_time_start		= true;
		else if (value == "time_end")		job.canvas_info_time_end		= true;
		else if (value == "frame_rate")		job.canvas_info_frame_rate		= true;
		else if (value == "frame_start")	job.canvas_info_frame_start		= true;
		else if (value == "frame_end")		job.canvas_info_frame_end		= true;
		else if (value == "w")				job.canvas_info_w				= true;
		else if (value == "h")				job.canvas_info_h				= true;
		else if (value == "image_aspect")	job.canvas_info_image_aspect	= true;
		else if (value == "pw")				job.canvas_info_pw				= true;
		else if (value == "ph")				job.canvas_info_ph				= true;
		else if (value == "pixel_aspect")	job.canvas_info_pixel_aspect	= true;
		else if (value == "tl")				job.canvas_info_tl				= true;
		else if (value == "br")				job.canvas_info_br				= true;
		else if (value == "physical_w")		job.canvas_info_physical_w		= true;
		else if (value == "physical_h")		job.canvas_info_physical_h		= true;
		else if (value == "x_res")			job.canvas_info_x_res			= true;
		else if (value == "y_res")			job.canvas_info_y_res			= true;
		else if (value == "span")			job.canvas_info_span			= true;
		else if (value == "interlaced")		job.canvas_info_interlaced		= true;
		else if (value == "antialias")		job.canvas_info_antialias		= true;
		else if (value == "clamp")			job.canvas_info_clamp			= true;
		else if (value == "flags")			job.canvas_info_flags			= true;
		else if (value == "focus")			job.canvas_info_focus			= true;
		else if (value == "bg_color")		job.canvas_info_bg_color		= true;
		else if (value == "metadata")		job.canvas_info_metadata		= true;
		else
		{
			cerr << _("Unrecognised canvas variable: ") << "'" << value << "'" << endl;
			cerr << _("Recognized variables are:") << endl <<
				"  all, time_start, time_end, frame_rate, frame_start, frame_end, w, h," << endl <<
				"  image_aspect, pw, ph, pixel_aspect, tl, br, physical_w, physical_h," << endl <<
				"  x_res, y_res, span, interlaced, antialias, clamp, flags," << endl <<
				"  focus, bg_color, metadata" << endl;
		}

		if (pos == std::string::npos)
			break;
	};
}

void extract_options(const po::variables_map& vm, RendDesc& desc)
{
	int w, h;
	float span;
	span = w = h = 0;

	if (vm.count("width"))
		w = vm["width"].as<int>();

	if (vm.count("height"))
		h = vm["height"].as<int>();

	if (vm.count("antialias"))
	{
		int a;
		a = vm["antialias"].as<int>();
		desc.set_antialias(a);
		VERBOSE_OUT(1) << strprintf(_("Antialiasing set to %d, "
									  "(%d samples per pixel)"), a, a*a)
					   << endl;
	}
	if (vm.count("span"))
	{
		span = vm["antialias"].as<int>();
		VERBOSE_OUT(1) << strprintf(_("Span set to %d units"), span)
					   << endl;
	}
	if (vm.count("fps"))
	{
		float fps;
		fps = vm["antialias"].as<float>();
		desc.set_frame_rate(fps);
		VERBOSE_OUT(1) << strprintf(_("Frame rate set to %d frames per "
									  "second"), fps) << endl;
	}
	if (vm.count("dpi"))
	{
		float dpi, dots_per_meter;
		dpi = vm["dpi"].as<float>();
		dots_per_meter = dpi * 39.3700787402;
		desc.set_x_res(dots_per_meter);
		desc.set_y_res(dots_per_meter);
		VERBOSE_OUT(1) << strprintf(_("Physical resolution set to %f "
									  "dpi"), dpi) << endl;
	}
	if (vm.count("dpi-x"))
	{
		float dpi, dots_per_meter;
		dpi = vm["dpi-x"].as<float>();
		dots_per_meter = dpi * 39.3700787402;
		desc.set_x_res(dots_per_meter);
		VERBOSE_OUT(1) << strprintf(_("Physical X resolution set to %f "
									  "dpi"), dpi) << endl;
	}
	if (vm.count("dpi-y"))
	{
		float dpi, dots_per_meter;
		dpi = vm["dpi-y"].as<float>();
		dots_per_meter = dpi * 39.3700787402;
		desc.set_y_res(dots_per_meter);
		VERBOSE_OUT(1) << strprintf(_("Physical Y resolution set to %f "
									  "dpi"), dpi) << endl;
	}
	if (vm.count("start-time"))
	{
		int seconds;
		stringstream ss;
		seconds = vm["start-time"].as<int>();
		ss << seconds;
		desc.set_time_start(Time(ss.str().c_str(), desc.get_frame_rate()));
	}
	if (vm.count("begin-time"))
	{
		int seconds;
		stringstream ss;
		seconds = vm["begin-time"].as<int>();
		ss << seconds;
		desc.set_time_start(Time(ss.str().c_str(), desc.get_frame_rate()));
	}
	if (vm.count("end-time"))
	{
		int seconds;
		stringstream ss;
		seconds = vm["end-time"].as<int>();
		ss << seconds;
		desc.set_time_end(Time(ss.str().c_str(), desc.get_frame_rate()));
	}
	if (vm.count("time"))
	{
		int seconds;
		stringstream ss;
		seconds = vm["time"].as<int>();
		ss << seconds;
		desc.set_time(Time(ss.str().c_str(), desc.get_frame_rate()));

		VERBOSE_OUT(1) << _("Rendering frame at ")
					   << desc.get_time_start().get_string(desc.get_frame_rate())
					   << endl;
	}
	if (vm.count("gamma"))
	{
		synfig::warning(_("Gamma argument is currently ignored"));
		//int gamma;
		//gamma = vm["gamma"].as<int>();
		//desc.set_gamma(Gamma(gamma));
	}

	if (w||h)
	{
		// scale properly
		if (!w)
			w = desc.get_w() * h / desc.get_h();
		else if (!h)
			h = desc.get_h() * w / desc.get_w();

		desc.set_wh(w,h);
		VERBOSE_OUT(1) << strprintf(_("Resolution set to %dx%d"), w, h)
					   << endl;
	}

	if(span)
		desc.set_span(span);

}

/// TODO: Check dependency between codec and bitrate parameters
void extract_options(const po::variables_map& vm, TargetParam& params)
{
	if (vm.count("video-codec"))
	{
		params.video_codec = vm["video-codec"].as<string>();

		// video_codec string to lowercase
		transform (params.video_codec.begin(),
				   params.video_codec.end(),
				   params.video_codec.begin(),
				   ::tolower);

		bool found = false;
		// Check if the given video codec is allowed.
		for (int i = 0; !found && allowed_video_codecs[i] != NULL; i++)
			if (params.video_codec == allowed_video_codecs[i])
				found = true;

		// TODO: if (!found) Error!
		// else
		VERBOSE_OUT(1) << strprintf(_("Target video codec set to %s"), params.video_codec.c_str())
					   << endl;
	}
	if(vm.count("video-bitrate"))
	{
		params.bitrate = vm["video-bitrate"].as<int>();
		VERBOSE_OUT(1) << strprintf(_("Target bitrate set to %dk"),params.bitrate)
					   << endl;
	}
	if(vm.count("sequence-separator"))
	{
		params.sequence_separator = vm["sequence-separator"].as<string>();
		VERBOSE_OUT(1) << strprintf(_("Output file sequence separator set to %s"),
									params.sequence_separator.c_str())
					   << endl;
	}
}

int main(int ac, char* av[])
{
	setlocale(LC_ALL, "");

#ifdef ENABLE_NLS
	bindtextdomain("synfig", LOCALEDIR);
	bind_textdomain_codeset("synfig", "UTF-8");
	textdomain("synfig");
#endif

	progname=av[0];

	if(!SYNFIG_CHECK_VERSION())
	{
		cerr << _("FATAL: Synfig Version Mismatch") << endl;
		return SYNFIGTOOL_BADVERSION;
	}

	if(ac==1)
	{
		print_usage();
		return SYNFIGTOOL_BLANK;
	}

    try {
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
		named_type<int>* fps_arg_desc = new named_type<int>("NUM");
		named_type<int>* time_arg_desc = new named_type<int>("seconds");
		named_type<int>* begin_time_arg_desc = new named_type<int>("seconds");
		named_type<int>* start_time_arg_desc = new named_type<int>("seconds");
		named_type<int>* end_time_arg_desc = new named_type<int>("seconds");
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
			("target,t", target_arg_desc, _("Specify output target (Default:unknown)"))
            ("width,w", width_arg_desc, _("Set the image width (Use zero for file default)"))
            ("height,h", height_arg_desc, _("Set the image height (Use zero for file default)"))
            ("span,s", span_arg_desc, _("Set the diagonal size of image window (Span)"))
            ("antialias,a", antialias_arg_desc, _("Set antialias amount for parametric renderer."))
            ("quality,Q", quality_arg_desc->default_value(DEFAULT_QUALITY), strprintf(_("Specify image quality for accelerated renderer (default=%d)"), DEFAULT_QUALITY).c_str())
            ("sequence-separator", sequence_separator_arg_desc, _("Output file sequence separator string (use double quotes if you want to use spaces)"))
            ("gamma,g", gamma_arg_desc, _("Gamma"))
            ("threads,T", threads_arg_desc, _("Enable multithreaded renderer using specified # of threads"))
            ("canvas,c", canvas_arg_desc, _("Render the canvas with the given id instead of the root."))
            ("output-file,o", output_file_arg_desc, _("Specify output filename"))
            ("input-file", input_file_arg_desc, _("Specify input filename"))
            ("fps", fps_arg_desc, _("Set the frame rate"))
			("time", time_arg_desc, _("Render a single frame at <seconds>"))
			("begin-time", begin_time_arg_desc, _("Set the starting time"))
			("start-time", start_time_arg_desc, _("Set the starting time"))
			("end-time", end_time_arg_desc, _("Set the ending time"))
			("dpi", dpi_arg_desc, _("Set the physical resolution (dots-per-inch)"))
			("dpi-x", dpi_x_arg_desc, _("Set the physical X resolution (dots-per-inch)"))
			("dpi-y", dpi_y_arg_desc, _("Set the physical Y resolution (dots-per-inch)"))
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
            ("list-canvases", _("List the exported canvases in the composition"))
            ;

        po::options_description po_ffmpeg(_("FFMPEG target options"));
        po_ffmpeg.add_options()
			("video-codec", video_codec_arg_desc, _("Set the codec for the video. See --ffmpeg-video-codecs"))
            ("video-bitrate", video_bitrate_arg_desc, _("Set the bitrate for the output video"))
            ;

        po::options_description po_info("Synfig info options");
        po_info.add_options()
			("help", _("Produce this help message"))
            ("importers", _("Print out the list of available importers"))
			("info", _("Print out misc build information"))
            ("layers", _("Print out the list of available layers"))
            ("layer-info", layer_info_field_arg_desc, _("Print out layer's description, parameter info, etc."))
            ("license", _("Print out license information"))
            ("modules", _("Print out the list of loaded modules"))
            ("targets", _("Print out the list of available targets"))
            ("ffmpeg-video-codecs", _("Print out the list of available video codecs when encoding through FFMPEG"))
            ("valuenodes", _("Print out the list of available ValueNodes"))
            ("version", _("Print out version information"))
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
        po_all.add(po_settings).add(po_switchopts).add(po_misc).add(po_info).add(po_ffmpeg);

#ifdef _DEBUG
		po_all.add(po_debug);
#endif

        // Declare an options description instance which will be shown
        // to the user
        po::options_description po_visible("");
        po_visible.add(po_settings).add(po_switchopts).add(po_misc).add(po_ffmpeg).add(po_info);


        po::variables_map vm;
        po::store(po::command_line_parser(ac, av).options(po_all).
				  positional(po_positional).run(), vm);

        OptionsProcessor op(vm, po_visible);

        // Switch options ---------------------------------------------
        op.process_switch_options();

#ifdef _DEBUG
		// DEBUG options ----------------------------------------------
		if (vm.count("signal-test"))
		{
			signal_test();
			return SYNFIGTOOL_HELP;
		}

		if (vm.count("guid-test"))
		{
			guid_test();
			return SYNFIGTOOL_HELP;
		}
#endif


		// TODO: Optional load of main only if needed. i.e. not needed to display help
		// Synfig Main initialization needs to be after verbose and
		// before any other where it's used
		Progress p(progname);
		synfig::Main synfig_main(dirname(progname), &p);

        // Info options -----------------------------------------------
        op.process_info_options();

		list<Job> job_list;

		// Processing options --------------------------------------------------
		string target_name;
		int threads;

		// Common input file loading
		if (vm.count("input-file"))
		{
			Job job;

			job.filename = vm["input-file"].as<string>();

			// Open the composition
			string errors, warnings;
			try
			{
				job.root = open_canvas(job.filename, errors, warnings);
			}
			catch(runtime_error x)
			{
				job.root = 0;
			}

			// By default, the canvas to render is the root canvas
			// This can be changed through --canvas option
			job.canvas = job.root;

			if(!job.canvas)
			{
				cerr << _("Unable to load '") << job.filename << "'."
					 << endl;

				return SYNFIGTOOL_FILENOTFOUND;
			}

			job.root->set_time(0);
			job_list.push_front(job);
		}

		if (vm.count("target"))
		{
			target_name = vm["target"].as<string>();
			VERBOSE_OUT(1) << _("Target set to ") << target_name << endl;
		}

		// Determine output
		if (vm.count("output-file"))
		{
			job_list.front().outfilename = vm["output-file"].as<string>();
		}

		if (vm.count("quality"))
			job_list.front().quality = vm["quality"].as<int>();
		else
			job_list.front().quality = DEFAULT_QUALITY;

		VERBOSE_OUT(1) << _("Quality set to ") << job_list.front().quality
					   << endl;

		if (vm.count("threads"))
			threads = vm["threads"].as<int>();
		else
			threads = 1;

		VERBOSE_OUT(1) << _("Threads set to ") << threads << endl;

		// WARNING: canvas must be before append

		if (vm.count("canvas"))
		{
			string canvasid;
			canvasid = vm["canvas"].as<string>();

			try
			{
				string warnings;
				job_list.front().canvas =
					job_list.front().root->find_canvas(canvasid, warnings);
			}
			catch(Exception::IDNotFound)
			{
				cerr << _("Unable to find canvas with ID \"")
					 << canvasid << _("\" in ") << job_list.front().filename
					 << "." << endl;
				cerr << _("Throwing out job...") << endl;
				job_list.pop_front();
			}
			catch(Exception::BadLinkName)
			{
				cerr << _("Invalid canvas name \"") << canvasid
					 << _("\" in ") << job_list.front().filename << "."
					 << endl;
				cerr << _("Throwing out job...") << endl;
				job_list.pop_front();
			}

			// Later we need to set the other parameters for the jobs
		}

		// WARNING: append must be before list-canvases

		if (vm.count("append"))
		{
			// TODO: Enable multi appending. Disabled in the previous CLI version
			string composite_file;
			composite_file = vm["append"].as<string>();

			string errors, warnings;
			Canvas::Handle composite(open_canvas(composite_file, errors, warnings));
			if(!composite)
			{
				cerr << _("Unable to append '") << composite_file
					 << "'." << endl;
			}
			else
			{
				Canvas::reverse_iterator iter;
				for(iter=composite->rbegin(); iter!=composite->rend(); ++iter)
				{
					Layer::Handle layer(*iter);
					if(layer->active())
						job_list.front().canvas->push_front(layer->clone());
				}
			}

			VERBOSE_OUT(2) << _("Appended contents of ") << composite_file << endl;
		}

		if (vm.count("list-canvases"))
		{
			print_child_canvases(job_list.front().filename + "#",
								 job_list.front().root);

			cerr << endl;

			return SYNFIGTOOL_OK;
		}

		if (vm.count("canvas-info"))
		{
			extract_canvas_info(job_list.front(),
								vm["canvas-info"].as<string>());
			print_canvas_info(job_list.front());

			return SYNFIGTOOL_OK;
		}


		// Setup Job list ----------------------------------------------

		extract_options(vm,job_list.front().canvas->rend_desc());
		job_list.front().desc = job_list.front().canvas->rend_desc();

		VERBOSE_OUT(4) << _("Attempting to determine target/outfile...") << endl;

		// If the target type is not yet defined,
		// try to figure it out from the outfile.
		if(!vm.count("target") && !job_list.front().outfilename.empty())
		{
			VERBOSE_OUT(3) << _("Target name undefined, attempting to figure it out")
						   << endl;
			string ext = filename_extension(job_list.front().outfilename);
			if (ext.length())
				ext = ext.substr(1);

			if(Target::ext_book().count(ext))
			{
				target_name = Target::ext_book()[ext];
				info("target name not specified - using %s", target_name.c_str());
			}
			else
			{
				string lower_ext(ext);

				for(unsigned int i = 0; i < ext.length(); i++)
					lower_ext[i] = tolower(ext[i]);

				if(Target::ext_book().count(lower_ext))
				{
					target_name=Target::ext_book()[lower_ext];
					info("target name not specified - using %s", target_name.c_str());
				}
				else
					target_name=ext;
			}
		}

		// If the target type is STILL not yet defined, then
		// set it to a some sort of default
		if(target_name.empty())
		{
			VERBOSE_OUT(2) << _("Defaulting to PNG target...") << endl;
			target_name = "png";
		}

		TargetParam target_parameters;
		extract_options(vm, target_parameters);

		// If no output filename was provided, then create a output filename
		// based on the given input filename and the selected target.
		// (ie: change the extension)
		if(job_list.front().outfilename.empty())
		{
			job_list.front().outfilename =
				filename_sans_extension(job_list.front().filename) + '.';

			if(Target::book().count(target_name))
				job_list.front().outfilename +=
					Target::book()[target_name].filename;
			else
				job_list.front().outfilename += target_name;
		}

		VERBOSE_OUT(4) << "Target name = " << target_name << endl;
		VERBOSE_OUT(4) << "Outfilename = " << job_list.front().outfilename
					   << endl;

		// Check permissions
		if (access(dirname(job_list.front().outfilename).c_str(), W_OK) == -1)
		{
			cerr << _("Unable to create ouput for ")
				 << job_list.front().outfilename + ": " + strerror(errno)
				 << endl;
			job_list.pop_front();
			cerr << _("Throwing out job...") << endl;
		}

		VERBOSE_OUT(4) << _("Creating the target...") << endl;
		job_list.front().target =
			synfig::Target::create(target_name,
								   job_list.front().outfilename,
								   target_parameters);

		if(target_name == "sif")
			job_list.front().sifout=true;
		else
		{
			if(!job_list.front().target)
			{
				cerr << _("Unknown target for ") << job_list.front().filename
					 << ": " << target_name << endl;
				cerr << _("Throwing out job...") << endl;
				job_list.pop_front();
			}
			job_list.front().sifout=false;
		}

		// Set the Canvas on the Target
		if(job_list.front().target)
		{
			VERBOSE_OUT(4) << _("Setting the canvas on the target...") << endl;
			job_list.front().target->set_canvas(job_list.front().canvas);

			VERBOSE_OUT(4) << _("Setting the quality of the target...") << endl;
			job_list.front().target->set_quality(job_list.front().quality);
		}

		// Set the threads for the target
		if (job_list.front().target &&
			Target_Scanline::Handle::cast_dynamic(job_list.front().target))
			Target_Scanline::Handle::cast_dynamic(job_list.front().target)->set_threads(threads);


		// Process Job list --------------------------------------------

		if(!job_list.size())
		{
			cerr << _("Nothing to do!") << endl;
			return SYNFIGTOOL_BORED;
		}

		for(; job_list.size(); job_list.pop_front())
		{
			VERBOSE_OUT(3) << job_list.front().filename << " -- " << endl;
			VERBOSE_OUT(3) << '\t'
						   <<
				strprintf("w:%d, h:%d, a:%d, pxaspect:%f, imaspect:%f, span:%f",
					job_list.front().desc.get_w(),
					job_list.front().desc.get_h(),
					job_list.front().desc.get_antialias(),
					job_list.front().desc.get_pixel_aspect(),
					job_list.front().desc.get_image_aspect(),
					job_list.front().desc.get_span()
					)
				<< endl;

			VERBOSE_OUT(3) << '\t'
						   <<
				strprintf("tl:[%f,%f], br:[%f,%f], focus:[%f,%f]",
					job_list.front().desc.get_tl()[0],
					job_list.front().desc.get_tl()[1],
					job_list.front().desc.get_br()[0],
					job_list.front().desc.get_br()[1],
					job_list.front().desc.get_focus()[0],
					job_list.front().desc.get_focus()[1]
					)
					<< endl;

			RenderProgress p;
			p.task(job_list.front().filename + " ==> " +
				   job_list.front().outfilename);
			if(job_list.front().sifout)
			{
				if(!save_canvas(job_list.front().outfilename,
								job_list.front().canvas))
				{
					cerr << _("Render Failure.") << endl;
					return SYNFIGTOOL_RENDERFAILURE;
				}
			}
			else
			{
				VERBOSE_OUT(1) << _("Rendering...") << endl;
				etl::clock timer;
				timer.reset();

				// Call the render member of the target
				if(!job_list.front().target->render(&p))
				{
					cerr << _("Render Failure.") << endl;
					return SYNFIGTOOL_RENDERFAILURE;
				}

				if(print_benchmarks)
					cout << job_list.front().filename
						 << _(": Rendered in ") << timer()
						 << _(" seconds.") << endl;
			}
		}

		job_list.clear();

		VERBOSE_OUT(1) << _("Done.") << endl;

		return SYNFIGTOOL_OK;

    }
    catch (SynfigToolException& e) {
    	exit_code code = e.get_exit_code();
    	if (code != SYNFIGTOOL_HELP && code != SYNFIGTOOL_OK)
    		cerr << e.get_message() << endl;

    	return code;
    }
    catch(std::exception& e) {
        cout << e.what() << "\n";
    }
}
