/* === S I N F G =========================================================== */
/*!	\file main.cpp
**	\brief \writeme
**
**	$Id: main.cpp,v 1.3 2005/01/10 07:40:26 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

//#define SINFG_NO_ANGLE

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <iostream>
#include "version.h"
#include "general.h"
#include "module.h"
#include <cstdlib>
#include <ltdl.h>
#include <stdexcept>
#include "target.h"
#include <ETL/stringf>
#include "listimporter.h"
#include "color.h"
#include "vector.h"
#include <fstream>
#include "layer.h"
#include "valuenode.h"

#include "main.h"
#include "loadcanvas.h"

#include "guid.h"

#include "mutex.h"

#ifdef DEATH_TIME
#include <time.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#endif

using namespace std;
using namespace etl;
using namespace sinfg;

/* === M A C R O S ========================================================= */

#define MODULE_LIST_FILENAME	"sinfg_modules.cfg"

/* === S T A T I C S ======================================================= */

static etl::reference_counter sinfg_ref_count_(0);

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */








const char *
sinfg::get_version()
{
#ifdef VERSION
	return VERSION;
#else
	return "Unknown";
#endif
}

const char *
sinfg::get_build_date()
{
	return __DATE__;
}

const char *
sinfg::get_build_time()
{
	return __TIME__;
}

extern const char *get_build_time();

bool
sinfg::check_version_(int version,int vec_size, int color_size,int canvas_size,int layer_size)
{
	bool ret=true;

	CHECK_EXPIRE_TIME();
	
	if(version!=SINFG_LIBRARY_VERSION)
	{
		sinfg::error(_("API Version mismatch (LIB:%d, PROG:%d)"),SINFG_LIBRARY_VERSION,version);
		ret=false;
	}
	if(vec_size!=sizeof(Vector))
	{
		sinfg::error(_("Size of Vector mismatch (app:%d, lib:%d)"),vec_size,sizeof(Vector));
		ret=false;
	}
	if(color_size!=sizeof(Color))
	{
		sinfg::error(_("Size of Color mismatch (app:%d, lib:%d)"),color_size,sizeof(Color));
		ret=false;
	}
	if(canvas_size!=sizeof(Canvas))
	{
		sinfg::error(_("Size of Canvas mismatch (app:%d, lib:%d)"),canvas_size,sizeof(Canvas));
		ret=false;
	}
	if(layer_size!=sizeof(Layer))
	{
		sinfg::error(_("Size of Layer mismatch (app:%d, lib:%d)"),layer_size,sizeof(Layer));
		ret=false;
	}
	
	return ret;
}

static void broken_pipe_signal (int sig)  {
	sinfg::warning("Broken Pipe...");
}

bool retrieve_modules_to_load(String filename,std::list<String> &modules_to_load)
{
	if(filename=="standard")
	{
		return false;
/*
		if(find(modules_to_load.begin(),modules_to_load.end(),"trgt_bmp")==modules_to_load.end())
			modules_to_load.push_back("trgt_bmp");
		if(find(modules_to_load.begin(),modules_to_load.end(),"trgt_gif")==modules_to_load.end())
			modules_to_load.push_back("trgt_gif");
		if(find(modules_to_load.begin(),modules_to_load.end(),"trgt_dv")==modules_to_load.end())
			modules_to_load.push_back("trgt_dv");
		if(find(modules_to_load.begin(),modules_to_load.end(),"mod_ffmpeg")==modules_to_load.end())
			modules_to_load.push_back("mod_ffmpeg");
		if(find(modules_to_load.begin(),modules_to_load.end(),"mod_imagemagick")==modules_to_load.end())
			modules_to_load.push_back("mod_imagemagick");
		if(find(modules_to_load.begin(),modules_to_load.end(),"lyr_std")==modules_to_load.end())
			modules_to_load.push_back("lyr_std");
		if(find(modules_to_load.begin(),modules_to_load.end(),"lyr_freetype")==modules_to_load.end())
			modules_to_load.push_back("lyr_freetype");
#ifdef HAVE_LIBPNG
		if(find(modules_to_load.begin(),modules_to_load.end(),"trgt_png")==modules_to_load.end())
			modules_to_load.push_back("trgt_png");
#endif
#ifdef HAVE_OPENEXR
		if(find(modules_to_load.begin(),modules_to_load.end(),"mod_openexr")==modules_to_load.end())
			modules_to_load.push_back("mod_openexr");
#endif
*/
	}
	else
	{
		std::ifstream file(filename.c_str());
		if(!file)
		{
		//	warning("Cannot open "+filename);
			return false;
		}
		while(file)
		{
			String modulename;
			getline(file,modulename);
			if(!modulename.empty() && find(modules_to_load.begin(),modules_to_load.end(),modulename)==modules_to_load.end())
				modules_to_load.push_back(modulename);			
		}
	}

	
	
	return true;
}





sinfg::Main::Main(const sinfg::String& basepath,ProgressCallback *cb):
	ref_count_(sinfg_ref_count_)
{
	if(ref_count_.count())
		return;

	sinfg_ref_count_.reset();
	ref_count_=sinfg_ref_count_;
	
	// Add initialization after this point


	CHECK_EXPIRE_TIME();

	String prefix=basepath+"/..";
	int i;
#ifdef _DEBUG
	std::set_terminate(__gnu_cxx::__verbose_terminate_handler);
#endif
	
#if defined(HAVE_SIGNAL_H) && defined(SIGPIPE)
	signal(SIGPIPE, broken_pipe_signal);
#endif
	
	//_config_search_path=new vector"string.h"();
	
	// Init the subsystems
	if(cb)cb->amount_complete(0, 100);
	if(cb)cb->task(_("Starting Subsystem \"Modules\""));
	if(!Module::subsys_init(prefix))
		throw std::runtime_error(_("Unable to initialize subsystem \"Module\""));

	if(cb)cb->task(_("Starting Subsystem \"Layers\""));
	if(!Layer::subsys_init())
	{
		Module::subsys_stop();
		throw std::runtime_error(_("Unable to initialize subsystem \"Layers\""));
	}

	if(cb)cb->task(_("Starting Subsystem \"Targets\""));
	if(!Target::subsys_init())
	{
		Layer::subsys_stop();
		Module::subsys_stop();
		throw std::runtime_error(_("Unable to initialize subsystem \"Targets\""));
	}
	
	if(cb)cb->task(_("Starting Subsystem \"Importers\""));
	if(!Importer::subsys_init())
	{
		Target::subsys_stop();
		Layer::subsys_stop();
		Module::subsys_stop();
		throw std::runtime_error(_("Unable to initialize subsystem \"Importers\""));
	}

	if(cb)cb->task(_("Starting Subsystem \"ValueNodes\""));
	if(!ValueNode::subsys_init())
	{
		Importer::subsys_stop();
		Target::subsys_stop();
		Layer::subsys_stop();
		Module::subsys_stop();
		throw std::runtime_error(_("Unable to initialize subsystem \"ValueNodes\""));
	}
	
	// Load up the list importer
	Importer::book()[String("lst")]=ListImporter::create;
		

	
	// Load up the modules	
	std::list<String> modules_to_load;
	const char *locations[]=
	{
		"standard",	//0
		"./"MODULE_LIST_FILENAME,	//1
		"../etc/"MODULE_LIST_FILENAME,	//1
		"~/.sinfg/"MODULE_LIST_FILENAME, //2
		"/usr/local/lib/sinfg/modules/"MODULE_LIST_FILENAME, //3
		"/usr/local/etc/"MODULE_LIST_FILENAME,
#ifdef SYSCONFDIR
		SYSCONFDIR"/"MODULE_LIST_FILENAME,
#endif
#ifdef __APPLE__
		"/Library/Frameworks/sinfg.framework/Resources/"MODULE_LIST_FILENAME,
		"/Library/SINFG/"MODULE_LIST_FILENAME,
		"~/Library/SINFG/"MODULE_LIST_FILENAME,
#endif
#ifdef WIN32
		"C:\\Program Files\\SINFG\\etc\\"MODULE_LIST_FILENAME,
#endif
	};
	String module_path=prefix+"/etc/"+MODULE_LIST_FILENAME;
	locations[3]=module_path.c_str();
	
	for(i=0;i<(signed)(sizeof(locations)/sizeof(char*));i++)
		if(retrieve_modules_to_load(locations[i],modules_to_load))
			if(cb)cb->task(strprintf(_("Loading modules from %s"),locations[i]));
	
	std::list<String>::iterator iter;
	
	for(i=0,iter=modules_to_load.begin();iter!=modules_to_load.end();++iter,i++)
	{
		Module::Register(*iter,cb);
		if(cb)cb->amount_complete((i+1)*100,modules_to_load.size()*100);
	}
	
//	load_modules(cb);
	
	CHECK_EXPIRE_TIME();

	
	if(cb)cb->amount_complete(100, 100);
	if(cb)cb->task(_("DONE"));
}

sinfg::Main::~Main()
{
	ref_count_.detach();
	if(!sinfg_ref_count_.unique())
		return;
	sinfg_ref_count_.detach();

	// Add deinitialization after this point

	if(get_open_canvas_map().size())
	{
		sinfg::warning("Canvases still open!");
		std::map<sinfg::String, etl::loose_handle<Canvas> >::iterator iter;
		for(iter=get_open_canvas_map().begin();iter!=get_open_canvas_map().end();++iter)
		{
			sinfg::warning("%s: count()=%d",iter->first.c_str(), iter->second.count());
		}
	}
		
	ValueNode::subsys_stop();
	Importer::subsys_stop();
	Target::subsys_stop();
	Layer::subsys_stop();

	/*! \fixme For some reason, uncommenting the next
	**	line will cause things to crash. This needs to be
	**	looked into at some point. */
	//Module::subsys_stop();

#if defined(HAVE_SIGNAL_H) && defined(SIGPIPE)
	signal(SIGPIPE, SIG_DFL);
#endif
}









void
sinfg::error(const char *format,...)
{
	va_list args;
	va_start(args,format);
	error(vstrprintf(format,args));
}

void
sinfg::error(const String &str)
{
	static Mutex mutex; Mutex::Lock lock(mutex);
	cerr<<"sinfg("<<getpid()<<"): "<<_("error")<<": "+str<<endl;
}

void
sinfg::warning(const char *format,...)
{
	va_list args;
	va_start(args,format);
	warning(vstrprintf(format,args));
}

void
sinfg::warning(const String &str)
{
	static Mutex mutex; Mutex::Lock lock(mutex);
	cerr<<"sinfg("<<getpid()<<"): "<<_("warning")<<": "+str<<endl;
}

void
sinfg::info(const char *format,...)
{
	va_list args;
	va_start(args,format);
	info(vstrprintf(format,args));
}

void
sinfg::info(const String &str)
{
	static Mutex mutex; Mutex::Lock lock(mutex);
	cerr<<"sinfg("<<getpid()<<"): "<<_("info")<<": "+str<<endl;
}
