/* === S Y N F I G ========================================================= */
/*!	\file synfig/module.h
**	\brief Base class for all libraries modules
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_MODULE_H
#define __SYNFIG_MODULE_H

/* === H E A D E R S ======================================================= */

#include <ETL/handle>
#include <map>
#include "string.h"
#include "releases.h"
#include "layer.h"

/* === M A C R O S ========================================================= */

//! Marks the start of a module description
#define MODULE_DESC_BEGIN(x) struct x##_modclass : public synfig::Module { x##_modclass(synfig::ProgressCallback *callback=nullptr);

//! Sets the localized name of the module
#define MODULE_NAME(x) 			virtual const char * Name() { return x; }

//! Sets a localized description of the module
#define MODULE_DESCRIPTION(x)	virtual const char * Desc() { return x; }

//! Sets the name of the module's author
#define MODULE_AUTHOR(x)		virtual const char * Author() { return x; }

//! Sets the version string for the module
#define MODULE_VERSION(x)		virtual const char * Version() { return x; }

//! Sets the copyright string for the module
#define MODULE_COPYRIGHT(x)		virtual const char * Copyright() { return x; }

//! Describes the module's construction function
#define MODULE_CONSTRUCTOR(x)	virtual bool constructor_(synfig::ProgressCallback *cb) { return x(cb); }

//! Describes the module's destruction function
#define MODULE_DESTRUCTOR(x)	virtual void destructor_() { return x(); }

//! Marks the end of a module description
#define MODULE_DESC_END };

#ifdef __APPLE__
//! Marks the start of a module's inventory
#define MODULE_INVENTORY_BEGIN(x)  extern "C" {		\
	synfig::Module* _##x##_LTX_new_instance(synfig::ProgressCallback *cb) \
	{ if(SYNFIG_CHECK_VERSION()){x##_modclass *mod=new x##_modclass(cb); mod->constructor_(cb); return mod; }\
	if(cb)cb->error(#x": Unable to load module due to version mismatch."); return nullptr; } \
	}; x##_modclass::x##_modclass(synfig::ProgressCallback */*cb*/) {
#else
//! Marks the start of a module's inventory
#define MODULE_INVENTORY_BEGIN(x)  extern "C" {		\
	synfig::Module* x##_LTX_new_instance(synfig::ProgressCallback *cb) \
	{ if(SYNFIG_CHECK_VERSION()){x##_modclass *mod=new x##_modclass(cb); mod->constructor_(cb); return mod; }\
	if(cb)cb->error(#x": Unable to load module due to version mismatch."); return nullptr; } \
	}; x##_modclass::x##_modclass(synfig::ProgressCallback */*cb*/) {
#endif

//! Marks the start of the layers in the module's inventory
#define BEGIN_LAYERS {

//! Register a Layer class in the book of layers
#define LAYER(class)																			\
	synfig::Layer::register_in_book(															\
		synfig::Layer::BookEntry(class::create,													\
								 class::name__,													\
								 synfigcore_localize(class::local_name__),		\
								 class::category__,												\
								 class::cvs_id__,												\
								 class::version__));

//! Register a Layer class in the book of layers with an alias
#define LAYER_ALIAS(class,alias)																\
	synfig::Layer::register_in_book(															\
		synfig::Layer::BookEntry(class::create,													\
								 alias,															\
								 alias,															\
								 CATEGORY_DO_NOT_USE,											\
								 class::cvs_id__,												\
								 class::version__));

//! Marks the end of the layers in the module's inventory
#define END_LAYERS }

//! Marks the start of the targets in the module's inventory
#define BEGIN_TARGETS {

#define TARGET(x)														\
	synfig::Target::book()[synfig::String(x::name__)].factory =			\
		reinterpret_cast<synfig::Target::Factory> (x::create);			\
	synfig::Target::book()[synfig::String(x::name__)].filename =		\
		synfig::String(x::ext__);										\
	synfig::Target::book()[synfig::String(x::name__)].target_param =	\
		synfig::TargetParam();													\
	synfig::Target::ext_book()[synfig::String(x::ext__)]=x::name__;

#define TARGET_EXT(x,y) synfig::Target::ext_book()[synfig::String(y)]=x::name__;

//! Marks the end of the targets in the module's inventory
#define END_TARGETS }

//! Marks the start of the importers in the module's inventory
#define BEGIN_IMPORTERS {

//! Register an Importer class in the book of importers by one file extension string
#define IMPORTER_EXT(x,y) \
		synfig::Importer::book()[synfig::String(y)]=synfig::Importer::BookEntry(x::create, x::supports_file_system_wrapper__);

//! Register an Importer class in the book of importers by the default extension
#define IMPORTER(x) IMPORTER_EXT(x,x::ext__)

//! Register an Importer class in the book of importers by one file extension string
#define CAIROIMPORTER_EXT(x,y) \
		synfig::CairoImporter::book()[synfig::String(y)]=synfig::CairoImporter::BookEntry(x::create, x::supports_file_system_wrapper__);

//! Register an CairoImporter class in the book of importers by the default extension
#define CAIROIMPORTER(x) CAIROIMPORTER_EXT(x,x::ext__)

//! Marks the end of the importers in the module's inventory
#define END_IMPORTERS }

//! Marks the end of a module's inventory
#define MODULE_INVENTORY_END	}


/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ProgressCallback;

/*!	\class Module
**	\todo writeme
*/
class Module : public etl::shared_object
{
public:
	//! The initializer of the module. Default implementation does nothing
	virtual bool constructor_(synfig::ProgressCallback */*cb*/) { return true; }
	//! The module cleanup funtion
	virtual void destructor_() { }

	typedef etl::handle<Module> Handle;
	typedef etl::loose_handle<Module> LooseHandle;
	typedef etl::handle<const Module> ConstHandle;

	//! Type that represents a pointer to a Module's constructor by name.
	//! As a pointer to the member, it represents a constructor of the module.
	typedef Module* (*constructor_type)(ProgressCallback *);
	typedef std::map<String, Handle> Book;
private:
	static Book* book_;
public:
	static Book& book();

	//! Inits the book of importers and add the paths to search for the
	//! ltdl library utilities.
	static bool subsys_init(const String &prefix);
	static bool subsys_stop();
	//! Register not optional modules
	static void register_default_modules(ProgressCallback *cb=nullptr);

	//! Register Module by handle
	static void Register(Handle mod);
	//! Register Module by name
	static bool Register(const String &module_name, ProgressCallback *cb=nullptr);
	//!Register Module by instance pointer
	static inline void Register(Module *mod) { Register(Handle(mod)); }

	// Virtual Modules properties wrappers.
	// They MUST be defined in the module implementation classes.
	// They SHOULD be defined via MODULE_* macros inside MODULE_DESC_BEGIN/END block.
	//! Localized module name
	virtual const char * Name() = 0;
	//! Localized module description
	virtual const char * Desc() = 0;
	//! Module author(s)'s name
	virtual const char * Author() = 0;
	//! Module version string
	virtual const char * Version() = 0;
	//! Module copyright text
	virtual const char * Copyright() = 0;

	virtual ~Module();
};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
