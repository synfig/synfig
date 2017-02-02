/* === S Y N F I G ========================================================= */
/*!	\file synfig/main.cpp
**	\brief Synfig library initialization and helper functions
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2013 Konstantin Dmitriev
**	Copyright (c) 2017 caryoscelus
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

#include <boost/filesystem.hpp>

#include <synfig/general.h>

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

#include <synfig/localization.h>

#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

using namespace std;
using namespace etl;
using namespace synfig;
namespace fs = boost::filesystem;

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
synfig::check_version_(int version,int vec_size, int color_size,int canvas_size,int layer_size)
{
	bool ret=true;

	if(version!=SYNFIG_LIBRARY_VERSION)
	{
		synfig::error(_("API Version mismatch (LIB:%d, PROG:%d)"),SYNFIG_LIBRARY_VERSION,version);
		ret=false;
	}
	if(vec_size!=sizeof(Vector))
	{
		synfig::error(_("Size of Vector mismatch (app:%d, lib:%d)"),vec_size,sizeof(Vector));
		ret=false;
	}
	if(color_size!=sizeof(Color))
	{
		synfig::error(_("Size of Color mismatch (app:%d, lib:%d)"),color_size,sizeof(Color));
		ret=false;
	}
	if(canvas_size!=sizeof(Canvas))
	{
		synfig::error(_("Size of Canvas mismatch (app:%d, lib:%d)"),canvas_size,sizeof(Canvas));
		ret=false;
	}
	if(layer_size!=sizeof(Layer))
	{
		synfig::error(_("Size of Layer mismatch (app:%d, lib:%d)"),layer_size,sizeof(Layer));
		ret=false;
	}

	return ret;
}

static void broken_pipe_signal (int /*sig*/)  {
	synfig::warning("Broken Pipe...");
}

bool retrieve_modules_to_load(fs::path filename, std::set<std::string> &modules_to_load)
{
	fs::ifstream file(filename);

	if(!file)
	{
		synfig::warning("Cannot open "+filename.native());
		return false;
	}

	while(file)
	{
		String modulename;
		getline(file,modulename);
		if(!modulename.empty() && find(modules_to_load.begin(),modules_to_load.end(),modulename)==modules_to_load.end())
			modules_to_load.insert(modulename);
	}

	return true;
}

void load_modules(fs::path root_path, ProgressCallback *cb)
{
	std::vector<fs::path> locations;
	auto home_dir = fs::path(getenv("HOME"));

	if (getenv("SYNFIG_CONFIG_DIR"))
		locations.emplace_back(getenv("SYNFIG_CONFIG_DIR"));
	locations.emplace_back(".");
	if (!home_dir.empty())
		locations.push_back(home_dir / ".local/share/synfig");
#ifdef SYSCONFDIR
	// Is this still used anywhere by anybody?
	locations.emplace_back(SYSCONFDIR);
#endif
	locations.push_back(root_path / "etc/synfig");
	// TODO: remove this when no longer required
	locations.push_back(root_path / "etc");
#ifdef __APPLE__
	// TODO: check if this is correct
	locations.emplace_back("/Library/Frameworks/synfig.framework/Resources/");
	locations.emplace_back("/Library/Synfig/");
	if (!home_dir.empty())
		locations.push_back(home_dir / "Library/Synfig/");
#endif
	auto found_location = std::find_if(
		std::begin(locations),
		std::end(locations),
		[](fs::path path) -> bool {
			auto modules_cfg = path / MODULE_LIST_FILENAME;
			return fs::exists(modules_cfg);
		}
	);
	synfig::info((*found_location).native());
	if (found_location == std::end(locations))
	{
		synfig::warning("Cannot find '%s', trying to load default modules", MODULE_LIST_FILENAME);
		Module::register_default_modules(cb);
	}
	else
	{
		auto config_dir = *found_location;
		std::set<std::string> modules_to_load;
		retrieve_modules_to_load(config_dir / MODULE_LIST_FILENAME, modules_to_load);
		auto module_d = config_dir / (MODULE_LIST_FILENAME ".d");
		if (fs::is_directory(module_d))
		{
			for (auto const& file : fs::directory_iterator(module_d))
				retrieve_modules_to_load(file.path(), modules_to_load);
		}
		// TODO: load asynchronously
		for (auto const& module : modules_to_load)
		{
			Module::Register(module, cb);
		}
	}
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

	// Load up the list importer
	Importer::book()[String("lst")]=Importer::BookEntry(ListImporter::create, ListImporter::supports_file_system_wrapper__);
	CairoImporter::book()[String("lst")]=CairoImporter::BookEntry(CairoListImporter::create, CairoListImporter::supports_file_system_wrapper__);

	// Load up the modules
	load_modules(root_path, cb);

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

void
synfig::error(const char *format,...)
{
	va_list args;
	va_start(args,format);
	error(vstrprintf(format,args));
}

void
synfig::error(const String &str)
{
	general_io_mutex.lock();
	cerr<<"synfig("<<getpid()<<")"<<current_time().c_str()<<_("error")<<": "<<str.c_str()<<endl;
	general_io_mutex.unlock();
}

void
synfig::warning(const char *format,...)
{
	va_list args;
	va_start(args,format);
	warning(vstrprintf(format,args));
}

void
synfig::warning(const String &str)
{
	general_io_mutex.lock();
	cerr<<"synfig("<<getpid()<<")"<<current_time().c_str()<<_("warning")<<": "<<str.c_str()<<endl;
	general_io_mutex.unlock();
}

void
synfig::info(const char *format,...)
{
	va_list args;
	va_start(args,format);
	info(vstrprintf(format,args));
}

void
synfig::info(const String &str)
{
	general_io_mutex.lock();
	cerr<<"synfig("<<getpid()<<")"<<current_time().c_str()<<_("info")<<": "<<str.c_str()<<endl;
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
	char* path2 = (char*)malloc(buf_size);
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
	
	free(path2);

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
