/* === S Y N F I G ========================================================= */
/*!	\file tool/optionsprocessor.cpp
**	\brief Synfig Tool Options Processor Class
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2009-2014 Diego Barrios Romero
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
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <autorevision.h>
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/canvas.h>
#include <synfig/context.h>
#include <synfig/target.h>
#include <synfig/layer.h>
#include <synfig/module.h>
#include <synfig/main.h>
#include <synfig/importer.h>
#include <synfig/loadcanvas.h>
#include <synfig/guid.h>
#include <synfig/filesystemgroup.h>
#include <synfig/filesystemnative.h>
#include <synfig/filecontainerzip.h>

#include "definitions.h"
#include "job.h"
#include "synfigtoolexception.h"
#include "printing_functions.h"
#include "optionsprocessor.h"

#endif

using namespace std;
using namespace synfig;
namespace bfs=boost::filesystem;

OptionsProcessor::OptionsProcessor(
	boost::program_options::variables_map& vm,
	const boost::program_options::options_description& po_visible)
		: _vm(vm), _po_visible(po_visible)
{
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

void OptionsProcessor::extract_canvas_info(Job& job)
{
	job.canvas_info = true;
	string value;
	string values = _vm["canvas-info"].as<string>();

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
			cerr << _("Unrecognised canvas variable: ") << "'" << value.c_str() << "'" << endl;
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

void OptionsProcessor::process_settings_options()
{
	if (_vm.count("verbose"))
	{
		SynfigToolGeneralOptions::instance()->set_verbosity(_vm["verbose"].as<int>());
		VERBOSE_OUT(1) << _("verbosity set to ")
					   << SynfigToolGeneralOptions::instance()->get_verbosity()
					   << std::endl;
	}

	if (_vm.count("benchmarks"))
	{
		SynfigToolGeneralOptions::instance()->set_should_print_benchmarks(true);
	}

	if (_vm.count("quiet"))
	{
		SynfigToolGeneralOptions::instance()->set_should_be_quiet(true);
	}

	if (_vm.count("threads"))
	{
		SynfigToolGeneralOptions::instance()->set_threads(_vm["threads"].as<int>());
	}

	VERBOSE_OUT(1) << _("Threads set to ")
				   << SynfigToolGeneralOptions::instance()->get_threads() << std::endl;
}

void OptionsProcessor::process_info_options()
{
	if (_vm.count("help"))
	{
		print_usage();
		cout << _po_visible;

		throw (SynfigToolException(SYNFIGTOOL_HELP));
	}

	if (_vm.count("info"))
	{
		cout << PACKAGE "-" VERSION << endl;
#ifdef DEVEL_VERSION
			cout << endl << DEVEL_VERSION << endl << endl;
#endif
		cout << "Compiled on " __DATE__ /* " at "__TIME__ */;
#ifdef __GNUC__
		cout << " with GCC " << __VERSION__;
#endif
#ifdef _MSC_VER
		cout << " with Microsoft Visual C++ "
			 << (_MSC_VER>>8) << '.' << (_MSC_VER&255);
#endif
#ifdef __TCPLUSPLUS__
		cout << " with Borland Turbo C++ "
			 << (__TCPLUSPLUS__>>8) << '.'
			 << ((__TCPLUSPLUS__&255)>>4) << '.'
			 << (__TCPLUSPLUS__&15);
#endif
		cout << endl << SYNFIG_COPYRIGHT << endl;
		cout << endl;

		throw (SynfigToolException(SYNFIGTOOL_HELP));
	}

	if (_vm.count("version"))
	{
		cerr << PACKAGE << " " << VERSION << endl;

		throw (SynfigToolException(SYNFIGTOOL_HELP));
	}

	if (_vm.count("license"))
	{
		cerr << PACKAGE << " " << VERSION << endl;
		cout << SYNFIG_COPYRIGHT << endl << endl;
		cerr << SYNFIG_LICENSE << endl << endl;

		throw (SynfigToolException(SYNFIGTOOL_HELP));
	}

	if (_vm.count("target-video-codecs"))
	{
		print_target_video_codecs_help();

		throw (SynfigToolException(SYNFIGTOOL_HELP));
	}

	if (_vm.count("layers"))
	{
		synfig::Layer::Book::iterator iter =
			synfig::Layer::book().begin();
		for(; iter != synfig::Layer::book().end(); iter++)
			if (iter->second.category != CATEGORY_DO_NOT_USE)
				cout << (iter->first).c_str() << endl;

		throw (SynfigToolException(SYNFIGTOOL_HELP));
	}

	if (_vm.count("layer-info"))
	{
		Layer::Handle layer =
			synfig::Layer::create(_vm["layer-info"].as<string>());

		cout << _("Layer Name: ") << layer->get_name() << endl;
		cout << _("Localized Layer Name: ")
			 << layer->get_local_name() << endl;
		cout << _("Version: ") << layer->get_version() << endl;

		Layer::Vocab vocab = layer->get_param_vocab();
		for(; !vocab.empty(); vocab.pop_front())
		{
			cout << _("param - ") << vocab.front().get_name().c_str();
			if(!vocab.front().get_critical())
				cout << _(" (not critical)");
			cout << endl << _("\tLocalized Name: ")
				 << vocab.front().get_local_name().c_str() << endl;

			if(!vocab.front().get_description().empty())
				cout << _("\tDescription: ")
					 << vocab.front().get_description().c_str() << endl;

			if(!vocab.front().get_hint().empty())
				cout << _("\tHint: ")
					 << vocab.front().get_hint().c_str() << endl;
		}

		throw (SynfigToolException(SYNFIGTOOL_HELP));
	}

	if (_vm.count("modules"))
	{
		synfig::Module::Book::iterator iter =
			synfig::Module::book().begin();
		for(; iter != synfig::Module::book().end(); iter++)
			cout << (iter->first).c_str() << endl;

		throw (SynfigToolException(SYNFIGTOOL_HELP));
	}

	if (_vm.count("targets"))
	{
		synfig::Target::Book::iterator iter =
			synfig::Target::book().begin();
		for(; iter != synfig::Target::book().end(); iter++)
			cout << (iter->first).c_str() << endl;

		throw (SynfigToolException(SYNFIGTOOL_HELP));
	}

	if (_vm.count("valuenodes"))
	{
		synfig::LinkableValueNode::Book::iterator iter =
			synfig::LinkableValueNode::book().begin();
		for(; iter != synfig::LinkableValueNode::book().end(); iter++)
			cout << (iter->first).c_str() << endl;

		throw (SynfigToolException(SYNFIGTOOL_HELP));
	}

	if (_vm.count("importers"))
	{
		synfig::Importer::Book::iterator iter =
			synfig::Importer::book().begin();
		for(; iter != synfig::Importer::book().end(); iter++)
			cout << (iter->first).c_str() << endl;

		throw (SynfigToolException(SYNFIGTOOL_HELP));
	}
}

RendDesc OptionsProcessor::extract_renddesc(const RendDesc& renddesc)
{
	RendDesc desc = renddesc;
	int w, h;
	float span;
	span = w = h = 0;

	if (_vm.count("width"))
		w = _vm["width"].as<int>();

	if (_vm.count("height"))
		h = _vm["height"].as<int>();

	if (_vm.count("antialias"))
	{
		int a;
		a = _vm["antialias"].as<int>();
		desc.set_antialias(a);
		VERBOSE_OUT(1) << boost::format(_("Antialiasing set to %d, "
										  "(%d samples per pixel)")) % a % (a*a)
						<< std::endl;
	}
	if (_vm.count("span"))
	{
	    span = _vm["span"].as<int>();
		VERBOSE_OUT(1) << boost::format(_("Span set to %d units")) % span
                       << std::endl;
	}
	if (_vm.count("fps"))
	{
		float fps;
		fps = _vm["fps"].as<float>();
		desc.set_frame_rate(fps);
		VERBOSE_OUT(1) << boost::format(_("Frame rate set to %d frames per "
										   "second")) % fps << std::endl;
	}
	if (_vm.count("dpi"))
	{
		float dpi, dots_per_meter;
		dpi = _vm["dpi"].as<float>();
		dots_per_meter = dpi * 39.3700787402;
		desc.set_x_res(dots_per_meter);
		desc.set_y_res(dots_per_meter);
		VERBOSE_OUT(1) << boost::format(_("Physical resolution set to %f "
                                          "dpi")) % dpi << std::endl;
	}
	if (_vm.count("dpi-x"))
	{
		float dpi, dots_per_meter;
		dpi = _vm["dpi-x"].as<float>();
		dots_per_meter = dpi * 39.3700787402;
		desc.set_x_res(dots_per_meter);
		VERBOSE_OUT(1) << boost::format(_("Physical X resolution set to %f "
										  "dpi")) % dpi << std::endl;
	}
	if (_vm.count("dpi-y"))
	{
		float dpi, dots_per_meter;
		dpi = _vm["dpi-y"].as<float>();
		dots_per_meter = dpi * 39.3700787402;
		desc.set_y_res(dots_per_meter);
		VERBOSE_OUT(1) << boost::format(_("Physical Y resolution set to %f "
                                          "dpi")) % dpi << std::endl;
	}
	if (_vm.count("start-time"))
	{
		std::string seconds = _vm["start-time"].as<std::string>();
		desc.set_time_start(Time(seconds.c_str(), desc.get_frame_rate()));
	}
	if (_vm.count("begin-time"))
	{
		std::string seconds = _vm["begin-time"].as<std::string>();
		desc.set_time_start(Time(seconds.c_str(), desc.get_frame_rate()));
	}
	if (_vm.count("end-time"))
	{
		std::string seconds = _vm["end-time"].as<std::string>();
		desc.set_time_end(Time(seconds.c_str(), desc.get_frame_rate()));
	}
	if (_vm.count("time"))
	{
		std::string seconds = _vm["time"].as<std::string>();
		desc.set_time(Time(seconds.c_str(), desc.get_frame_rate()));

		VERBOSE_OUT(1) << _("Rendering frame at ")
					   << desc.get_time_start().get_string(desc.get_frame_rate())
					   << endl;
	}
	if (_vm.count("gamma"))
	{
		synfig::warning(_("Gamma argument is currently ignored"));
		//int gamma;
		//gamma = _vm["gamma"].as<int>();
		//desc.set_gamma(Gamma(gamma));
	}

	if (w || h)
	{
		// scale properly
		if (!w)
			w = desc.get_w() * h / desc.get_h();
		else if (!h)
			h = desc.get_h() * w / desc.get_w();

		desc.set_wh(w, h);
		VERBOSE_OUT(1) << boost::format(_("Resolution set to %dx%d.")) % w % h
                       << std::endl;
	}

	if(span)
		desc.set_span(span);

	return desc;
}

TargetParam OptionsProcessor::extract_targetparam()
{
	TargetParam params;

	// Both parameters are co-dependent
	if (_vm.count("video-codec") ^ _vm.count("video-bitrate"))
		throw (SynfigToolException(SYNFIGTOOL_MISSINGARGUMENT,
									_("Both video codec and bitrate parameters are necessary.")));

	if (_vm.count("video-codec"))
	{
		params.video_codec = _vm["video-codec"].as<std::string>();

		// video_codec string to lowercase
		transform (params.video_codec.begin(),
				   params.video_codec.end(),
				   params.video_codec.begin(),
				   ::tolower);

		bool found = false;
		// Check if the given video codec is allowed.
		for (std::vector<VideoCodec>::const_iterator itr = _allowed_video_codecs.begin();
		 itr != _allowed_video_codecs.end(); ++itr)
		{
			if (params.video_codec == itr->name)
			{
				found = true;
			}
		}

		if (!found)
		{
		    throw SynfigToolException(SYNFIGTOOL_UNKNOWNARGUMENT,
                                      (boost::format(_("Video codec \"%s\" is not supported."))
                                                      % params.video_codec).str());
		}

		VERBOSE_OUT(1) << _("Target video codec set to: ") << params.video_codec
                       << std::endl;
	}
	if(_vm.count("video-bitrate"))
	{
		params.bitrate = _vm["video-bitrate"].as<int>();
		VERBOSE_OUT(1) << _("Target bitrate set to: ") << params.bitrate << "k."
					   << std::endl;
	}
	if(_vm.count("sequence-separator"))
	{
		params.sequence_separator = _vm["sequence-separator"].as<std::string>();
		VERBOSE_OUT(1) << _("Output file sequence separator set to: '")
                       << params.sequence_separator
                       << "'."
					   << std::endl;
	}

	return params;
}

Job OptionsProcessor::extract_job()
{
	Job job;

	// Common input file loading
	if (_vm.count("input-file"))
	{
		job.filename = _vm["input-file"].as<string>();

		// Open the composition
		string errors, warnings;
		try
		{
			// todo: literals ".sfg", "container:", "project.sifz"
			if (bfs::path(job.filename).extension().string() == ".sfg")
			{
				etl::handle< FileContainerZip > container = new FileContainerZip();
				if (container->open(job.filename))
				{
					etl::handle< FileSystemGroup > file_system( new FileSystemGroup(FileSystemNative::instance()) );
					file_system->register_system("#", container);
					job.root = open_canvas_as(file_system->get_identifier("#project.sifz"), job.filename, errors, warnings);
				} else
				{
					errors.append("Cannot open container " + job.filename + "\n");
				}
			} else
			{
				job.root = open_canvas_as(FileSystemNative::instance()->get_identifier(job.filename), job.filename, errors, warnings);
			}
		}
		catch(runtime_error& x)
		{
			job.root = 0;
		}

		// By default, the canvas to render is the root canvas
		// This can be changed through --canvas option
		job.canvas = job.root;

		if(!job.canvas)
		{
		    throw SynfigToolException(SYNFIGTOOL_FILENOTFOUND,
                                      (boost::format(_("Unable to load file '%s'.")) % job.filename).str());
		}

		job.root->set_time(0);
	}
	else
	{
	    throw SynfigToolException(SYNFIGTOOL_MISSINGARGUMENT,
                                  _("No input file provided."));
	}

	if (_vm.count("target"))
	{
		job.target_name = _vm["target"].as<std::string>();
		VERBOSE_OUT(1) << _("Target set to ") << job.target_name << std::endl;
	}

	// Determine output
	if (_vm.count("output-file"))
	{
		job.outfilename = _vm["output-file"].as<std::string>();
	}

	if (_vm.count("extract-alpha"))
	{
		job.extract_alpha = true;
	}

	if (_vm.count("quality"))
		job.quality = _vm["quality"].as<int>();
	else
		job.quality = DEFAULT_QUALITY;

	VERBOSE_OUT(1) << _("Quality set to ") << job.quality << std::endl;

	// WARNING: canvas must be before append

	if (_vm.count("canvas"))
	{
		std::string canvasid;
		canvasid = _vm["canvas"].as<std::string>();

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
                    (boost::format(_("Unable to find canvas with ID \"%s\" in %s.\n"
                                     "Throwing out job..."))
                                   % canvasid % job.filename).str());
		}
		catch(Exception::BadLinkName&)
		{
		    throw SynfigToolException(SYNFIGTOOL_INVALIDJOB,
                    (boost::format(_("Invalid canvas name \"%s\" in %s.\n"
                                     "Throwing out job..."))
                                   % canvasid % job.filename).str());
		}

		// Later we need to set the other parameters for the jobs
	}

	// WARNING: append must be before list-canvases

	if (_vm.count("append"))
	{
		// TODO: Enable multi-appending. Disabled in the previous CLI version
		std::string composite_file = _vm["append"].as<std::string>();

		std::string errors, warnings;
		Canvas::Handle composite;
		// todo: literals ".sfg", "container:", "project.sifz"
		if (bfs::path(composite_file).extension() == ".sfg")
		{
			etl::handle< FileContainerZip > container = new FileContainerZip();
			if (container->open(job.filename))
			{
				etl::handle< FileSystemGroup > file_system( new FileSystemGroup(FileSystemNative::instance()) );
				file_system->register_system("#", container);
				job.root = open_canvas_as(file_system->get_identifier("#project.sifz"), composite_file, errors, warnings);
			} else
			{
				errors.append("Cannot open container " + composite_file + "\n");
			}
		} else
		{
			composite = open_canvas_as(FileSystemNative::instance()->get_identifier(composite_file), composite_file, errors, warnings);
		}

		if(!composite)
		{
			VERBOSE_OUT(1) << _("Unable to append '") << composite_file.c_str()
							<< "'." << endl;
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

		VERBOSE_OUT(2) << _("Appended contents of ") << composite_file << endl;
	}
	/*=== This is a code that comes from bones branch
	      possibly it is the solution for multi-appending mentioned before ====
	// Extract composite
	do{
		string composite_file;
		extract_append(imageargs,composite_file);
		if(!composite_file.empty())
		{
			String errors, warnings;
			Canvas::Handle composite(open_canvas_as(FileSystemNative::instance()->get_identifier(composite_file), composite_file, errors, warnings));
			if(!composite)
			{
				cerr<<_("Unable to append '")<<composite_file<<"'."<<endl;
				break;
			}
			Canvas::reverse_iterator iter;
			for(iter=composite->rbegin();iter!=composite->rend();++iter)
			{
				Layer::Handle layer(*iter);
				//if(layer->active())
					job_list.front().canvas->push_front(layer->clone(composite));
			}
			VERBOSE_OUT(2)<<_("Appended contents of ")<<composite_file<<endl;
		}
	} while(false);

	VERBOSE_OUT(4)<<_("Attempting to determine target/outfile...")<<endl;
	>>>>>>> genete_bones
	 */
	if (_vm.count("list-canvases") || _vm.count("canvases"))
	{
		print_child_canvases(job.filename + "#", job.root);
		std::cerr << std::endl;

		throw SynfigToolException(SYNFIGTOOL_OK);
	}

	if (_vm.count("canvas-info"))
	{
		extract_canvas_info(job);
		print_canvas_info(job);

		throw SynfigToolException(SYNFIGTOOL_OK);
	}

	return job;
}

void OptionsProcessor::print_target_video_codecs_help() const
{
	for (std::vector<VideoCodec>::const_iterator itr = _allowed_video_codecs.begin();
		 itr != _allowed_video_codecs.end(); ++itr)
	{
		std::cout << " " << itr->name << ":   \t" << itr->description
				  << std::endl;
	}
}

#ifdef _DEBUG

// DEBUG auxiliar functions
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
void OptionsProcessor::process_debug_options() throw (SynfigToolException&)
{
	if (_vm.count("signal-test"))
	{
		signal_test();
		throw (SynfigToolException(SYNFIGTOOL_HELP));
	}

	if (_vm.count("guid-test"))
	{
		guid_test();
		throw (SynfigToolException(SYNFIGTOOL_HELP));
	}
}

#endif

