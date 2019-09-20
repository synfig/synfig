/* === S Y N F I G ========================================================= */
/*!	\file synfig/main.cpp
**	\brief \writeme
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2013 Konstantin Dmitriev
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

#include <synfig/localization.h>
#include <synfig/general.h>

#include <iostream>
#include "version.h"
#include "general.h"
#include "module.h"
#include <cstdlib>
#include <ltdl.h>
#include <glibmm.h>
#include <stdexcept>

// Includes used by get_binary_path():
#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <sys/param.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "token.h"
#include "target.h"
#include <ETL/stringf>
#include "cairolistimporter.h"
#include "listimporter.h"
#include "cairoimporter.h"
#include "color.h"
#include "vector.h"
#include <fstream>
#include <time.h>
#include "layer.h"
#include "valuenode.h"
#include "soundprocessor.h"
#include "rendering/renderer.h"

#include "main.h"
#include "loadcanvas.h"

#include "guid.h"

#include "mutex.h"

#include <giomm.h>

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

#define MODULE_LIST_FILENAME	"synfig_modules.cfg"

/* === S T A T I C S ======================================================= */

static etl::reference_counter synfig_ref_count_(0);
Main *Main::instance = NULL;

class GeneralIOMutexHolder {
private:
	Mutex mutex;
	bool initialized;
public:
	GeneralIOMutexHolder(): initialized(true) { }
	~GeneralIOMutexHolder() { initialized = false; }
	void lock() { if (initialized) mutex.lock(); }
	void unlock() { if (initialized) mutex.unlock(); }
};

GeneralIOMutexHolder general_io_mutex;

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

const char *
synfig::get_version()
{
#ifdef VERSION
	return VERSION;
#else
	return "Unknown";
#endif
}

const char *
synfig::get_build_date()
{
	return __DATE__;
}

bool
synfig::check_version_(size_t version, size_t vec_size, size_t color_size, size_t canvas_size, size_t layer_size)
{
	bool ret=true;

	if(version!=SYNFIG_LIBRARY_VERSION)
	{
		synfig::error(_("API Version mismatch (LIB:%zu, PROG:%zu)"), SYNFIG_LIBRARY_VERSION, version);
		ret=false;
	}
	if(vec_size!=sizeof(Vector))
	{
		synfig::error(_("Size of Vector mismatch (app:%zu, lib:%zu)"),vec_size,sizeof(Vector));
		ret=false;
	}
	if(color_size!=sizeof(Color))
	{
		synfig::error(_("Size of Color mismatch (app:%zu, lib:%zu)"),color_size,sizeof(Color));
		ret=false;
	}
	if(canvas_size!=sizeof(Canvas))
	{
		synfig::error(_("Size of Canvas mismatch (app:%zu, lib:%zu)"),canvas_size,sizeof(Canvas));
		ret=false;
	}
	if(layer_size!=sizeof(Layer))
	{
		synfig::error(_("Size of Layer mismatch (app:%zu, lib:%zu)"),layer_size,sizeof(Layer));
		ret=false;
	}

	return ret;
}

static void broken_pipe_signal (int /*sig*/)  {
	synfig::warning("Broken Pipe...");
}

bool retrieve_modules_to_load(String filename,std::list<String> &modules_to_load)
{
	std::ifstream file(Glib::locale_from_utf8(filename).c_str());

	if(!file)
	{
		synfig::warning("Cannot open "+filename);
		return false;
	}

	while(file)
	{
		String modulename;
		getline(file,modulename);
		if(!modulename.empty() && find(modules_to_load.begin(),modules_to_load.end(),modulename)==modules_to_load.end())
			modules_to_load.push_back(modulename);
	}

	return true;
}

synfig::Main::Main(const synfig::String& basepath,ProgressCallback *cb):
	ref_count_(synfig_ref_count_)
{
	if(ref_count_.count())
		return;

	synfig_ref_count_.reset();
	ref_count_=synfig_ref_count_;

	assert(!instance);
	instance = this;

	// Paths

	root_path       = etl::dirname(basepath);
	bin_path        = root_path  + ETL_DIRECTORY_SEPARATOR + "bin";
	share_path      = root_path  + ETL_DIRECTORY_SEPARATOR + "share";
	locale_path     = share_path + ETL_DIRECTORY_SEPARATOR + "locale";
	lib_path        = root_path  + ETL_DIRECTORY_SEPARATOR + "lib";
	lib_synfig_path = lib_path   + ETL_DIRECTORY_SEPARATOR + "synfig";

	// Add initialization after this point

#ifdef ENABLE_NLS
	String locale_dir;
	locale_dir = locale_path;

	bindtextdomain("synfig", Glib::locale_from_utf8(locale_path).c_str() );
	bind_textdomain_codeset("synfig", "UTF-8");
#endif

	unsigned int i;
#ifdef _DEBUG
#ifndef __APPLE__
	std::set_terminate(__gnu_cxx::__verbose_terminate_handler);
#endif
#endif

#if defined(HAVE_SIGNAL_H) && defined(SIGPIPE)
	signal(SIGPIPE, broken_pipe_signal);
#endif

	//_config_search_path=new vector"string.h"();
	
	// Init Gio to allow use Gio::File::create_for_uri() in ValueNode_AnimatedFile::load file() function
	Gio::init();

	// Init the subsystems
	if(cb)cb->amount_complete(0, 100);

	if(cb)cb->task(_("Starting Subsystem \"Sound\""));
	if(!SoundProcessor::subsys_init())
		throw std::runtime_error(_("Unable to initialize subsystem \"Sound\""));

	if(cb)cb->task(_("Starting Subsystem \"Types\""));
	if(!Type::subsys_init())
	{
		SoundProcessor::subsys_stop();
		throw std::runtime_error(_("Unable to initialize subsystem \"Types\""));
	}

	if(cb)cb->task(_("Starting Subsystem \"Rendering\""));
	if(!rendering::Renderer::subsys_init())
	{
		Type::subsys_stop();
		SoundProcessor::subsys_stop();
		throw std::runtime_error(_("Unable to initialize subsystem \"Rendering\""));
	}

	if(cb)cb->task(_("Starting Subsystem \"Modules\""));
	if(!Module::subsys_init(root_path))
	{
		rendering::Renderer::subsys_stop();
		Type::subsys_stop();
		SoundProcessor::subsys_stop();
		throw std::runtime_error(_("Unable to initialize subsystem \"Modules\""));
	}

	if(cb)cb->task(_("Starting Subsystem \"Layers\""));
	if(!Layer::subsys_init())
	{
		Module::subsys_stop();
		rendering::Renderer::subsys_stop();
		Type::subsys_stop();
		SoundProcessor::subsys_stop();
		throw std::runtime_error(_("Unable to initialize subsystem \"Layers\""));
	}

	if(cb)cb->task(_("Starting Subsystem \"Targets\""));
	if(!Target::subsys_init())
	{
		Layer::subsys_stop();
		Module::subsys_stop();
		rendering::Renderer::subsys_stop();
		Type::subsys_stop();
		SoundProcessor::subsys_stop();
		throw std::runtime_error(_("Unable to initialize subsystem \"Targets\""));
	}

	if(cb)cb->task(_("Starting Subsystem \"Importers\""));
	if(!Importer::subsys_init())
	{
		Target::subsys_stop();
		Layer::subsys_stop();
		Module::subsys_stop();
		rendering::Renderer::subsys_stop();
		Type::subsys_stop();
		SoundProcessor::subsys_stop();
		throw std::runtime_error(_("Unable to initialize subsystem \"Importers\""));
	}

	if(cb)cb->task(_("Starting Subsystem \"Cairo Importers\""));
	if(!CairoImporter::subsys_init())
	{
		Importer::subsys_stop();
		Target::subsys_stop();
		Layer::subsys_stop();
		Module::subsys_stop();
		rendering::Renderer::subsys_stop();
		Type::subsys_stop();
		SoundProcessor::subsys_stop();
		throw std::runtime_error(_("Unable to initialize subsystem \"Cairo Importers\""));
	}

	// Rebuild tokens data
	Token::rebuild();

	// Load up the list importer
	Importer::book()[String("lst")]=Importer::BookEntry(ListImporter::create, ListImporter::supports_file_system_wrapper__);
	CairoImporter::book()[String("lst")]=CairoImporter::BookEntry(CairoListImporter::create, CairoListImporter::supports_file_system_wrapper__);

	// Load up the modules
	std::list<String> modules_to_load;
	std::vector<String> locations;

	if(getenv("SYNFIG_MODULE_LIST"))
		locations.push_back(getenv("SYNFIG_MODULE_LIST"));
	else
	{
		locations.push_back("./" MODULE_LIST_FILENAME);
		if(getenv("HOME"))
			locations.push_back(strprintf("%s/.local/share/synfig/%s", getenv("HOME"), MODULE_LIST_FILENAME));
	#ifdef SYSCONFDIR
		locations.push_back(SYSCONFDIR"/" MODULE_LIST_FILENAME);
	#endif
		locations.push_back(root_path + ETL_DIRECTORY_SEPARATOR + "etc" + ETL_DIRECTORY_SEPARATOR + MODULE_LIST_FILENAME);
		locations.push_back("/usr/local/etc/" MODULE_LIST_FILENAME);
	#ifdef __APPLE__
		locations.push_back("/Library/Frameworks/synfig.framework/Resources/" MODULE_LIST_FILENAME);
		locations.push_back("/Library/Synfig/" MODULE_LIST_FILENAME);
		if(getenv("HOME"))
			locations.push_back(strprintf("%s/Library/Synfig/%s", getenv("HOME"), MODULE_LIST_FILENAME));
	#endif
	}

	for(i=0;i<locations.size();i++)
		if(retrieve_modules_to_load(locations[i],modules_to_load))
		{
			synfig::info(_("Loading modules from %s"), Glib::locale_from_utf8(locations[i]).c_str());
			if(cb)cb->task(strprintf(_("Loading modules from %s"),locations[i].c_str()));
			break;
		}

	if (i == locations.size())
	{
		synfig::warning("Cannot find '%s', trying to load default modules", MODULE_LIST_FILENAME);
		Module::register_default_modules(cb);
	}

	std::list<String>::iterator iter;

	for(i=0,iter=modules_to_load.begin();iter!=modules_to_load.end();++iter,i++)
	{
		synfig::info("Loading %s..", iter->c_str());
		Module::Register(*iter,cb);
		if(cb)cb->amount_complete((i+1)*100,modules_to_load.size()*100);
	}

	// Rebuild tokens data again to include new tokens from modules
	Token::rebuild();

	if(cb)cb->amount_complete(100, 100);
	if(cb)cb->task(_("DONE"));
}

synfig::Main::~Main()
{
	ref_count_.detach();
	if(!synfig_ref_count_.unique())
		return;
	synfig_ref_count_.detach();

	// Add deinitialization after this point

	if(get_open_canvas_map().size())
	{
		synfig::warning("Canvases still open!");
		std::map<synfig::String, etl::loose_handle<Canvas> >::iterator iter;
		for(iter=get_open_canvas_map().begin();iter!=get_open_canvas_map().end();++iter)
		{
			synfig::warning("%s: count()=%d",iter->first.c_str(), iter->second.count());
		}
	}

	// synfig::info("Importer::subsys_stop()");
	Importer::subsys_stop();
	CairoImporter::subsys_stop();
	// synfig::info("Target::subsys_stop()");
	Target::subsys_stop();
	// synfig::info("Layer::subsys_stop()");
	Layer::subsys_stop();
	/*! \todo For some reason, uncommenting the next line will cause things to crash.
			  This needs to be looked into at some point. */
 	// synfig::info("Module::subsys_stop()");
	// Module::subsys_stop();
	// synfig::info("Exiting");
	rendering::Renderer::subsys_stop();
	Type::subsys_stop();
	SoundProcessor::subsys_stop();

#if defined(HAVE_SIGNAL_H) && defined(SIGPIPE)
	signal(SIGPIPE, SIG_DFL);
#endif

	assert(instance);
	instance = NULL;
}

static const String
current_time()
{
	const int buflen = 50;
	time_t t;
	struct tm *lt;
	char b[buflen];
	time(&t);
	lt = localtime(&t);
	strftime(b, buflen, " [%X] ", lt);
	return String(b);
}

bool synfig::synfig_quiet_mode = false;

void
synfig::error(const char *format,...)
{
	va_list args;
	va_start(args, format);
	error(vstrprintf(format, args));
	va_end(args);
}

void
synfig::error(const String &str)
{
	general_io_mutex.lock();
	cerr<<"\033[31m"<<"synfig("<<getpid()<<")"<<current_time().c_str()<<_("error")<<": "<<str.c_str()<<"\033[0m"<<endl;
	general_io_mutex.unlock();
}

void
synfig::warning(const char *format,...)
{
	va_list args;
	va_start(args,format);
	warning(vstrprintf(format,args));
	va_end(args);
}

void
synfig::warning(const String &str)
{
	general_io_mutex.lock();
	cerr<<"\033[33m"<<"synfig("<<getpid()<<")"<<current_time().c_str()<<_("warning")<<": "<<str.c_str()<<"\033[0m"<<endl;
	general_io_mutex.unlock();
}

void
synfig::info(const char *format,...)
{
	va_list args;
	va_start(args, format);
	info(vstrprintf(format, args));
	va_end(args);
}

void
synfig::info(const String &str)
{
	//if (SynfigToolGeneralOptions::instance()->should_be_quiet()) return; // don't show info messages in quiet mode
	if (synfig::synfig_quiet_mode) return;

	general_io_mutex.lock();
	cout<<"synfig("<<getpid()<<")"<<current_time().c_str()<<_("info")<<": "<<str.c_str()<<endl;
	general_io_mutex.unlock();
}

// synfig::get_binary_path()
// See also: http://libsylph.sourceforge.net/wiki/Full_path_to_binary

String
synfig::get_binary_path(const String &fallback_path)
{
	
	String result;

#ifdef _WIN32
	
	size_t buf_size = PATH_MAX - 1;
	char* path = (char*)malloc(buf_size);
	
	GetModuleFileName(NULL, path, PATH_MAX);

	result = String(path);
	
	free(path);

#elif defined(__APPLE__)
	
	uint32_t buf_size = MAXPATHLEN;
	char* path = (char*)malloc(MAXPATHLEN);
	
	if(_NSGetExecutablePath(path, &buf_size) == -1 ) {
		path = (char*)realloc(path, buf_size);
		_NSGetExecutablePath(path, &buf_size);
	}
	
	result = String(path);
	
	free(path);
	
	// "./synfig" case workaround
	String artifact("/./");
	size_t start_pos = result.find(artifact);
	if (start_pos != std::string::npos)
		result.replace(start_pos, artifact.length(), "/");
	
#elif !defined(__OpenBSD__)

	size_t buf_size = PATH_MAX - 1;
	char* path = (char*)malloc(buf_size);

	ssize_t size;
	struct stat stat_buf;
	FILE *f;

	/* Read from /proc/self/exe (symlink) */
	//char* path2 = (char*)malloc(buf_size);
	char* path2 = new char[buf_size];
	strncpy(path2, "/proc/self/exe", buf_size - 1);

	while (1) {
		int i;

		size = readlink(path2, path, buf_size - 1);
		if (size == -1) {
			/* Error. */
			break;
		}

		/* readlink() success. */
		path[size] = '\0';

		/* Check whether the symlink's target is also a symlink.
		 * We want to get the final target. */
		i = stat(path, &stat_buf);
		if (i == -1) {
			/* Error. */
			break;
		}

		/* stat() success. */
		if (!S_ISLNK(stat_buf.st_mode)) {

			/* path is not a symlink. Done. */
			result = String(path);
			
			break;
		}

		/* path is a symlink. Continue loop and resolve this. */
		strncpy(path, path2, buf_size - 1);
	}
	
	//free(path2);
	delete[] path2;

	if (result == "")
	{
		/* readlink() or stat() failed; this can happen when the program is
		 * running in Valgrind 2.2. Read from /proc/self/maps as fallback. */

		buf_size = PATH_MAX + 128;
		char* line = (char*)malloc(buf_size);

		f = fopen("/proc/self/maps", "r");
		if (f == NULL) {
			synfig::error("Cannot open /proc/self/maps.");
		}

		/* The first entry should be the executable name. */
		char *r;
		r = fgets(line, (int) buf_size, f);
		if (r == NULL) {
			synfig::error("Cannot read /proc/self/maps.");
		}

		/* Get rid of newline character. */
		buf_size = strlen(line);
		if (buf_size <= 0) {
			/* Huh? An empty string? */
			synfig::error("Invalid /proc/self/maps.");
		}
		if (line[buf_size - 1] == 10)
			line[buf_size - 1] = 0;

		/* Extract the filename; it is always an absolute path. */
		path = strchr(line, '/');

		/* Sanity check. */
		if (strstr(line, " r-xp ") == NULL || path == NULL) {
			synfig::error("Invalid /proc/self/maps.");
		}

		result = String(path);
		free(line);
		fclose(f);
	}
	
	free(path);

#endif
	
	if (result == "")
	{
		// In worst case use value specified as fallback 
		// (usually should come from argv[0])
		result = etl::absolute_path(fallback_path);
	}
	
	result = Glib::locale_to_utf8(result);
	
	return result;
}
