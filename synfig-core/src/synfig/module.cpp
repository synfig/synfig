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

#include <ETL/stringf>

#include "module.h"

#include "general.h"
#include <synfig/localization.h>
#include "type.h"
#include <glibmm.h>

#ifndef USE_CF_BUNDLES
#include <ltdl.h>
#include <dlfcn.h>
#endif

#endif

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

using namespace std;
using namespace etl;
using namespace synfig;

Module::Book *synfig::Module::book_;

/* === P R O C E D U R E S ================================================= */

bool
Module::subsys_init(const String &prefix)
{
#ifndef USE_CF_BUNDLES
	#ifndef SYNFIG_LTDL_NO_STATIC
	//LTDL_SET_PRELOADED_SYMBOLS();
	#endif

	if(lt_dlinit())
	{
		error(_("Errors on lt_dlinit()"));
		error(lt_dlerror());
		return false;
	}

	if(getenv("HOME"))
		lt_dladdsearchdir(strprintf("%s/.local/share/synfig/modules", getenv("HOME")).c_str());
	lt_dladdsearchdir((Glib::locale_from_utf8(prefix) + \
		ETL_DIRECTORY_SEPARATOR + "lib" + \
		ETL_DIRECTORY_SEPARATOR + "synfig" + \
		ETL_DIRECTORY_SEPARATOR + "modules" ).c_str());
#ifdef LIBDIR
	lt_dladdsearchdir(LIBDIR"/synfig/modules");
#endif
#ifdef __APPLE__
	lt_dladdsearchdir("/Library/Frameworks/synfig.framework/Resources/modules");
#endif
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

	module=lt_dlopenext((string("lib")+module_name).c_str());
	if(!module)module=lt_dlopenext(module_name.c_str());
	Type::initialize_all();

	if(!module)
	{
		if(callback)callback->warning(strprintf(_("Unable to find module \"%s\" (%s/%s)"), module_name.c_str(), lt_dlerror(), dlerror()));
		return false;
	}

	if(callback)callback->task(strprintf(_("Found module \"%s\""),module_name.c_str()));

	Module::constructor_type constructor=NULL;
	Handle mod;

	if(!constructor)
	{
//		if(callback)callback->task(string("looking for -> ")+module_name+"_LTX_new_instance()");
		constructor=(Module::constructor_type )lt_dlsym(module,(module_name+"_LTX_new_instance").c_str());
	}

	if(!constructor)
	{
//		if(callback)callback->task(string("looking for -> lib")+module_name+"_LTX_new_instance()");
		constructor=(Module::constructor_type )lt_dlsym(module,(string("lib")+module_name+"_LTX_new_instance").c_str());
	}
	if(!constructor)
	{
//		if(callback)callback->task(string("looking for -> lib")+module_name+"_LTX_new_instance()");
		constructor=(Module::constructor_type )lt_dlsym(module,(string("_lib")+module_name+"_LTX_new_instance").c_str());
	}
	if(!constructor)
	{
//		if(callback)callback->task(string("looking for -> lib")+module_name+"_LTX_new_instance()");
		constructor=(Module::constructor_type )lt_dlsym(module,(string("_")+module_name+"_LTX_new_instance").c_str());
	}

	if(constructor)
	{
//		if(callback)callback->task(strprintf("Executing callback for \"%s\"",module_name.c_str()));
		mod=handle<Module>((*constructor)(callback));
	}
	else
	{
		if(callback)callback->error(strprintf(_("Unable to find entrypoint in module \"%s\" (%s)"),module_name.c_str(),lt_dlerror()));
		return false;
	}

//	if(callback)callback->task(strprintf("Done executing callback for \"%s\"",module_name.c_str()));

	if(mod)
	{
//		if(callback)callback->task(strprintf("Registering \"%s\"",module_name.c_str()));
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

//! Virtual Modules properties wrappers. Must be defined in the modules classes
const char * Module::Name() { return " "; }
const char * Module::Desc() { return " "; }
const char * Module::Author() { return " "; }
const char * Module::Version() { return " "; }
const char * Module::Copyright() { return SYNFIG_COPYRIGHT; }
