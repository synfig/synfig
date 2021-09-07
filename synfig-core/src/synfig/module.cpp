/* === S Y N F I G ========================================================= */
/*!	\file synfig/module.cpp
**	\brief writeme
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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


#include "module.h"

#include "general.h"
#include <synfig/localization.h>
#include "type.h"
#include <glibmm.h>

#ifndef USE_CF_BUNDLES
#include <ltdl.h>
#endif

#endif

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

using namespace etl;
using namespace synfig;

Module::Book *synfig::Module::book_;

/* === P R O C E D U R E S ================================================= */

bool
Module::subsys_init(const String &prefix)
{
#ifndef USE_CF_BUNDLES

	if(lt_dlinit())
	{
		error(_("Errors on lt_dlinit()"));
		error(lt_dlerror());
		return false;
	}

	// user's synfig library path
#ifdef _WIN32
	if(const char *localappdata = getenv("%LOCALAPPDATA%")) {
		std::string user_module_path = Glib::locale_from_utf8(localappdata) + "/synfig/modules";
		lt_dladdsearchdir(user_module_path.c_str());
	}
#else
#ifdef __APPLE__
	if(const char *home = getenv("HOME"))
		lt_dladdsearchdir(strprintf("%s/Library/Application Support/org.synfig.SynfigStudio/modules", home).c_str());
#else
	if(const char *home = getenv("HOME"))
		lt_dladdsearchdir(strprintf("%s/.local/share/synfig/modules", home).c_str());
#endif
#endif

	// (runtime) prefix path
	lt_dladdsearchdir((Glib::locale_from_utf8(prefix) + "/lib/synfig/modules").c_str());

	// path defined on build time
#ifdef LIBDIR
	lt_dladdsearchdir(LIBDIR"/synfig/modules");
#endif
#ifdef __APPLE__
	lt_dladdsearchdir("/Library/Frameworks/synfig.framework/Resources/modules");
#endif

	// current working path...
	lt_dladdsearchdir(".");
#endif
	book_=new Book;
	return true;
}

bool
Module::subsys_stop()
{
	delete book_;

#ifndef USE_CF_BUNDLES
	lt_dlexit();
#endif
	return true;
}

void
Module::register_default_modules(ProgressCallback *callback)
{
	#define REGISTER_MODULE(module) if (!Register(module, callback)) \
										throw std::runtime_error(strprintf(_("Unable to load module '%s'"), module))
	REGISTER_MODULE("lyr_freetype");
	REGISTER_MODULE("mod_geometry");
	REGISTER_MODULE("mod_gradient");
	REGISTER_MODULE("mod_particle");
}

Module::Book&
Module::book()
{
	return *book_;
}

void
synfig::Module::Register(Module::Handle mod)
{
	book()[mod->Name()]=mod;
}

bool
synfig::Module::Register(const String &module_name, ProgressCallback *callback)
{
#ifndef USE_CF_BUNDLES
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
		mod=handle<Module>((*constructor)(callback));
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

#endif
	return true;
}

synfig::Module::~Module()
{
	destructor_();
}
