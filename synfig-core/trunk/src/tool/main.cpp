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
#include <synfig/time.h>
#include <synfig/string.h>
#include <synfig/paramdesc.h>
#include <synfig/main.h>
#include <synfig/guid.h>
#endif

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

#ifdef ENABLE_NLS
#undef _
#define _(x) gettext(x)
#else
#undef _
#define _(x) (x)
#endif

enum exit_code
{
	SYNFIGTOOL_OK				= 0,
	SYNFIGTOOL_FILENOTFOUND		= 1,
	SYNFIGTOOL_BORED			= 2,
	SYNFIGTOOL_HELP				= 3,
	SYNFIGTOOL_UNKNOWNARGUMENT	= 4,
	SYNFIGTOOL_UNKNOWNERROR		= 5,
	SYNFIGTOOL_INVALIDTARGET	= 6,
	SYNFIGTOOL_RENDERFAILURE	= 7,
	SYNFIGTOOL_BLANK			= 8,
	SYNFIGTOOL_BADVERSION		= 9,
	SYNFIGTOOL_MISSINGARGUMENT	=10
};

#ifndef VERSION
#define VERSION "unknown"
#define PACKAGE "synfig-tool"
#endif

#ifdef DEFAULT_QUALITY
#undef DEFAULT_QUALITY
#endif

#define DEFAULT_QUALITY		2
#define VERBOSE_OUT(x) if(verbosity>=(x))std::cerr

/* === G L O B A L S ======================================================= */

const char *progname;
int verbosity=0;
bool be_quiet=false;
bool print_benchmarks=false;

/* === M E T H O D S ======================================================= */

class Progress : public synfig::ProgressCallback
{
	const char *program;

public:

	Progress(const char *name):program(name) { }

	virtual bool
	task(const String &task)
	{
		VERBOSE_OUT(1)<<program<<": "<<task<<std::endl;
		return true;
	}

	virtual bool
	error(const String &task)
	{
		std::cerr<<program<<": "<<_("error")<<": "<<task<<std::endl;
		return true;
	}

	virtual bool
	warning(const String &task)
	{
		std::cerr<<program<<": "<<_("warning")<<": "<<task<<std::endl;
		return true;
	}

	virtual bool
	amount_complete(int /*current*/, int /*total*/)
	{
		return true;
	}
};

class RenderProgress : public synfig::ProgressCallback
{
	string taskname;

	etl::clock clk;
	int clk_scanline; // The scanline at which the clock was reset
	etl::clock clk2;

	float last_time;
public:

	RenderProgress():clk_scanline(0), last_time(0) { }

	virtual bool
	task(const String &thetask)
	{
		taskname=thetask;
		return true;
	}

	virtual bool
	error(const String &task)
	{
		std::cout<<_("error")<<": "<<task<<std::endl;
		return true;
	}

	virtual bool
	warning(const String &task)
	{
		std::cout<<_("warning")<<": "<<task<<std::endl;
		return true;
	}

	virtual bool
	amount_complete(int scanline, int h)
	{
		if(be_quiet)return true;
		if(scanline!=h)
		{
			const float time(clk()*(float)(h-scanline)/(float)(scanline-clk_scanline));
			const float delta(time-last_time);

			int weeks=0,days=0,hours=0,minutes=0,seconds=0;

			last_time=time;

			if(clk2()<0.2)
				return true;
			clk2.reset();

			if(scanline)
				seconds=(int)time+1;
			else
			{
				//cerr<<"reset"<<endl;
				clk.reset();
				clk_scanline=scanline;
			}

			if(seconds<0)
			{
				clk.reset();
				clk_scanline=scanline;
				seconds=0;
			}
			while(seconds>=60)
				minutes++,seconds-=60;
			while(minutes>=60)
				hours++,minutes-=60;
			while(hours>=24)
				days++,hours-=24;
			while(days>=7)
				weeks++,days-=7;

			cerr<<taskname<<": "<<_("Line")<<" "<<scanline<<_(" of ")<<h<<" -- ";
			//cerr<<time/(h-clk_scanline)<<" ";
			/*
			if(delta>=-time/(h-clk_scanline)  )
				cerr<<">";
			*/
			if(delta>=0 && clk()>4.0 && scanline>clk_scanline+200)
			{
				//cerr<<"reset"<<endl;
				clk.reset();
				clk_scanline=scanline;
			}

			if(weeks)
				cerr<<weeks<<"w ";
			if(days)
				cerr<<days<<"d ";
			if(hours)
				cerr<<hours<<"h ";
			if(minutes)
				cerr<<minutes<<"m ";
			if(seconds)
				cerr<<seconds<<"s ";

			cerr<<"           \r";
		}
		else
			cerr<<taskname<<": "<<_("DONE")<<"                        "<<endl;;
		return true;
	}
};

struct Job
{
	String filename;
	String outfilename;

	RendDesc desc;

	Canvas::Handle root;
	Canvas::Handle canvas;
	Target::Handle target;

	int quality;
	bool sifout;
};

typedef list<String> arg_list_t;
typedef list<Job> job_list_t;

void guid_test()
{
	cout<<"GUID Test"<<endl;
	for(int i=20;i;i--)
	{
		cout<<synfig::GUID().get_string()<<' '<<synfig::GUID().get_string()<<endl;
	}
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

void display_help(int amount)
{
	class Argument
	{
	public:
		Argument(const char *flag,const char *arg, string description)
		{
			const char spaces[]="                      ";
			if(arg)
				cerr<<strprintf(" %s %s %s",flag, arg, spaces+strlen(arg)+strlen(flag)+1)+description<<endl;
			else
				cerr<<strprintf(" %s %s",flag,spaces+strlen(flag))+description<<endl;
		}
	};

	cerr << endl << _("syntax: ") << progname << " [DEFAULT OPTIONS] ([SIF FILE] [SPECIFIC OPTIONS])..." << endl << endl;

	if(amount == 0)
		Argument("--help",NULL,_("Print out usage and syntax info"));
	else
	{
		Argument("-t","<output type>",_("Specify output target (Default:unknown)"));
		Argument("-w","<pixel width>",_("Set the image width (Use zero for file default)"));
		Argument("-h","<pixel height>",_("Set the image height (Use zero for file default)"));
		Argument("-s","<image dist>",_("Set the diagonal size of image window (Span)"));
		Argument("-a","<1...30>",_("Set antialias amount for parametric renderer."));
		Argument("-Q","<0...10>",strprintf(_("Specify image quality for accelerated renderer (default=%d)"),DEFAULT_QUALITY).c_str());
		Argument("-g","<amount>",_("Gamma (default=2.2)"));
		Argument("-v",NULL,_("Verbose Output (add more for more verbosity)"));
		Argument("-q",NULL,_("Quiet mode (No progress/time-remaining display)"));
		Argument("-c","<canvas id>",_("Render the canvas with the given id instead of the root."));
		Argument("-o","<output file>",_("Specify output filename"));
		Argument("-T","<# of threads>",_("Enable multithreaded renderer using specified # of threads"));
		Argument("-b",NULL,_("Print Benchmarks"));
		Argument("--fps","<framerate>",_("Set the frame rate"));
		Argument("--time","<time>",_("Render a single frame at <seconds>"));
		Argument("--begin-time","<time>",_("Set the starting time"));
		Argument("--end-time","<time>",_("Set the ending time"));
		Argument("--dpi","<res>",_("Set the physical resolution (dots-per-inch)"));
		Argument("--dpi-x","<res>",_("Set the physical X resolution (dots-per-inch"));
		Argument("--dpi-y","<res>",_("Set the physical Y resolution (dots-per-inch)"));
		Argument("--append","<filename>",_("Append layers in <filename> to composition"));
		Argument("--layer-info","<layer>",_("Print out layer's description, parameter info, etc."));
		Argument("--layers",NULL,_("Print out the list of available layers"));
		Argument("--targets",NULL,_("Print out the list of available targets"));
		Argument("--importers",NULL,_("Print out the list of available importers"));
		Argument("--valuenodes",NULL,_("Print out the list of available ValueNodes"));
		Argument("--modules",NULL,_("Print out the list of loaded modules"));
		Argument("--version",NULL,_("Print out version information"));
		Argument("--info",NULL,_("Print out misc build information"));
		Argument("--license",NULL,_("Print out license information"));

#ifdef _DEBUG
		Argument("--guid-test",NULL,_("Test GUID generation"));
		Argument("--signal-test",NULL,_("Test signal implementation"));
#endif
	}

	cerr<<endl;
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
			display_help(1);
			return SYNFIGTOOL_HELP;
		}

		if(*iter == "--info")
		{
			cout<<PACKAGE"-"VERSION<<endl;
			cout<<"Compiled on "__DATE__ " at "__TIME__;
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
bool flag_requires_value(String flag) {
	return (flag=="-a"	|| flag=="-c"	|| flag=="-g"	|| flag=="-h"	|| flag=="-o" ||
			flag=="-Q"	|| flag=="-s"	|| flag=="-t"	|| flag=="-T"	|| flag=="-w" ||
			flag=="--append"	|| flag=="--begin-time"		|| flag=="--dpi"		|| flag=="--dpi-x"	||
			flag=="--dpi-y"		|| flag=="--end-frame"		|| flag=="--end-time"	|| flag=="--fps"	||
			flag=="--frame"		|| flag=="--start-frame"	|| flag=="--start-time"	|| flag=="--time"	);
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
			if (iter==arg_list.end()) {
				error("The `%s' flag requires a value.  Use --help for a list of options.", cluster.back().c_str());
				return SYNFIGTOOL_MISSINGARGUMENT;
			}
		}

		cluster.push_back(*iter);
		arg_list.erase(iter);
	}

	return SYNFIGTOOL_OK;
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
			arg_list.erase(iter);
			iter=next++;
			w=atoi(iter->c_str());
			arg_list.erase(iter);
		}
		else if(*iter=="-h")
		{
			arg_list.erase(iter);
			iter=next++;
			h=atoi(iter->c_str());
			arg_list.erase(iter);
		}
		else if(*iter=="-a")
		{
            int a;
			arg_list.erase(iter);
			iter=next++;
			a=atoi(iter->c_str());
			desc.set_antialias(a);
			VERBOSE_OUT(1)<<strprintf(_("Antialiasing set to %d, (%d samples per pixel)"),a,a*a)<<endl;
			arg_list.erase(iter);
		}
		else if(*iter=="-s")
		{
			arg_list.erase(iter);
			iter=next++;
			span=atof(iter->c_str());
			VERBOSE_OUT(1)<<strprintf(_("Span set to %d units"),span)<<endl;
			arg_list.erase(iter);
		}
		else if(*iter=="--fps")
		{
			arg_list.erase(iter);
			iter=next++;
			float fps=atof(iter->c_str());
			desc.set_frame_rate(fps);
			arg_list.erase(iter);
			VERBOSE_OUT(1)<<strprintf(_("Frame rate set to %d frames per second"),fps)<<endl;
		}
		else if(*iter=="--dpi")
		{
			arg_list.erase(iter);
			iter=next++;
			float dpi=atof(iter->c_str());
			float dots_per_meter=dpi*39.3700787402;
			desc.set_x_res(dots_per_meter).set_y_res(dots_per_meter);
			arg_list.erase(iter);
			VERBOSE_OUT(1)<<strprintf(_("Physical resolution set to %f dpi"),dpi)<<endl;
		}
		else if(*iter=="--dpi-x")
		{
			arg_list.erase(iter);
			iter=next++;
			float dpi=atof(iter->c_str());
			float dots_per_meter=dpi*39.3700787402;
			desc.set_x_res(dots_per_meter);
			arg_list.erase(iter);
			VERBOSE_OUT(1)<<strprintf(_("Physical X resolution set to %f dpi"),dpi)<<endl;
		}
		else if(*iter=="--dpi-y")
		{
			arg_list.erase(iter);
			iter=next++;
			float dpi=atof(iter->c_str());
			float dots_per_meter=dpi*39.3700787402;
			desc.set_y_res(dots_per_meter);
			arg_list.erase(iter);
			VERBOSE_OUT(1)<<strprintf(_("Physical Y resolution set to %f dpi"),dpi)<<endl;
		}
		else if(*iter=="--start-time" || *iter=="--begin-time")
		{
			arg_list.erase(iter);
			iter=next++;
			desc.set_time_start(Time(*iter,desc.get_frame_rate()));
			arg_list.erase(iter);
		}
		else if(*iter=="--end-time")
		{
			arg_list.erase(iter);
			iter=next++;
			desc.set_time_end(Time(*iter,desc.get_frame_rate()));
			arg_list.erase(iter);
		}
		else if(*iter=="--time")
		{
			arg_list.erase(iter);
			iter=next++;
			desc.set_time(Time(*iter,desc.get_frame_rate()));
			VERBOSE_OUT(1)<<_("Rendering frame at ")<<desc.get_time_start().get_string(desc.get_frame_rate())<<endl;
			arg_list.erase(iter);
		}
		else if(*iter=="-g")
		{
			synfig::warning("Gamma argument is currently ignored");
			arg_list.erase(iter);
			iter=next++;
			//desc.set_gamma(Gamma(atof(iter->c_str())));
			arg_list.erase(iter);
		}
		else if (flag_requires_value(*iter))
			iter=next++;
	}
	if(w&&h)
	{
		desc.set_wh(w,h);
		VERBOSE_OUT(1)<<strprintf(_("Resolution set to %dx%d"),w,h)<<endl;
	}
	else
	{
		if(w)
		{
			VERBOSE_OUT(1)<<strprintf(_("Width set to %d pixels"),w)<<endl;
			desc.set_w(w);
		}
		if(h)
		{
			VERBOSE_OUT(1)<<strprintf(_("Height set to %d pixels"),h)<<endl;
			desc.set_h(h);
		}
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
			arg_list.erase(iter);
			iter=next++;
			quality=atoi(iter->c_str());
			VERBOSE_OUT(1)<<strprintf(_("Quality set to %d"),quality)<<endl;
			arg_list.erase(iter);
		}
		else if (flag_requires_value(*iter))
			iter=next++;
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
			arg_list.erase(iter);
			iter=next++;
			threads=atoi(iter->c_str());
			VERBOSE_OUT(1)<<strprintf(_("Threads set to %d"),threads)<<endl;
			arg_list.erase(iter);
		}
		else if (flag_requires_value(*iter))
			iter=next++;
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
			arg_list.erase(iter);
			iter=next++;
			type=*iter;
			arg_list.erase(iter);
		}
		else if (flag_requires_value(*iter))
			iter=next++;
	}

	return SYNFIGTOOL_OK;
}

int extract_append(arg_list_t &arg_list,string &filename)
{
	arg_list_t::iterator iter, next;
	filename.clear();

	for(next=arg_list.begin(),iter=next++;iter!=arg_list.end();iter=next++)
	{
		if(*iter=="--append")
		{
			arg_list.erase(iter);
			iter=next++;
			filename=*iter;
			arg_list.erase(iter);
		}
		else if (flag_requires_value(*iter))
			iter=next++;
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
			arg_list.erase(iter);
			iter=next++;
			outfile=*iter;
			arg_list.erase(iter);
			ret=SYNFIGTOOL_OK;
		}
		else if (flag_requires_value(*iter))
			iter=next++;
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
			arg_list.erase(iter);
			iter=next++;
			canvasid=*iter;
			arg_list.erase(iter);
		}
		else if (flag_requires_value(*iter))
			iter=next++;
	}

	return SYNFIGTOOL_OK;
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
		display_help(0);
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
			job_list.front().root=open_canvas(job_list.front().filename);

			if(!job_list.front().root)
			{
				cerr<<_("Unable to open ")<<job_list.front().filename<<"."<<endl;
				cerr<<_("Throwing out job...")<<endl;
				job_list.pop_front();
				continue;
			}

			job_list.front().root->set_time(0);

			string canvasid;
			extract_canvasid(imageargs,canvasid);
			if(!canvasid.empty())
			{
				try
				{
					job_list.front().canvas=job_list.front().root->find_canvas(canvasid);
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
					Canvas::Handle composite(open_canvas(composite_file));
					if(!composite)
						break;
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
					job_list.front().outfilename+=Target::book()[target_name].second;
				else
					job_list.front().outfilename+=target_name;
			}

			VERBOSE_OUT(4)<<"target_name="<<target_name<<endl;
			VERBOSE_OUT(4)<<"outfile_name="<<job_list.front().outfilename<<endl;

			VERBOSE_OUT(4)<<_("Creating the target...")<<endl;
			job_list.front().target=synfig::Target::create(target_name,job_list.front().outfilename);

			if(target_name=="sif")
			{
				job_list.front().sifout=true;
			}
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
			{
				Target_Scanline::Handle::cast_dynamic(job_list.front().target)->set_threads(threads);
			}

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
		if(!job_list.front().sifout)
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
			{
				cout<<job_list.front().filename+": Rendered in "<<timer()<<" seconds."<<endl;
			}
		}
		else
		{
			if(!save_canvas(job_list.front().outfilename,job_list.front().canvas))
			{
				cerr<<"Render Failure."<<endl;
				return SYNFIGTOOL_RENDERFAILURE;
			}
		}
	}

	job_list.clear();

	VERBOSE_OUT(1)<<_("Done.")<<endl;

	return SYNFIGTOOL_OK;
}
