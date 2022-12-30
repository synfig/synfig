/* === S Y N F I G ========================================================= */
/*!	\file synfig/module.cpp
**	\brief writeme
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif


#include "module.h"

#include "general.h"
#include <synfig/localization.h>
#include "type.h"
#include <glibmm.h>

#include <ltdl.h>

#endif

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

static void add_search_dir(const std::string& dir) {
	lt_dladdsearchdir(dir.c_str());
#ifdef _MSC_VER
	const std::string path = Glib::getenv("PATH");
	std::string new_path = path + ";" + dir;
	Glib::setenv("PATH", new_path);
#endif
}

bool
synfig::Module::subsys_init(const std::string& prefix)
{
	if(lt_dlinit())
	{
		error(_("Errors on lt_dlinit()"));
		error(lt_dlerror());
		return false;
	}

	// user's synfig library path
#ifdef _WIN32
	std::string localappdata = Glib::getenv("%LOCALAPPDATA%");
	if (!localappdata.empty()) {
		std::string user_module_path = localappdata + "/synfig/modules";
		add_search_dir(user_module_path);
	}
#elif defined(__APPLE__)
	std::string home = Glib::getenv("HOME");
	if (!home.empty())
		add_search_dir(home + "/Library/Application Support/org.synfig.SynfigStudio/modules");

	add_search_dir("/Library/Frameworks/synfig.framework/Resources/modules");
#else
	std::string home = Glib::getenv("HOME");
	if (!home.empty())
		add_search_dir(home + "/.local/share/synfig/modules");
#endif

	// (runtime) prefix path
	add_search_dir((Glib::locale_from_utf8(prefix) + "/lib/synfig/modules"));

	// path defined on build time
#ifdef LIBDIR
	add_search_dir(LIBDIR"/synfig/modules");
#endif

	// current working path...
	add_search_dir(".");

	return true;
}

bool
synfig::Module::subsys_stop()
{
	lt_dlexit();
	return true;
}

void
synfig::Module::register_default_modules(ProgressCallback *callback)
{
	#define REGISTER_MODULE(module) if (!Register(module, callback)) \
										throw std::runtime_error(strprintf(_("Unable to load module '%s'"), module))
	REGISTER_MODULE("lyr_freetype");
	REGISTER_MODULE("mod_geometry");
	REGISTER_MODULE("mod_gradient");
	REGISTER_MODULE("mod_particle");
}

synfig::Module::Book&
synfig::Module::book()
{
	static Book book_;
	return book_;
}

void
synfig::Module::Register(Module::Handle mod)
{
	book()[mod->Name()]=mod;
}

bool
synfig::Module::Register(const String &module_name, ProgressCallback *callback)
{
	// reset error string
	lt_dlerror();

	lt_dlhandle module;

	if(callback)callback->task(strprintf(_("Attempting to register \"%s\""),module_name.c_str()));

	module=lt_dlopenext((std::string("lib")+module_name).c_str());
	if(!module)module=lt_dlopenext(module_name.c_str());
	Type::initialize_all();

	if(!module)
	{
		if(callback)callback->warning(strprintf(_("Unable to find module \"%s\" (%s)"), module_name.c_str(), lt_dlerror()));
		return false;
	}

	if(callback)callback->task(strprintf(_("Found module \"%s\""),module_name.c_str()));

	Module::constructor_type constructor=nullptr;
	Handle mod;

	const std::vector<const char*> symbol_prefixes = {"", "lib", "_lib", "_"};
	for (const char * symbol_prefix : symbol_prefixes)
	{
		constructor=(Module::constructor_type )lt_dlsym(module,(symbol_prefix+module_name+"_LTX_new_instance").c_str());
		if (constructor)
			break;
	}

	if(constructor)
	{
		mod=etl::handle<Module>((*constructor)(callback));
	}
	else
	{
		if(callback)callback->error(strprintf(_("Unable to find entrypoint in module \"%s\" (%s)"),module_name.c_str(),lt_dlerror()));
		return false;
	}

	if(mod)
	{
		Register(mod);
	}
	else
	{
		if(callback)callback->error(_("Entrypoint did not return a module."));
		return false;
    }

	if(callback)callback->task(strprintf(_("Success for \"%s\""),module_name.c_str()));

	return true;
}

synfig::Module::~Module()
{
	destructor_();
}
