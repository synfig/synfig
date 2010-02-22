/* === S Y N F I G ========================================================= */
/*!	\file tool/main.cpp
**	\brief SYNFIG Tool
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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
#endif

using namespace std;
using namespace etl;
using namespace synfig;

/* === G L O B A L S ======================================================= */

const char *progname;
int verbosity=0;
bool be_quiet=false;
bool print_benchmarks=false;

//! Allowed video codecs
/*! \warning This variable is linked to allowed_video_codecs_description,
 *  if you change this you must change the other acordingly.
 */
const char* allowed_video_codecs[] =
{
	"flv", "gif", "h261", "h263", "h263p", "huffyuv", "libtheora",
	"libx264", "libxvid", "ljpeg", "mjpeg",	"mpeg1video", "mpeg2video",
	"mpeg4", "msmpeg4",	"msmpeg4v1", "msmpeg4v2", "wmv1", "wmv2", NULL
};

//! Allowed video codecs description.
/*! \warning This variable is linked to allowed_video_codecs,
 *  if you change this you must change the other acordingly.
 */
const char* allowed_video_codecs_description[] =
{
	"Flash Video (FLV) / Sorenson Spark / Sorenson H.263.",
	"GIF (Graphics Interchange Format).",
	"H.261.",
	"H.263 / H.263-1996.",
	"H.263+ / H.263-1998 / H.263 version 2.",
	"Huffyuv / HuffYUV.",
	"libtheora Theora.",
	"libx264 H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10.",
	"libxvidcore MPEG-4 part 2.",
	"Lossless JPEG.",
	"MJPEG (Motion JPEG).",
	"raw MPEG-1 video.",
	"raw MPEG-2 video.",
	"MPEG-4 part 2.",
	"MPEG-4 part 2 Microsoft variant version 3.",
	"MPEG-4 part 2 Microsoft variant version 1.",
	"MPEG-4 part 2 Microsoft variant version 2.",
	"Windows Media Video 7.",
	"Windows Media Video 8.",
	NULL
};

/* === T Y P E D E F S ===================================================== */

typedef list<String> arg_list_t;
typedef list<Job> job_list_t;

/* === M E T H O D S ======================================================= */

void guid_test()
{
	cout<<"GUID Test"<<endl;
	for(int i=20;i;i--)
		cout<<synfig::GUID().get_string()<<' '<<synfig::GUID().get_string()<<endl;
}

void signal_test_func()
{
	cout<<"**SIGNAL CALLED**"<<endl;
}

void signal_test()
{
	sigc::signal<void> sig;
	sigc::connection conn;
	cout<<"Signal Test"<<endl;
	conn=sig.connect(sigc::ptr_fun(signal_test_func));
	cout<<"Next line should exclaim signal called."<<endl;
	sig();
	conn.disconnect();
	cout<<"Next line should NOT exclaim signal called."<<endl;
	sig();
	cout<<"done."<<endl;
}

/* === P R O C E D U R E S ================================================= */

void display_help_option(const char *flag, const char *arg, string description)
{
	const char spaces[]="                      ";
	if(arg)
		cerr << strprintf(" %s %s %s", flag, arg, spaces+strlen(arg)+strlen(flag)+1)+description << endl;
	else
		cerr << strprintf(" %s %s", flag, spaces+strlen(flag))+description << endl;
}

void display_help(bool full)
{
	cerr << endl << _("syntax: ") << progname << " [DEFAULT OPTIONS] ([SIF FILE] [SPECIFIC OPTIONS])..." << endl << endl;

	if(full)
	{
		display_help_option("-t", "<output type>", _("Specify output target (Default:unknown)"));
		display_help_option("-w", "<pixel width>", _("Set the image width (Use zero for file default)"));
		display_help_option("-h", "<pixel height>", _("Set the image height (Use zero for file default)"));
		display_help_option("-s", "<image dist>", _("Set the diagonal size of image window (Span)"));
		display_help_option("-a", "<1...30>", _("Set antialias amount for parametric renderer."));
		display_help_option("-Q", "<0...10>", strprintf(_("Specify image quality for accelerated renderer (default=%d)"), DEFAULT_QUALITY).c_str());
		display_help_option("-g", "<amount>", _("Gamma (default=2.2)"));
		display_help_option("-v", NULL, _("Verbose Output (add more for more verbosity)"));
		display_help_option("-q", NULL, _("Quiet mode (No progress/time-remaining display)"));
		display_help_option("-c", "<canvas id>", _("Render the canvas with the given id instead of the root."));
		display_help_option("-o", "<output file>", _("Specify output filename"));
		display_help_option("-T", "<# of threads>", _("Enable multithreaded renderer using specified # of threads"));
		display_help_option("-b", NULL, _("Print Benchmarks"));
		display_help_option("--fps", "<framerate>", _("Set the frame rate"));
		display_help_option("--time", "<time>", _("Render a single frame at <seconds>"));
		display_help_option("--begin-time", "<time>", _("Set the starting time"));
		display_help_option("--start-time", "<time>", _("Set the starting time"));
		display_help_option("--end-time", "<time>", _("Set the ending time"));
		display_help_option("--dpi", "<res>", _("Set the physical resolution (dots-per-inch)"));
		display_help_option("--dpi-x", "<res>", _("Set the physical X resolution (dots-per-inch)"));
		display_help_option("--dpi-y", "<res>", _("Set the physical Y resolution (dots-per-inch)"));

		display_help_option("--list-canvases", NULL, _("List the exported canvases in the composition"));
		display_help_option("--canvas-info", "<fields>", _("Print out specified details of the root canvas"));
		display_help_option("--append", "<filename>", _("Append layers in <filename> to composition"));

		display_help_option("--layer-info", "<layer>", _("Print out layer's description, parameter info, etc."));
		display_help_option("--layers", NULL, _("Print out the list of available layers"));
		display_help_option("--targets", NULL, _("Print out the list of available targets"));
		display_help_option("--target-video-codecs", NULL, _("Print out the list of available target video codecs"));
		display_help_option("--importers", NULL, _("Print out the list of available importers"));
		display_help_option("--valuenodes", NULL, _("Print out the list of available ValueNodes"));
		display_help_option("--modules", NULL, _("Print out the list of loaded modules"));
		display_help_option("--version", NULL, _("Print out version information"));
		display_help_option("--info", NULL, _("Print out misc build information"));
		display_help_option("--license", NULL, _("Print out license information"));

#ifdef _DEBUG
		display_help_option("--guid-test", NULL, _("Test GUID generation"));
		display_help_option("--signal-test", NULL, _("Test signal implementation"));
#endif
	}
	else
		display_help_option("--help", NULL, _("Print out usage and syntax info"));

	cerr << endl;
}

void display_target_video_codecs_help ()
{
	for (int i = 0; allowed_video_codecs[i] != NULL &&
					allowed_video_codecs_description[i] != NULL; i++)
		cout << " " << allowed_video_codecs[i] << ":   \t"
			 << allowed_video_codecs_description[i]
			 << endl;
}

int process_global_flags(arg_list_t &arg_list)
{
	arg_list_t::iterator iter, next;

	for(next=arg_list.begin(),iter=next++;iter!=arg_list.end();iter=next++)
	{
		if(*iter == "--")
			return SYNFIGTOOL_OK;

		if(*iter == "--signal-test")
		{
			signal_test();
			return SYNFIGTOOL_HELP;
		}

		if(*iter == "--guid-test")
		{
			guid_test();
			return SYNFIGTOOL_HELP;
		}

		if(*iter == "--help")
		{
			display_help(true);
			return SYNFIGTOOL_HELP;
		}

		if(*iter == "--info")
		{
			cout<<PACKAGE"-"VERSION<<endl;
			#ifdef DEVEL_VERSION
				cout<<endl<<DEVEL_VERSION<<endl<<endl;
			#endif
			cout<<"Compiled on "__DATE__ /* " at "__TIME__ */;
#ifdef __GNUC__
			cout<<" with GCC "<<__VERSION__;
#endif
#ifdef _MSC_VER
			cout<<" with Microsoft Visual C++ "<<(_MSC_VER>>8)<<'.'<<(_MSC_VER&255);
#endif
#ifdef __TCPLUSPLUS__
			cout<<" with Borland Turbo C++ "<<(__TCPLUSPLUS__>>8)<<'.'<<((__TCPLUSPLUS__&255)>>4)<<'.'<<(__TCPLUSPLUS__&15);
#endif
			cout<<endl<<SYNFIG_COPYRIGHT<<endl;
			cout<<endl;
			return SYNFIGTOOL_HELP;
		}

		if(*iter == "--layers")
		{
			Progress p(PACKAGE);
			synfig::Main synfig_main(dirname(progname),&p);
			synfig::Layer::Book::iterator iter=synfig::Layer::book().begin();
			for(;iter!=synfig::Layer::book().end();iter++)
				if (iter->second.category != CATEGORY_DO_NOT_USE)
					cout<<iter->first<<endl;

			return SYNFIGTOOL_HELP;
		}

		if(*iter == "--layer-info")
		{
			Progress p(PACKAGE);
			synfig::Main synfig_main(dirname(progname),&p);
			iter=next++;
			if (iter==arg_list.end())
			{
				error("The `%s' flag requires a value.  Use --help for a list of options.", "--layer-info");
				return SYNFIGTOOL_MISSINGARGUMENT;
			}
			Layer::Handle layer=synfig::Layer::create(*iter);
			cout<<"Layer Name: "<<layer->get_name()<<endl;
			cout<<"Localized Layer Name: "<<layer->get_local_name()<<endl;
			cout<<"Version: "<<layer->get_version()<<endl;
			Layer::Vocab vocab=layer->get_param_vocab();
			for(;!vocab.empty();vocab.pop_front())
			{
				cout<<"param - "<<vocab.front().get_name();
				if(!vocab.front().get_critical())
					cout<<" (not critical)";
				cout<<endl<<"\tLocalized Name: "<<vocab.front().get_local_name()<<endl;
				if(!vocab.front().get_description().empty())
					cout<<"\tDescription: "<<vocab.front().get_description()<<endl;
				if(!vocab.front().get_hint().empty())
					cout<<"\tHint: "<<vocab.front().get_hint()<<endl;
			}

			return SYNFIGTOOL_HELP;
		}

		if(*iter == "--modules")
		{
			Progress p(PACKAGE);
			synfig::Main synfig_main(dirname(progname),&p);
			synfig::Module::Book::iterator iter=synfig::Module::book().begin();
			for(;iter!=synfig::Module::book().end();iter++)
				cout<<iter->first<<endl;
			return SYNFIGTOOL_HELP;
		}

		if(*iter == "--targets")
		{
			Progress p(PACKAGE);
			synfig::Main synfig_main(dirname(progname),&p);
			synfig::Target::Book::iterator iter=synfig::Target::book().begin();
			for(;iter!=synfig::Target::book().end();iter++)
				cout<<iter->first<<endl;
			return SYNFIGTOOL_HELP;
		}

		if(*iter == "--target-video-codecs")
		{
			display_target_video_codecs_help();

			return SYNFIGTOOL_HELP;
		}

		if(*iter == "--valuenodes")
		{
			Progress p(PACKAGE);
			synfig::Main synfig_main(dirname(progname),&p);
			synfig::LinkableValueNode::Book::iterator iter=synfig::LinkableValueNode::book().begin();
			for(;iter!=synfig::LinkableValueNode::book().end();iter++)
				cout<<iter->first<<endl;
			return SYNFIGTOOL_HELP;
		}

		if(*iter == "--importers")
		{
			Progress p(PACKAGE);
			synfig::Main synfig_main(dirname(progname),&p);
			synfig::Importer::Book::iterator iter=synfig::Importer::book().begin();
			for(;iter!=synfig::Importer::book().end();iter++)
				cout<<iter->first<<endl;
			return SYNFIGTOOL_HELP;
		}

		if(*iter == "--version")
		{
			cerr<<PACKAGE<<" "<<VERSION<<endl;
			arg_list.erase(iter);
			return SYNFIGTOOL_HELP;
		}

		if(*iter == "--license")
		{
			cerr<<PACKAGE<<" "<<VERSION<<endl;
			cout<<SYNFIG_COPYRIGHT<<endl<<endl;
			cerr<<"\
**	This package is free software; you can redistribute it and/or\n\
**	modify it under the terms of the GNU General Public License as\n\
**	published by the Free Software Foundation; either version 2 of\n\
**	the License, or (at your option) any later version.\n\
**\n\
**	" << endl << endl;
			arg_list.erase(iter);
			return SYNFIGTOOL_HELP;
		}

		if(*iter == "-v")
		{
			verbosity++;
			arg_list.erase(iter);
			continue;
		}

		if(*iter == "-q")
		{
			be_quiet=true;
			arg_list.erase(iter);
			continue;
		}
		if(*iter == "-b")
		{
			print_benchmarks=true;
			arg_list.erase(iter);
			continue;
		}
	}

	return SYNFIGTOOL_OK;
}

/* true if the given flag takes an extra parameter */
bool flag_requires_value(String flag)
{
	return (flag=="-a"			|| flag=="-c"			|| flag=="-g"			|| flag=="-h"			|| flag=="-o"			||
			flag=="-Q"			|| flag=="-s"			|| flag=="-t"			|| flag=="-T"			|| flag=="-w"			||
			flag=="--append"	|| flag=="--begin-time"	|| flag=="--canvas-info"|| flag=="--dpi"		|| flag=="--dpi-x"		||
			flag=="--dpi-y"		|| flag=="--end-time"	|| flag=="--fps"		|| flag=="--layer-info"	|| flag=="--start-time"	||
			flag=="--time"		|| flag=="-vc"			|| flag=="-vb");
}

int extract_arg_cluster(arg_list_t &arg_list,arg_list_t &cluster)
{
	arg_list_t::iterator iter, next;

	for(next=arg_list.begin(),iter=next++;iter!=arg_list.end();iter=next++)
	{
		if(*iter->begin() != '-')
		{
			//cerr<<*iter->begin()<<"-----------"<<endl;
			return SYNFIGTOOL_OK;
		}

		if (flag_requires_value(*iter))
		{
			cluster.push_back(*iter);
			arg_list.erase(iter);
			iter=next++;
			if (iter==arg_list.end())
			{
				error("The `%s' flag requires a value.  Use --help for a list of options.", cluster.back().c_str());
				return SYNFIGTOOL_MISSINGARGUMENT;
			}
		}

		cluster.push_back(*iter);
		arg_list.erase(iter);
	}

	return SYNFIGTOOL_OK;
}

/*! Extract a parameter from the argument list
 *
 * \param arg_list Argument list from wich the parameter is extracted.
 * \param iter Iterator pointing to the argument list parameter to be
 * extracted.
 * \param next Iterator pointing to the next argument.
 */
string extract_parameter (arg_list_t& arg_list,
						  arg_list_t::iterator& iter,
						  arg_list_t::iterator& next)
{
	string parameter;
	arg_list.erase(iter);
	iter = next++;
	parameter = *iter;
	arg_list.erase(iter);
	return parameter;
}

int extract_RendDesc(arg_list_t &arg_list,RendDesc &desc)
{
	arg_list_t::iterator iter, next;
	int w=0,h=0;
	float span=0;
	for(next=arg_list.begin(),iter=next++;iter!=arg_list.end();iter=next++)
	{
		if(*iter=="-w")
		{
			w = atoi(extract_parameter(arg_list, iter, next).c_str());
		}
		else if(*iter=="-h")
		{
			h = atoi(extract_parameter(arg_list, iter, next).c_str());
		}
		else if(*iter=="-a")
		{
            int a;
			a = atoi(extract_parameter(arg_list, iter, next).c_str());
			desc.set_antialias(a);
			VERBOSE_OUT(1)<<strprintf(_("Antialiasing set to %d, (%d samples per pixel)"),a,a*a)<<endl;
		}
		else if(*iter=="-s")
		{
			span = atoi(extract_parameter(arg_list, iter, next).c_str());
			VERBOSE_OUT(1)<<strprintf(_("Span set to %d units"),span)<<endl;
		}
		else if(*iter=="--fps")
		{
			float fps = atof(extract_parameter(arg_list, iter, next).c_str());
			desc.set_frame_rate(fps);
			VERBOSE_OUT(1)<<strprintf(_("Frame rate set to %d frames per second"),fps)<<endl;
		}
		else if(*iter=="--dpi")
		{
			float dpi = atof(extract_parameter(arg_list, iter, next).c_str());
			float dots_per_meter=dpi*39.3700787402;
			desc.set_x_res(dots_per_meter).set_y_res(dots_per_meter);
			VERBOSE_OUT(1)<<strprintf(_("Physical resolution set to %f dpi"),dpi)<<endl;
		}
		else if(*iter=="--dpi-x")
		{
			float dpi = atof(extract_parameter(arg_list, iter, next).c_str());
			float dots_per_meter=dpi*39.3700787402;
			desc.set_x_res(dots_per_meter);
			VERBOSE_OUT(1)<<strprintf(_("Physical X resolution set to %f dpi"),dpi)<<endl;
		}
		else if(*iter=="--dpi-y")
		{
			float dpi = atof(extract_parameter(arg_list, iter, next).c_str());
			float dots_per_meter=dpi*39.3700787402;
			desc.set_y_res(dots_per_meter);
			VERBOSE_OUT(1)<<strprintf(_("Physical Y resolution set to %f dpi"),dpi)<<endl;
		}
		else if(*iter=="--start-time" || *iter=="--begin-time")
		{
			desc.set_time_start(Time(extract_parameter(arg_list, iter, next),
								desc.get_frame_rate()));
		}
		else if(*iter=="--end-time")
		{
			desc.set_time_end(Time(extract_parameter(arg_list, iter, next),
								   desc.get_frame_rate()));
		}
		else if(*iter=="--time")
		{
			desc.set_time(Time(extract_parameter(arg_list, iter, next),
							   desc.get_frame_rate()));
			VERBOSE_OUT(1)<<_("Rendering frame at ")<<desc.get_time_start().get_string(desc.get_frame_rate())<<endl;
		}
		else if(*iter=="-g")
		{
			synfig::warning("Gamma argument is currently ignored");
			//desc.set_gamma(Gamma(atoi(extract_parameter(arg_list, iter, next).c_str())));
		}
		else if (flag_requires_value(*iter))
			iter++;
	}
	if (w||h)
	{
		if (!w)
			w = desc.get_w() * h / desc.get_h();
		else if (!h)
			h = desc.get_h() * w / desc.get_w();

		desc.set_wh(w,h);
		VERBOSE_OUT(1)<<strprintf(_("Resolution set to %dx%d"),w,h)<<endl;
	}
	if(span)
		desc.set_span(span);
	return SYNFIGTOOL_OK;
}

int extract_quality(arg_list_t &arg_list,int &quality)
{
	arg_list_t::iterator iter, next;
	for(next=arg_list.begin(),iter=next++;iter!=arg_list.end();iter=next++)
	{
		if(*iter=="-Q")
		{
			quality = atoi(extract_parameter(arg_list, iter, next).c_str());
			VERBOSE_OUT(1)<<strprintf(_("Quality set to %d"),quality)<<endl;
		}
		else if (flag_requires_value(*iter))
			iter++;
	}

	return SYNFIGTOOL_OK;
}

int extract_threads(arg_list_t &arg_list,int &threads)
{
	arg_list_t::iterator iter, next;
	for(next=arg_list.begin(),iter=next++;iter!=arg_list.end();iter=next++)
	{
		if(*iter=="-T")
		{
			threads = atoi(extract_parameter(arg_list, iter, next).c_str());
			VERBOSE_OUT(1)<<strprintf(_("Threads set to %d"),threads)<<endl;
		}
		else if (flag_requires_value(*iter))
			iter++;
	}

	return SYNFIGTOOL_OK;
}

int extract_target(arg_list_t &arg_list,string &type)
{
	arg_list_t::iterator iter, next;
	type.clear();

	for(next=arg_list.begin(),iter=next++;iter!=arg_list.end();iter=next++)
	{
		if(*iter=="-t")
		{
			type = extract_parameter(arg_list, iter, next);
			VERBOSE_OUT(1)<<strprintf(_("Target set to %s"), type.c_str())<<endl;
		}
		else if (flag_requires_value(*iter))
			iter++;
	}

	return SYNFIGTOOL_OK;
}

int extract_target_params(arg_list_t& arg_list,
						  TargetParam& params)
{
	int ret;
	ret = SYNFIGTOOL_OK;
	// If -vc parameter is provided, -vb parameter is needed.
	bool need_bitrate_parameter = false;
	arg_list_t::iterator iter, next;

	for(next=arg_list.begin(),iter=next++;iter!=arg_list.end();iter=next++)
	{
		if(*iter=="-vc")
		{
			// Target video codec
			params.video_codec = extract_parameter(arg_list, iter, next);

			// video_codec string to lowercase
			transform (params.video_codec.begin(),
					   params.video_codec.end(),
					   params.video_codec.begin(),
					   ::tolower);

			int local_ret;
			local_ret = SYNFIGTOOL_UNKNOWNARGUMENT;

			// Check if the given video codec is allowed.
			for (int i = 0; local_ret != SYNFIGTOOL_OK &&
							allowed_video_codecs[i] != NULL; i++)
				if (params.video_codec == allowed_video_codecs[i])
					local_ret = SYNFIGTOOL_OK;

			ret = local_ret;

			if (ret == SYNFIGTOOL_OK)
			{
				VERBOSE_OUT(1)<<strprintf(_("Target video codec set to %s"), params.video_codec.c_str())<<endl;
				need_bitrate_parameter = true;
			}
		}
		else if(*iter=="-vb")
		{
			need_bitrate_parameter = false;
			// Target bitrate
			params.bitrate =
				atoi(extract_parameter(arg_list, iter, next).c_str());
			VERBOSE_OUT(1)<<strprintf(_("Target bitrate set to %dk"),params.bitrate)<<endl;
		}
		else if (flag_requires_value(*iter))
			iter++;
	}

	if (need_bitrate_parameter)
		ret = SYNFIGTOOL_MISSINGARGUMENT;

	return ret;
}

int extract_append(arg_list_t &arg_list,string &filename)
{
	arg_list_t::iterator iter, next;
	filename.clear();

	for(next=arg_list.begin(),iter=next++;iter!=arg_list.end();iter=next++)
	{
		if(*iter=="--append")
		{
			filename = extract_parameter(arg_list, iter, next);
		}
		else if (flag_requires_value(*iter))
			iter++;
	}

	return SYNFIGTOOL_OK;
}

int extract_outfile(arg_list_t &arg_list,string &outfile)
{
	arg_list_t::iterator iter, next;
	int ret=SYNFIGTOOL_FILENOTFOUND;
	outfile.clear();

	for(next=arg_list.begin(),iter=next++;iter!=arg_list.end();iter=next++)
	{
		if(*iter=="-o")
		{
			outfile = extract_parameter(arg_list, iter, next);
			ret=SYNFIGTOOL_OK;
		}
		else if (flag_requires_value(*iter))
			iter++;
	}

	return ret;
}

int extract_canvasid(arg_list_t &arg_list,string &canvasid)
{
	arg_list_t::iterator iter, next;
	//canvasid.clear();

	for(next=arg_list.begin(),iter=next++;iter!=arg_list.end();iter=next++)
	{
		if(*iter=="-c")
		{
			canvasid = extract_parameter(arg_list, iter, next);
		}
		else if (flag_requires_value(*iter))
			iter++;
	}

	return SYNFIGTOOL_OK;
}

int extract_list_canvases(arg_list_t &arg_list,bool &list_canvases)
{
	arg_list_t::iterator iter, next;

	for(next=arg_list.begin(), iter = next++; iter!=arg_list.end();
		iter = next++)
		if(*iter=="--list-canvases")
		{
			list_canvases = true;
			arg_list.erase(iter);
		}

	return SYNFIGTOOL_OK;
}

void extract_canvas_info(arg_list_t &arg_list, Job &job)
{
	arg_list_t::iterator iter, next;

	for(next=arg_list.begin(),iter=next++;iter!=arg_list.end();iter=next++)
		if(*iter=="--canvas-info")
		{
			job.canvas_info = true;
			String values(extract_parameter(arg_list, iter, next)), value;

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
					cerr<<_("Unrecognised canvas variable: ") << "'" << value << "'" << endl;
					cerr<<_("Recognized variables are:") << endl <<
						"  all, time_start, time_end, frame_rate, frame_start, frame_end, w, h," << endl <<
						"  image_aspect, pw, ph, pixel_aspect, tl, br, physical_w, physical_h," << endl <<
						"  x_res, y_res, span, interlaced, antialias, clamp, flags," << endl <<
						"  focus, bg_color, metadata" << endl;
				}

				if (pos == std::string::npos)
					break;
			};
		}
}

void list_child_canvases(string prefix, Canvas::Handle canvas)
{
	Canvas::Children children(canvas->children());
	for (Canvas::Children::iterator iter = children.begin(); iter != children.end(); iter++)
	{
		cout << prefix << ":" << (*iter)->get_id() << endl;
		list_child_canvases(prefix + ":" + (*iter)->get_id(), *iter);
	}
}

void list_canvas_info(Job job)
{
	Canvas::Handle canvas(job.canvas);
	const RendDesc &rend_desc(canvas->rend_desc());

	if (job.canvas_info_all || job.canvas_info_time_start)
	{
		cout << endl << "# " << _("Start Time") << endl;
		cout << "time_start"	<< "=" << rend_desc.get_time_start().get_string().c_str() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_time_end)
	{
		cout << endl << "# " << _("End Time") << endl;
		cout << "time_end"		<< "=" << rend_desc.get_time_end().get_string().c_str() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_frame_rate)
	{
		cout << endl << "# " << _("Frame Rate") << endl;
		cout << "frame_rate"	<< "=" << rend_desc.get_frame_rate() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_frame_start)
	{
		cout << endl << "# " << _("Start Frame") << endl;
		cout << "frame_start"	<< "=" << rend_desc.get_frame_start() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_frame_end)
	{
		cout << endl << "# " << _("End Frame") << endl;
		cout << "frame_end"		<< "=" << rend_desc.get_frame_end() << endl;
	}

	if (job.canvas_info_all)
		cout << endl;

	if (job.canvas_info_all || job.canvas_info_w)
	{
		cout << endl << "# " << _("Width") << endl;
		cout << "w"				<< "=" << rend_desc.get_w() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_h)
	{
		cout << endl << "# " << _("Height") << endl;
		cout << "h"				<< "=" << rend_desc.get_h() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_image_aspect)
	{
		cout << endl << "# " << _("Image Aspect Ratio") << endl;
		cout << "image_aspect"	<< "=" << rend_desc.get_image_aspect() << endl;
	}

	if (job.canvas_info_all)
		cout << endl;

	if (job.canvas_info_all || job.canvas_info_pw)
	{
		cout << endl << "# " << _("Pixel Width") << endl;
		cout << "pw"			<< "=" << rend_desc.get_pw() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_ph)
	{
		cout << endl << "# " << _("Pixel Height") << endl;
		cout << "ph"			<< "=" << rend_desc.get_ph() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_pixel_aspect)
	{
		cout << endl << "# " << _("Pixel Aspect Ratio") << endl;
		cout << "pixel_aspect"	<< "=" << rend_desc.get_pixel_aspect() << endl;
	}

	if (job.canvas_info_all)
		cout << endl;

	if (job.canvas_info_all || job.canvas_info_tl)
	{
		cout << endl << "# " << _("Top Left") << endl;
		cout << "tl"			<< "=" << rend_desc.get_tl()[0]
			 << " " << rend_desc.get_tl()[1] << endl;
	}

	if (job.canvas_info_all || job.canvas_info_br)
	{
		cout << endl << "# " << _("Bottom Right") << endl;
		cout << "br"			<< "=" << rend_desc.get_br()[0]
			 << " " << rend_desc.get_br()[1] << endl;
	}

	if (job.canvas_info_all || job.canvas_info_physical_w)
	{
		cout << endl << "# " << _("Physical Width") << endl;
		cout << "physical_w"	<< "=" << rend_desc.get_physical_w() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_physical_h)
	{
		cout << endl << "# " << _("Physical Height") << endl;
		cout << "physical_h"	<< "=" << rend_desc.get_physical_h() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_x_res)
	{
		cout << endl << "# " << _("X Resolution") << endl;
		cout << "x_res"			<< "=" << rend_desc.get_x_res() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_y_res)
	{
		cout << endl << "# " << _("Y Resolution") << endl;
		cout << "y_res"			<< "=" << rend_desc.get_y_res() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_span)
	{
		cout << endl << "# " << _("Diagonal Image Span") << endl;
		cout << "span"			<< "=" << rend_desc.get_span() << endl;
	}

	if (job.canvas_info_all)
		cout << endl;

	if (job.canvas_info_all || job.canvas_info_interlaced)
	{
		cout << endl << "# " << _("Interlaced") << endl;
		cout << "interlaced"	<< "=" << rend_desc.get_interlaced() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_antialias)
	{
		cout << endl << "# " << _("Antialias") << endl;
		cout << "antialias"		<< "=" << rend_desc.get_antialias() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_clamp)
	{
		cout << endl << "# " << _("Clamp") << endl;
		cout << "clamp"			<< "=" << rend_desc.get_clamp() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_flags)
	{
		cout << endl << "# " << _("Flags") << endl;
		cout << "flags"			<< "=" << rend_desc.get_flags() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_focus)
	{
		cout << endl << "# " << _("Focus") << endl;
		cout << "focus"			<< "=" << rend_desc.get_focus()[0]
			 << " " << rend_desc.get_focus()[1] << endl;
	}

	if (job.canvas_info_all || job.canvas_info_bg_color)
	{
		cout << endl << "# " << _("Background Color") << endl;
		cout << "bg_color"		<< "=" << rend_desc.get_bg_color().get_string().c_str() << endl;
	}

	if (job.canvas_info_all)
		cout << endl;

	if (job.canvas_info_all || job.canvas_info_metadata)
	{
		std::list<String> keys(canvas->get_meta_data_keys());
		cout << endl << "# " << _("Metadata") << endl;
		for (std::list<String>::iterator iter = keys.begin(); iter != keys.end(); iter++)
			cout << *iter << "=" << canvas->get_meta_data(*iter) << endl;
	}
}

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

int main(int argc, char *argv[])
{
	int i;
	arg_list_t arg_list;
	job_list_t job_list;

	setlocale(LC_ALL, "");

#ifdef ENABLE_NLS
	bindtextdomain("synfig", LOCALEDIR);
	bind_textdomain_codeset("synfig", "UTF-8");
	textdomain("synfig");
#endif

	progname=argv[0];
	Progress p(argv[0]);

	if(!SYNFIG_CHECK_VERSION())
	{
		cerr<<_("FATAL: Synfig Version Mismatch")<<endl;
		return SYNFIGTOOL_BADVERSION;
	}

	if(argc==1)
	{
		display_help(false);
		return SYNFIGTOOL_BLANK;
	}

	for(i=1;i<argc;i++)
		arg_list.push_back(argv[i]);

	if((i=process_global_flags(arg_list)))
		return i;

	VERBOSE_OUT(1)<<_("verbosity set to ")<<verbosity<<endl;
	synfig::Main synfig_main(dirname(progname),&p);

	{
		arg_list_t defaults, imageargs;
		int ret;

		// Grab the defaults before the first file
		if ((ret = extract_arg_cluster(arg_list,defaults)) != SYNFIGTOOL_OK)
		  return ret;

		while(arg_list.size())
		{
			string target_name;
			job_list.push_front(Job());
			int threads=0;

			imageargs=defaults;
			job_list.front().filename=arg_list.front();
			arg_list.pop_front();

			if ((ret = extract_arg_cluster(arg_list,imageargs)) != SYNFIGTOOL_OK)
			  return ret;

			// Open the composition
			String errors, warnings;
			try
			{
				job_list.front().root=open_canvas(job_list.front().filename, errors, warnings);
			}
			catch(runtime_error x)
			{
				job_list.front().root = 0;
			}

			if(!job_list.front().root)
			{
				cerr<<_("Unable to load '")<<job_list.front().filename<<"'."<<endl;
				cerr<<_("Throwing out job...")<<endl;
				job_list.pop_front();
				continue;
			}

			bool list_canvases = false;
			extract_list_canvases(imageargs, list_canvases);
			job_list.front().list_canvases = list_canvases;

			extract_canvas_info(imageargs, job_list.front());

			job_list.front().root->set_time(0);

			string canvasid;
			extract_canvasid(imageargs,canvasid);
			if(!canvasid.empty())
			{
				try
				{
					String warnings;
					job_list.front().canvas=job_list.front().root->find_canvas(canvasid, warnings);
				}
				catch(Exception::IDNotFound)
				{
					cerr<<_("Unable to find canvas with ID \"")<<canvasid<<_("\" in ")<<job_list.front().filename<<"."<<endl;
					cerr<<_("Throwing out job...")<<endl;
					job_list.pop_front();
					continue;

				}
				catch(Exception::BadLinkName)
				{
					cerr<<_("Invalid canvas name \"")<<canvasid<<_("\" in ")<<job_list.front().filename<<"."<<endl;
					cerr<<_("Throwing out job...")<<endl;
					job_list.pop_front();
					continue;
				}
			}
			else
				job_list.front().canvas=job_list.front().root;

			extract_RendDesc(imageargs,job_list.front().canvas->rend_desc());
			extract_target(imageargs,target_name);
			extract_threads(imageargs,threads);
			job_list.front().quality=DEFAULT_QUALITY;
			extract_quality(imageargs,job_list.front().quality);
			VERBOSE_OUT(2)<<_("Quality set to ")<<job_list.front().quality<<endl;
			job_list.front().desc=job_list.front().canvas->rend_desc();
			extract_outfile(imageargs,job_list.front().outfilename);

			// Extract composite
			do{
				string composite_file;
				extract_append(imageargs,composite_file);
				if(!composite_file.empty())
				{
					String errors, warnings;
					Canvas::Handle composite(open_canvas(composite_file, errors, warnings));
					if(!composite)
					{
						cerr<<_("Unable to append '")<<composite_file<<"'."<<endl;
						break;
					}
					Canvas::reverse_iterator iter;
					for(iter=composite->rbegin();iter!=composite->rend();++iter)
					{
						Layer::Handle layer(*iter);
						if(layer->active())
							job_list.front().canvas->push_front(layer->clone());
					}
					VERBOSE_OUT(2)<<_("Appended contents of ")<<composite_file<<endl;
				}
			} while(false);

			VERBOSE_OUT(4)<<_("Attempting to determine target/outfile...")<<endl;

			// If the target type is not yet defined,
			// try to figure it out from the outfile.
			if(target_name.empty() && !job_list.front().outfilename.empty())
			{
				VERBOSE_OUT(3)<<_("Target name undefined, attempting to figure it out")<<endl;
				string ext = filename_extension(job_list.front().outfilename);
				if (ext.length()) ext = ext.substr(1);
				if(Target::ext_book().count(ext))
				{
					target_name=Target::ext_book()[ext];
					info("target name not specified - using %s", target_name.c_str());
				}
				else
				{
					string lower_ext(ext);

					for(unsigned int i=0;i<ext.length();i++)
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

			TargetParam target_parameters;
			// Extract the extra parameters for the targets that
			// need them.
			if (target_name == "ffmpeg")
			{
				int status;
				status = extract_target_params(imageargs, target_parameters);
				if (status == SYNFIGTOOL_UNKNOWNARGUMENT)
				{
					cerr << strprintf(_("Unknown target video codec: %s."),
									 target_parameters.video_codec.c_str())
						 << endl;
					cerr << _("Available target video codecs are:")
						 << endl;
					display_target_video_codecs_help();

					return SYNFIGTOOL_UNKNOWNARGUMENT;
				}
				else if (status == SYNFIGTOOL_MISSINGARGUMENT)
				{
					cerr << _("Missing argument: \"-vb\".") << endl;

					return SYNFIGTOOL_MISSINGARGUMENT;
				}
			}

			// If the target type is STILL not yet defined, then
			// set it to a some sort of default
			if(target_name.empty())
			{
				VERBOSE_OUT(2)<<_("Defaulting to PNG target...")<<endl;
				target_name="png";
			}

			// If no output filename was provided, then
			// create a output filename based on the
			// given input filename. (ie: change the extension)
			if(job_list.front().outfilename.empty())
			{
				job_list.front().outfilename = filename_sans_extension(job_list.front().filename) + '.';
				if(Target::book().count(target_name))
					job_list.front().outfilename+=Target::book()[target_name].filename;
				else
					job_list.front().outfilename+=target_name;
			}

			VERBOSE_OUT(4)<<"target_name="<<target_name<<endl;
			VERBOSE_OUT(4)<<"outfile_name="<<job_list.front().outfilename<<endl;

			VERBOSE_OUT(4)<<_("Creating the target...")<<endl;
			job_list.front().target =
				synfig::Target::create(target_name,
									   job_list.front().outfilename,
									   target_parameters);

			if(target_name=="sif")
				job_list.front().sifout=true;
			else
			{
				if(!job_list.front().target)
				{
					cerr<<_("Unknown target for ")<<job_list.front().filename<<": "<<target_name<<endl;
					cerr<<_("Throwing out job...")<<endl;
					job_list.pop_front();
					continue;
				}
				job_list.front().sifout=false;
			}

			// Set the Canvas on the Target
			if(job_list.front().target)
			{
				VERBOSE_OUT(4)<<_("Setting the canvas on the target...")<<endl;
				job_list.front().target->set_canvas(job_list.front().canvas);
				VERBOSE_OUT(4)<<_("Setting the quality of the target...")<<endl;
				job_list.front().target->set_quality(job_list.front().quality);
			}

			// Set the threads for the target
			if(job_list.front().target && Target_Scanline::Handle::cast_dynamic(job_list.front().target))
				Target_Scanline::Handle::cast_dynamic(job_list.front().target)->set_threads(threads);

			if(imageargs.size())
			{
				cerr<<_("Unidentified arguments for ")<<job_list.front().filename<<": ";
				for(;imageargs.size();imageargs.pop_front())
					cerr<<' '<<imageargs.front();
				cerr<<endl;
				cerr<<_("Throwing out job...")<<endl;
				job_list.pop_front();
				continue;
			}
		}
	}

	if(arg_list.size())
	{
		cerr<<_("Unidentified arguments:");
		for(;arg_list.size();arg_list.pop_front())
			cerr<<' '<<arg_list.front();
		cerr<<endl;
		return SYNFIGTOOL_UNKNOWNARGUMENT;
	}

	if(!job_list.size())
	{
		cerr<<_("Nothing to do!")<<endl;
		return SYNFIGTOOL_BORED;
	}

	for(;job_list.size();job_list.pop_front())
	{
		VERBOSE_OUT(3)<<job_list.front().filename<<" -- "<<endl<<'\t'<<
		strprintf("w:%d, h:%d, a:%d, pxaspect:%f, imaspect:%f, span:%f",
			job_list.front().desc.get_w(),
			job_list.front().desc.get_h(),
			job_list.front().desc.get_antialias(),
			job_list.front().desc.get_pixel_aspect(),
			job_list.front().desc.get_image_aspect(),
			job_list.front().desc.get_span()
			)<<endl<<'\t'<<
		strprintf("tl:[%f,%f], br:[%f,%f], focus:[%f,%f]",
			job_list.front().desc.get_tl()[0],job_list.front().desc.get_tl()[1],
			job_list.front().desc.get_br()[0],job_list.front().desc.get_br()[1],
			job_list.front().desc.get_focus()[0],job_list.front().desc.get_focus()[1]
			)<<endl;

		RenderProgress p;
		p.task(job_list.front().filename+" ==> "+job_list.front().outfilename);
		if(job_list.front().sifout)
		{
			if(!save_canvas(job_list.front().outfilename,job_list.front().canvas))
			{
				cerr<<"Render Failure."<<endl;
				return SYNFIGTOOL_RENDERFAILURE;
			}
		}
		else if (job_list.front().list_canvases)
		{
			list_child_canvases(job_list.front().filename + "#", job_list.front().canvas);
			cerr << endl;
		}
		else if (job_list.front().canvas_info)
		{
			list_canvas_info(job_list.front());
			cerr << endl;
		}
		else
		{
			VERBOSE_OUT(1)<<_("Rendering...")<<endl;
			etl::clock timer;
			timer.reset();
			// Call the render member of the target
			if(!job_list.front().target->render(&p))
			{
				cerr<<"Render Failure."<<endl;
				return SYNFIGTOOL_RENDERFAILURE;
			}
			if(print_benchmarks)
				cout<<job_list.front().filename+": Rendered in "<<timer()<<" seconds."<<endl;
		}
	}

	job_list.clear();

	VERBOSE_OUT(1)<<_("Done.")<<endl;

	return SYNFIGTOOL_OK;
}
