/* === S Y N F I G ========================================================= */
/*!	\file tool/optionsprocessor.cpp
**	\brief Synfig Tool Options Processor Class
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2009-2014 Diego Barrios Romero
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

#include <autorevision.h>
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/canvas.h>
#include <synfig/canvasfilenaming.h>
#include <synfig/target.h>
#include <synfig/layer.h>
#include <synfig/module.h>
#include <synfig/importer.h>
#include <synfig/loadcanvas.h>
#include <synfig/valuenode_registry.h>

#include "definitions.h"
#include "job.h"
#include "synfigtoolexception.h"
#include "printing_functions.h"
#include "optionsprocessor.h"
#include <glibmm/init.h>
#endif

using namespace synfig;

template<typename T>
void SynfigCommandLineParser::add_option(Glib::OptionGroup& og, const std::string& name, const gchar& short_name,
	T& entry, const std::string& description, const Glib::ustring& arg_description) {

	    Glib::OptionEntry new_entry;
		new_entry.set_long_name(name);
		if (short_name != ' ') new_entry.set_short_name(short_name);
		new_entry.set_description(description);
		new_entry.set_arg_description(arg_description);
		og.add_entry(new_entry, entry);
}

// we need explicit method in case of different string/filename encodings
void SynfigCommandLineParser::add_option_filename(Glib::OptionGroup& og, const std::string& name, const gchar& short_name, std::string& entry, const std::string& description, const Glib::ustring& arg_description) {
	    Glib::OptionEntry new_entry;
		new_entry.set_long_name(name);
		if (short_name != ' ') new_entry.set_short_name(short_name);
		new_entry.set_description(description);
		new_entry.set_arg_description(arg_description);
		og.add_entry_filename(new_entry, entry);
}


SynfigCommandLineParser::SynfigCommandLineParser() :
	og_set("settings", _("Settings"), _("Show settings help")),
	og_switch("switch", _("Switch options"), _("Show switch help")),
	og_misc("misc", _("Misc options"), _("Show Misc options help")),
	og_ffmpeg("ffmpeg", _("FFMPEG target options"), _("Show FFMPEG target options help")),
	og_info("info", _("Synfig info options"), _("Show Synfig info options help")),
#ifdef _DEBUG
	og_debug("debug", _("Synfig debug flags"), _("Show Synfig debug flags help")),
#endif
	set_target(),
	set_width(),
	set_height(),
	set_span(),
	set_antialias(),
	set_quality(),
	set_num_threads(),
	set_input_file(),
	set_output_file(),
	set_sequence_separator(),
	set_canvas_id(),
	set_fps(),
	set_time(),
	set_begin_time(),
	set_start_time(),
	set_end_time(),
	set_dpi(),
	set_dpi_x(),
	set_dpi_y(),

	// Switch group
	sw_verbosity(),
	sw_quiet(),
	sw_print_benchmarks(),
	sw_extract_alpha(),

	// Misc group
	misc_append_filename(),
	misc_canvas_info(),
	misc_canvases(),

	//FFMPEG group
	video_codec(),
	video_bitrate(),

	// Synfig info group
	show_help(),
	show_importers(),
	show_build_info(),
	show_layers_list(),
	show_layer_info(),
	show_license(),
	show_modules(),
	show_targets(),
	show_codecs(),
	show_value_nodes(),
	show_version()

#ifdef _DEBUG
	,
	// Debug group
	debug_guid(),
	debug_signal()
#endif
{
	Glib::init();

	//og_set("settings", "Settings", "Show settings help");
	add_option(og_set, "target",      't', set_target,		_("Specify output target (Default: PNG)"), "module");
	add_option(og_set, "width",       'w', set_width,		_("Set the image width in pixels (Use zero for file default)"), "NUM");
	add_option(og_set, "height",      'h', set_height,		_("Set the image height in pixels (Use zero for file default)"), "NUM");
	add_option(og_set, "span",        's', set_span,		_("Set the diagonal size of image window (Span)"), "NUM");
	add_option(og_set, "antialias",   'a', set_antialias,	_("Set antialias amount for parametric renderer."), "1..30");
	//og_set.add_option("quality",     'Q', quality_arg_desc, strprintf(_("Specify image quality for accelerated renderer (Default: %d)"), DEFAULT_QUALITY).c_str(), "NUM");
	add_option(og_set, "threads",     'T', set_num_threads, _("Enable multithreaded renderer using the specified number of threads"), "NUM");
	add_option(og_set, "input-file",  'i', set_input_file, 	_("Specify input filename"), "filename");
	add_option(og_set, "output-file", 'o', set_output_file, _("Specify output filename"), "filename");
	add_option(og_set, "sequence-separator", ' ', set_sequence_separator, _("Output file sequence separator string (Use double quotes if you want to use spaces)"), "string");
	add_option(og_set, "canvas",      'c', set_canvas_id, 	_("Render the canvas with the given id instead of the root."), "id");
	add_option(og_set, "fps",         ' ', set_fps, 		_("Set the frame rate"), "NUM");
	add_option(og_set, "time",        ' ', set_time, 		_("Render a single frame at <seconds>"), "seconds");
	add_option(og_set, "begin-time",  ' ', set_begin_time, 	_("Set the starting time"), "seconds");
	add_option(og_set, "start-time",  ' ', set_start_time,	_("Set the starting time"), "seconds");
	add_option(og_set, "end-time",    ' ', set_end_time, 	_("Set the ending time"), "seconds");
	add_option(og_set, "dpi",         ' ', set_dpi, 		_("Set the physical resolution (Dots-per-inch)"), "NUM");
	add_option(og_set, "dpi-x",       ' ', set_dpi_x, 		_("Set the physical X resolution (Dots-per-inch)"), "NUM");
	add_option(og_set, "dpi-y",       ' ', set_dpi_y, 		_("Set the physical Y resolution (Dots-per-inch)"), "NUM");

	// Switch options
	//og_switch("switch", _("Switch options"), "Show switch help");
	add_option(og_switch, "verbose",       'v', sw_verbosity, 			_("Output verbosity level"), "NUM");
	add_option(og_switch, "quiet",         'q', sw_quiet, 				_("Quiet mode (No progress/time-remaining display)"), "");
	add_option(og_switch, "benchmarks",    'b', sw_print_benchmarks,	_("Print benchmarks"), "");
	add_option(og_switch, "extract-alpha", 'x', sw_extract_alpha, 		_("Extract alpha"), "");

	//SynfigOptionGroup og_misc("misc", _("Misc options"), "Show Misc options help");
	add_option_filename(og_misc, "append", ' ', misc_append_filename, 	_("Append layers in <filename> to composition"), _("filename"));
	add_option(og_misc, "canvas-info",     ' ', misc_canvas_info, 			_("Print out specified details of the root canvas"), _("fields"));
	add_option(og_misc, "canvases",		   ' ', misc_canvases,				_("Print out the list of exported canvases in the composition"), "");

	//SynfigOptionGroup og_ffmpeg("ffmpeg", _("FFMPEG target options"), "Show FFMPEG target options help");
	add_option(og_ffmpeg, "video-codec",   ' ', video_codec, 	_("Set the codec for the video. See --target-video-codecs"), _("codec"));
	add_option(og_ffmpeg, "video-bitrate", ' ', video_bitrate,	_("Set the bitrate for the output video"), _("bitrate"));

	//SynfigOptionGroup og_info("info", _("Synfig info options"), "Show Synfig info options help");
	add_option(og_info, "help",       ' ', show_help, 			_("Produce this help message"), "");
	add_option(og_info, "importers",  ' ', show_importers, 		_("Print out the list of available importers"), "");
	add_option(og_info, "info",       ' ', show_build_info, 	_("Print out misc build information"), "");
	add_option(og_info, "layers",     ' ', show_layers_list, 	_("Print out the list of available layers"), "");
	add_option(og_info, "layer-info", ' ', show_layer_info, 	_("Print out layer's description, parameter info, etc."), _("layer-name"));
	add_option(og_info, "license",    ' ', show_license, 		_("Print out license information"), "");
	add_option(og_info, "modules",    ' ', show_modules, 		_("Print out the list of loaded modules"), "");
	add_option(og_info, "targets",    ' ', show_targets, 		_("Print out the list of available targets"), "");
	add_option(og_info, "target-video-codecs",' ', show_codecs, _("Print out the list of available video codecs when encoding through FFMPEG"), "");
	add_option(og_info, "valuenodes", ' ', show_value_nodes, 	_("Print out the list of available ValueNodes"), "");
	add_option(og_info, "version",    ' ', show_version, 		_("Print out version information"), "");

#ifdef _DEBUG
	//SynfigOptionGroup og_debug("debug", _("Synfig debug flags"), "Show Synfig debug flags help");
	add_option(og_debug, "guid-test",    ' ', debug_guid, 		_("Test GUID generation"), "");
	add_option(og_debug, "signal-test",  ' ', debug_signal, 	_("Test signal implementation"), "");
#endif

	// remaining options
	Glib::OptionEntry entry_remaining;
	entry_remaining.set_long_name(G_OPTION_REMAINING);
	entry_remaining.set_arg_description(G_OPTION_REMAINING);
	og_info.add_entry(entry_remaining, remaining_options_list);

	context.add_group(og_set);
	context.add_group(og_switch);
	context.add_group(og_misc);
	context.add_group(og_ffmpeg);
	//context.add_group(og_info);
	context.set_main_group(og_info); // remaining args works only in main group (OMG!)
#ifdef _DEBUG	
	context.add_group(og_debug);
#endif

	_allowed_video_codecs.push_back(VideoCodec("flv", "Flash Video (FLV) / Sorenson Spark / Sorenson H.263."));
	_allowed_video_codecs.push_back(VideoCodec("h263p", "H.263+ / H.263-1998 / H.263 version 2."));
	_allowed_video_codecs.push_back(VideoCodec("huffyuv", "Huffyuv / HuffYUV."));
	_allowed_video_codecs.push_back(VideoCodec("libtheora", "libtheora Theora."));
	_allowed_video_codecs.push_back(VideoCodec("libx264", "H.264 / AVC / MPEG-4 AVC."));
	_allowed_video_codecs.push_back(VideoCodec("libx264-lossless", "H.264 / AVC / MPEG-4 AVC (LossLess)."));
	_allowed_video_codecs.push_back(VideoCodec("mjpeg", "MJPEG (Motion JPEG)."));
	_allowed_video_codecs.push_back(VideoCodec("mpeg1video", "Raw MPEG-1 video."));
	_allowed_video_codecs.push_back(VideoCodec("mpeg2video", "Raw MPEG-2 video."));
	_allowed_video_codecs.push_back(VideoCodec("mpeg4", "MPEG-4 part 2 (XviD/DivX)."));
	_allowed_video_codecs.push_back(VideoCodec("msmpeg4", "MPEG-4 part 2 Microsoft variant version 3."));
	_allowed_video_codecs.push_back(VideoCodec("msmpeg4v1", "MPEG-4 part 2 Microsoft variant version 1."));
	_allowed_video_codecs.push_back(VideoCodec("msmpeg4v2", "MPEG-4 part 2 Microsoft variant version 2."));
	_allowed_video_codecs.push_back(VideoCodec("wmv1", "Windows Media Video 7."));
	_allowed_video_codecs.push_back(VideoCodec("wmv2", "Windows Media Video 8."));

}

bool SynfigCommandLineParser::parse(int argc, char* argv[])
{

#ifdef GLIBMM_EXCEPTIONS_ENABLED
	try
	{
#ifdef _WIN32
		context.parse(argv);
#else
		context.parse(argc, argv);
#endif
	}
	catch(const Glib::Error& ex)
	{
		std::cout << "Exception: " << ex.what() << std::endl;
		return false;
	}
#else
	std::auto_ptr<Glib::Error> ex;
	context.parse(argc, argv, ex);
	if(ex.get())
	{
		std::cout << "Exception: " << ex->what() << std::endl;
		return false;
	}
#endif //GLIBMM_EXCEPTIONS_ENABLED
	
	// set input filename from remaining options if it not already filled
	if (!remaining_options_list.empty() && set_input_file.empty()) set_input_file = remaining_options_list.front();

	return true;
}

void SynfigCommandLineParser::extract_canvas_info(Job& job)
{
	job.canvas_info = true;
	std::string value;
	std::string values = misc_canvas_info;//_vm["canvas-info"].as<string>();

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
			std::cerr << _("Unrecognised canvas variable: ") << "'" << value.c_str() << "'" << std::endl;
			std::cerr << _("Recognized variables are:\n") <<
				"  all, time_start, time_end, frame_rate, frame_start, frame_end, w, h,\n"
				"  image_aspect, pw, ph, pixel_aspect, tl, br, physical_w, physical_h,\n"
				"  x_res, y_res, span, interlaced, antialias, clamp, flags,\n"
				"  focus, bg_color, metadata" << std::endl;
		}

		if (pos == std::string::npos)
			break;
	}
}

void SynfigCommandLineParser::process_settings_options() const
{
	if (sw_verbosity > 0)
	{
		SynfigToolGeneralOptions::instance()->set_verbosity(sw_verbosity);
		VERBOSE_OUT(1) << _("verbosity set to ")
					   << SynfigToolGeneralOptions::instance()->get_verbosity()
					   << std::endl;
	}

	if (sw_print_benchmarks)
	{
		SynfigToolGeneralOptions::instance()->set_should_print_benchmarks(true);
	}

	if (sw_quiet)
	{
		SynfigToolGeneralOptions::instance()->set_should_be_quiet(true);
	}

	if (set_num_threads > 0)
	{
		SynfigToolGeneralOptions::instance()->set_threads(size_t(set_num_threads));
	}

	VERBOSE_OUT(1) << _("Threads set to ")
				   << SynfigToolGeneralOptions::instance()->get_threads() << std::endl;
}

void SynfigCommandLineParser::process_trivial_info_options()
{
	if (show_help)
	{
		print_usage();
		//cout << _po_visible;

		throw (SynfigToolException(SYNFIGTOOL_HELP));
	}

	if (show_build_info)
	{
		std::cout << PACKAGE "-" VERSION << std::endl;
#ifdef DEVEL_VERSION
		std::cout << std::endl << DEVEL_VERSION << std::endl << std::endl;
#endif
		std::cout << "Compiled on " << get_build_date();
#ifdef __GNUC__
		std::cout << " with GCC " << __VERSION__;
#elif defined(__clang__)
		std::cout << " with Clang " << __VERSION__;
#elif defined(_MSC_VER)
		std::cout << " with Microsoft Visual C++ "
			 << (_MSC_VER>>8) << '.' << (_MSC_VER&255);
#elif defined(__TCPLUSPLUS__)
		std::cout << " with Borland Turbo C++ "
			 << (__TCPLUSPLUS__>>8) << '.'
			 << ((__TCPLUSPLUS__&255)>>4) << '.'
			 << (__TCPLUSPLUS__&15);
#endif
		std::cout << std::endl << SYNFIG_COPYRIGHT << std::endl << std::endl;

		throw (SynfigToolException(SYNFIGTOOL_HELP));
	}

	if (show_version)
	{
		std::cerr << PACKAGE << " " << VERSION << std::endl;

		throw (SynfigToolException(SYNFIGTOOL_HELP));
	}

	if (show_license)
	{
		std::cerr << PACKAGE << " " << VERSION << std::endl;
		std::cout << SYNFIG_COPYRIGHT << std::endl << std::endl;
		std::cerr << SYNFIG_LICENSE << std::endl << std::endl;

		throw (SynfigToolException(SYNFIGTOOL_HELP));
	}

	if (show_codecs)
	{
		print_target_video_codecs_help();

		throw (SynfigToolException(SYNFIGTOOL_HELP));
	}

}

void SynfigCommandLineParser::process_info_options()
{
	if (show_layers_list) {
		for(const auto& iter : synfig::Layer::book()) {
			if (iter.second.category != CATEGORY_DO_NOT_USE)
				std::cout << (iter.first).c_str() << std::endl;
		}

		throw (SynfigToolException(SYNFIGTOOL_HELP));
	}

	if (!show_layer_info.empty()) {
		Layer::Handle layer = synfig::Layer::create(show_layer_info.c_str());

		std::cout << _("Layer Name: ") << layer->get_name() << std::endl;
		std::cout << _("Localized Layer Name: ")
			 << layer->get_local_name() << std::endl;
		std::cout << _("Version: ") << layer->get_version() << std::endl;

		Layer::Vocab vocab = layer->get_param_vocab();
		for(; !vocab.empty(); vocab.pop_front())
		{
			std::cout << _("param - ") << vocab.front().get_name().c_str();
			if(!vocab.front().get_critical())
				std::cout << _(" (not critical)");
			std::cout << std::endl << _("\tLocalized Name: ")
				 << vocab.front().get_local_name().c_str() << std::endl;

			if(!vocab.front().get_description().empty())
				std::cout << _("\tDescription: ")
					 << vocab.front().get_description().c_str() << std::endl;

			if(!vocab.front().get_hint().empty())
				std::cout << _("\tHint: ")
					 << vocab.front().get_hint().c_str() << std::endl;
		}

		throw (SynfigToolException(SYNFIGTOOL_HELP));
	}

	if (show_modules) {
		for (const auto& iter : synfig::Module::book()) {
			std::cout << (iter.first).c_str() << std::endl;
		}

		throw (SynfigToolException(SYNFIGTOOL_HELP));
	}

	if (show_targets) {
		for(const auto& iter : synfig::Target::book()) {
			std::cout << (iter.first).c_str() << std::endl;
		}

		throw (SynfigToolException(SYNFIGTOOL_HELP));
	}

	if (show_value_nodes) {
		for(const auto& iter : synfig::ValueNodeRegistry::book()) {
			std::cout << (iter.first).c_str() << std::endl;
		}

		throw (SynfigToolException(SYNFIGTOOL_HELP));
	}

	if (show_importers)	{
		for(const auto& iter : synfig::Importer::book()) {
			std::cout << (iter.first).c_str() << std::endl;
		}

		throw (SynfigToolException(SYNFIGTOOL_HELP));
	}
}

RendDesc SynfigCommandLineParser::extract_renddesc(const RendDesc& renddesc)
{
	RendDesc desc = renddesc;
	Real span = 0;
	int w = set_width;
	int h = set_height;

	if (set_antialias > 0)
	{
		int a = set_antialias;
		desc.set_antialias(a);
		synfig::info(_("Antialiasing set to %d, "
					"(%d samples per pixel)"), a, (a*a));
	}
	if (set_span > 0) {
	    span = set_span;
		synfig::info(_("Span set to %d units"), span);
	}

	if (set_fps > 0) {
		float fps = float(set_fps);
		desc.set_frame_rate(fps);
		synfig::info(_("Frame rate set to %.3f frames per second"), fps);
	}

	if (set_dpi > 0) {
		Real dots_per_meter;
		dots_per_meter = DPI2DPM(set_dpi);
		desc.set_x_res(dots_per_meter);
		desc.set_y_res(dots_per_meter);
		synfig::info(_("Physical resolution set to %f dpi"), set_dpi);
	}

	if (set_dpi_x > 0) {
		Real dots_per_meter;
		dots_per_meter = DPI2DPM(set_dpi_x);
		desc.set_x_res(dots_per_meter);
		synfig::info(_("Physical X resolution set to %f dpi"), set_dpi_x);
	}

	if (set_dpi_y > 0) {
		Real dots_per_meter;
		dots_per_meter = DPI2DPM(set_dpi_y);
		desc.set_y_res(dots_per_meter);
		synfig::info(_("Physical Y resolution set to %f dpi"), set_dpi_y);
	}

	if (!set_start_time.empty())
	{
		desc.set_time_start(Time(set_start_time.c_str(), desc.get_frame_rate()));
	}
	if (!set_begin_time.empty())
	{
		desc.set_time_start(Time(set_begin_time.c_str(), desc.get_frame_rate()));
	}
	if (!set_end_time.empty())
	{
		desc.set_time_end(Time(set_end_time.c_str(), desc.get_frame_rate()));
	}
	if (!set_time.empty())
	{
		desc.set_time(Time(set_time.c_str(), desc.get_frame_rate()));

		VERBOSE_OUT(1) << _("Rendering frame at ")
					   << desc.get_time_start().get_string(desc.get_frame_rate())
					   << std::endl;
	}

	if (w || h)
	{
		// scale properly
		if (!w)
			w = desc.get_w() * h / desc.get_h();
		else if (!h)
			h = desc.get_h() * w / desc.get_w();

		desc.set_wh(w, h);
		VERBOSE_OUT(1) << strprintf(_("Resolution set to %dx%d."), w, h) << std::endl;
	}

	if(span > 0)
		desc.set_span(span);

	return desc;
}

TargetParam SynfigCommandLineParser::extract_targetparam()
{
	TargetParam params;

	// Both parameters are co-dependent
	if ( ( video_codec.empty() && video_bitrate != 0)
	  || (!video_codec.empty() && video_bitrate == 0) )
		throw (SynfigToolException(SYNFIGTOOL_MISSINGARGUMENT,
									_("Both video codec and bitrate parameters are necessary.")));

	if (!video_codec.empty())
	{
		params.video_codec = video_codec;

		// video_codec string to lowercase
		transform (params.video_codec.begin(),
				   params.video_codec.end(),
				   params.video_codec.begin(),
				   ::tolower);

		bool found = false;
		// Check if the given video codec is allowed.
		for (const VideoCodec& vcodec : _allowed_video_codecs)
		{
			if (params.video_codec == vcodec.name)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
		    throw SynfigToolException(SYNFIGTOOL_UNKNOWNARGUMENT,
                                      strprintf(_("Video codec \"%s\" is not supported."), params.video_codec.c_str()));
		}

		VERBOSE_OUT(1) << _("Target video codec set to: ") << params.video_codec << std::endl;
	}
	if (video_bitrate > 0)
	{
		params.bitrate = video_bitrate;
		VERBOSE_OUT(1) << _("Target bitrate set to: ") << params.bitrate << "k."
					   << std::endl;
	}
	if (!set_sequence_separator.empty())
	{
		params.sequence_separator = set_sequence_separator;
		VERBOSE_OUT(1) << _("Output file sequence separator set to: '")
                       << params.sequence_separator
                       << "'."
					   << std::endl;
	}

	return params;
}

Job SynfigCommandLineParser::extract_job()
{
	Job job;

	// Common input file loading
	if (!set_input_file.empty())
	{
		job.filename = set_input_file;

		// Open the composition
		std::string errors, warnings;
		try
		{
			if (FileSystem::Handle file_system = CanvasFileNaming::make_filesystem(job.filename))
			{
				FileSystem::Identifier identifier = file_system->get_identifier(CanvasFileNaming::project_file(job.filename));
				job.root = open_canvas_as(identifier, job.filename, errors, warnings);
			}
			else
			{
				errors.append("Cannot open container " + job.filename + "\n");
			}
		}
		catch(std::runtime_error& /*x*/)
		{
			job.root = nullptr;
		}

		// By default, the canvas to render is the root canvas
		// This can be changed through --canvas option
		job.canvas = job.root;

		if(!job.canvas)
		{
		    throw SynfigToolException(SYNFIGTOOL_FILENOTFOUND,
                                      strprintf(_("Unable to load file '%s'."), job.filename.c_str()));
		}

		job.root->set_time(0);
	}
	else
	{
	    throw SynfigToolException(SYNFIGTOOL_MISSINGARGUMENT,
                                  _("No input file provided."));
	}

	if (!set_target.empty())
	{
		job.target_name = set_target;
		VERBOSE_OUT(1) << _("Target set to ") << job.target_name << std::endl;
	}

	// Determine output
	if (!set_output_file.empty())
	{
		job.outfilename = set_output_file;
	}

	if (sw_extract_alpha)
	{
		job.extract_alpha = true;
	}

	if (set_quality > 0)
		job.quality = set_quality;
	else
		job.quality = DEFAULT_QUALITY;

	VERBOSE_OUT(1) << _("Quality set to ") << job.quality << std::endl;

	// WARNING: canvas must be before append

	if (!set_canvas_id.empty())
	{
		std::string canvasid = set_canvas_id;

		try
		{
			std::string warnings;
			job.canvas = job.root->find_canvas(canvasid, warnings);
			// TODO: This exceptions should not terminate the program if multi-job
			// processing is available.
		}
		catch(Exception::IDNotFound&)
		{
			throw SynfigToolException(SYNFIGTOOL_INVALIDJOB,
					strprintf(_("Unable to find canvas with ID \"%s\" in %s.\n"
                                    "Throwing out job..."), 
									canvasid.c_str(), job.filename.c_str()));
		}
		catch(Exception::BadLinkName&)
		{
		    throw SynfigToolException(SYNFIGTOOL_INVALIDJOB,
                    strprintf(_("Invalid canvas name \"%s\" in %s.\n"
                                    "Throwing out job..."),
                                   	canvasid.c_str(), job.filename.c_str())); // FIXME: is here must be canvasid nor canvasname?
		}

		// Later we need to set the other parameters for the jobs
	}

	// WARNING: append must be before list-canvases

	if (!misc_append_filename.empty())
	{
		// TODO: Enable multi-appending. Disabled in the previous CLI version
		std::string composite_file = misc_append_filename;

		std::string errors, warnings;
		Canvas::Handle composite;
		if (FileSystem::Handle file_system = CanvasFileNaming::make_filesystem(composite_file))
		{
			FileSystem::Identifier identifier = file_system->get_identifier(CanvasFileNaming::project_file(composite_file));
			composite = open_canvas_as(identifier, composite_file, errors, warnings);
		}
		else
		{
			errors.append("Cannot open container " + composite_file + "\n");
		}

		if(!composite)
		{
			VERBOSE_OUT(1) << _("Unable to append '") << composite_file.c_str()
							<< "'." << std::endl;
		}
		else
		{
			Canvas::reverse_iterator iter;
			for(iter=composite->rbegin(); iter!=composite->rend(); ++iter)
			{
				Layer::Handle layer(*iter);
				//if(layer->active())
					job.canvas->push_front(layer->clone(composite));
			}
		}

		VERBOSE_OUT(2) << _("Appended contents of ") << composite_file << std::endl;
	}

	//if (_vm.count("list-canvases") || misc_canvases)
	if (misc_canvases)
	{
		print_child_canvases(job.filename + "#", job.root);
		std::cerr << std::endl;

		throw SynfigToolException(SYNFIGTOOL_OK);
	}

	if (!misc_canvas_info.empty())
	{
		extract_canvas_info(job);
		print_canvas_info(job);

		throw SynfigToolException(SYNFIGTOOL_OK);
	}

	return job;
}

void SynfigCommandLineParser::print_target_video_codecs_help() const
{
	for (std::vector<VideoCodec>::const_iterator itr = _allowed_video_codecs.begin();
		 itr != _allowed_video_codecs.end(); ++itr)
	{
		std::cout << " " << itr->name << ":   \t" << itr->description
				  << std::endl;
	}
}

#ifdef _DEBUG

// DEBUG auxiliary functions
void guid_test()
{
	std::cout << "GUID Test" << std::endl;
	for(int i = 20; i; i--)
		std::cout << synfig::GUID().get_string() << ' '
				  << synfig::GUID().get_string() << std::endl;
}

void signal_test_func()
{
	std::cout << "**SIGNAL CALLED**" << std::endl;
}

void signal_test()
{
	sigc::signal<void> sig;
	sigc::connection conn;
	std::cout << "Signal Test" << std::endl;
	conn = sig.connect(sigc::ptr_fun(signal_test_func));
	std::cout << "Next line should exclaim signal called." << std::endl;
	sig();
	conn.disconnect();
	std::cout << "Next line should NOT exclaim signal called." << std::endl;
	sig();
	std::cout << "done." << std::endl;
}

// DEBUG options ----------------------------------------------
void SynfigCommandLineParser::process_debug_options()
{
	if (debug_signal)
	{
		signal_test();
		throw (SynfigToolException(SYNFIGTOOL_HELP));
	}

	if (debug_guid)
	{
		guid_test();
		throw (SynfigToolException(SYNFIGTOOL_HELP));
	}
}

#endif
