/* === S Y N F I G ========================================================= */
/*!	\file cairoimporter.h
**	\brief It is the base class for all the cairo importers.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2012-2013 Carlos López
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

#ifndef __SYNFIG_CAIROIMPORTER_H
#define __SYNFIG_CAIROIMPORTER_H

/* === H E A D E R S ======================================================= */

#include <cstdio>


#include <map>

#include <ETL/handle>

#include "cairo.h"
#include "filesystem.h"
#include "color.h"
#include "progresscallback.h"
#include "renddesc.h"
#include "string.h"
#include "time.h"

/* === M A C R O S ========================================================= */

//! Defines various variables and the create method, common for all importers.
//! To be used in the private part of the importer class definition.
#define SYNFIG_CAIROIMPORTER_MODULE_EXT \
		public: static const char name__[], version__[], ext__[],cvs_id__[]; \
		static const bool supports_file_system_wrapper__; \
		static synfig::CairoImporter *create(const synfig::FileSystem::Identifier &identifier);

//! Defines constructor for class derived from other class which derived from CairoImporter
#define SYNFIG_CAIROIMPORTER_MODULE_CONSTRUCTOR_DERIVED(class, parent) \
		public: class(const synfig::FileSystem::Identifier &identifier): parent(identifier) { }

//! Defines constructor for class derived from CairoImporter
#define SYNFIG_CAIROIMPORTER_MODULE_CONSTRUCTOR(class) \
		SYNFIG_CAIROIMPORTER_MODULE_CONSTRUCTOR_DERIVED(class, synfig::CairoImporter)

//! Defines various variables and the create method, common for all importers.
//! To be used in the private part of the importer class definition.
//! And defines constructor for class derived from other class which derived from CairoImporter
#define SYNFIG_CAIROIMPORTER_MODULE_DECLARATIONS_DERIVED(class, parent) \
		SYNFIG_CAIROIMPORTER_MODULE_EXT \
		SYNFIG_CAIROIMPORTER_MODULE_CONSTRUCTOR_DERIVED(class, parent)

//! Defines various variables and the create method, common for all importers.
//! To be used in the private part of the importer class definition.
//! And defines constructor
#define SYNFIG_CAIROIMPORTER_MODULE_DECLARATIONS(class) \
		SYNFIG_CAIROIMPORTER_MODULE_EXT \
		SYNFIG_CAIROIMPORTER_MODULE_CONSTRUCTOR(class)

//! Sets the name of the importer.
#define SYNFIG_CAIROIMPORTER_SET_NAME(class,x) const char class::name__[]=x

//! Sets the primary file extension of the importer.
#define SYNFIG_CAIROIMPORTER_SET_EXT(class,x) const char class::ext__[]=x

//! Sets the version of the importer.
#define SYNFIG_CAIROIMPORTER_SET_VERSION(class,x) const char class::version__[]=x

//! Sets the CVS ID of the importer.
#define SYNFIG_CAIROIMPORTER_SET_CVS_ID(class,x) const char class::cvs_id__[]=x

//! Sets the supports_file_system_wrapper flag of the importer.
#define SYNFIG_CAIROIMPORTER_SET_SUPPORTS_FILE_SYSTEM_WRAPPER(class,x) const bool class::supports_file_system_wrapper__=x

//! Defines de implementation of the create method for the importer
//! \param filename The file name to be imported by the importer.
#define SYNFIG_CAIROIMPORTER_INIT(class) synfig::CairoImporter* class::create(const synfig::FileSystem::Identifier &identifier) { return new class(identifier); }

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ProgressCallback;

/*!	\class CairoImporter
**	\brief Used for importing bitmaps of various formats, including animations.
*
*	It is the base class for all the importers for CairoSurfaces. It has a static Book
*	pointer class that is a map for the importers factory creators and the strings
*	of the extension that the importer can understand. It allows to create the a
*	pointer to a particular importer just by using the extension of the name of file
*	to import. Also it creates a virtual member get_frame that must be declared in
*	the inherited classes.
*	\see module.h
**	\
*/
class CairoImporter : public etl::shared_object
{
public:
	//! Type that represents a pointer to a CairoImporter's constructor.
	//! As a pointer to the constructor, it represents a "factory" of importers.
	typedef CairoImporter* (*Factory)(const FileSystem::Identifier &identifier);

	struct BookEntry
	{
		Factory factory;
		bool supports_file_system_wrapper;

		BookEntry(): factory(NULL), supports_file_system_wrapper(false) { }
		BookEntry(Factory factory, bool supports_file_system_wrapper):
		factory(factory), supports_file_system_wrapper(supports_file_system_wrapper)
		{ }
	};

	typedef std::map<std::string,BookEntry> Book;
	static Book* book_;

	static Book& book();

	//! Initializes the CairoImport module by creating a book of importers names
	//! and its creators and the list of open importers
	static bool subsys_init();
	//! Stops the Import module by deleting the book and the list of open
	//! importers
	static bool subsys_stop();

	typedef etl::handle<CairoImporter> Handle;
	typedef etl::loose_handle<CairoImporter> LooseHandle;
	typedef etl::handle<const CairoImporter> ConstHandle;

protected:
	CairoImporter(const FileSystem::Identifier &identifier);

public:
	const FileSystem::Identifier identifier;

	virtual ~CairoImporter();

	//! Gets a frame and puts it into \a cairo_surface_t
	/*!	\param	surface Reference to surface to put frame into
	**	\param	time	For animated importers, determines which frame to get.
	**		For static importers, this parameter is unused.
	**	\param	callback Pointer to callback class for progress, errors, etc.
	**	\return \c true on success, \c false on error
	**	\see ProgressCallback, Surface
	*/
	virtual bool get_frame(cairo_surface_t *&csurface, const RendDesc &renddesc, Time time, ProgressCallback *callback=NULL)=0;
	virtual bool get_frame(cairo_surface_t *&csurface, const RendDesc &renddesc,Time time,
						   bool &trimmed,
						   unsigned int &width,
						   unsigned int &height,
						   unsigned int &top,
						   unsigned int &left,
						   ProgressCallback *callback=NULL)
	{
		return get_frame(csurface,renddesc,time,callback);
	}

	//! Returns \c true if the importer pays attention to the \a time parameter of get_frame()
	virtual bool is_animated() { return false; }

	//! Attempts to open \a filename, and returns a handle to the associated CairoImporter
	static Handle open(const FileSystem::Identifier &identifier);
};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
