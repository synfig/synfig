/* === S I N F G =========================================================== */
/*!	\file importer.h
**	\brief writeme
**
**	$Id: importer.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

/* === S T A R T =========================================================== */

#ifndef __SINFG_IMPORTER_H
#define __SINFG_IMPORTER_H

/* === H E A D E R S ======================================================= */

#include <map>
//#include <cmath>
#include <ETL/handle>
#include "string.h"
//#include "surface.h"
//#include "general.h"
//#include "vector.h"
#include "time.h"
#include "gamma.h"

/* === M A C R O S ========================================================= */

//! \writeme
#define SINFG_IMPORTER_MODULE_EXT public: static const char name__[], version__[], ext__[],cvs_id__[]; static Importer *create(const char *filename);

//! Sets the name of the importer
#define SINFG_IMPORTER_SET_NAME(class,x) const char class::name__[]=x

//! \writeme
#define SINFG_IMPORTER_SET_EXT(class,x) const char class::ext__[]=x

//! Sets the version of the importer
#define SINFG_IMPORTER_SET_VERSION(class,x) const char class::version__[]=x

//! Sets the CVS ID of the importer
#define SINFG_IMPORTER_SET_CVS_ID(class,x) const char class::cvs_id__[]=x

//! \writeme
#define SINFG_IMPORTER_INIT(class) sinfg::Importer* class::create(const char *filename) { return new class(filename); }

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {

class Surface;
class ProgressCallback;

/*!	\class Importer
**	\brief Used for importing bitmaps of various formats, including animations
**	\todo Write more detailed description
*/
class Importer : public etl::shared_object
{
public:
	typedef Importer* (*Factory)(const char *filename);
	typedef std::map<String,Factory> Book;
	static Book* book_;
	
	static Book& book();
	
	static bool subsys_init();
	static bool subsys_stop();

	typedef etl::handle<Importer> Handle;
	typedef etl::loose_handle<Importer> LooseHandle;
	typedef etl::handle<const Importer> ConstHandle;

private:
	Gamma gamma_;

protected:
	Importer();

public:

	Gamma& gamma() { return gamma_; }
	const Gamma& gamma()const { return gamma_; }
	
	virtual ~Importer();

	//! Gets a frame and puts it into \a surface
	/*!	\param	surface Reference to surface to put frame into
	**	\param	time	For animated importers, determines which frame to get.
	**		For static importers, this parameter is unused.
	**	\param	callback Pointer to callback class for progress, errors, etc.
	**	\return \c true on success, \c false on error
	**	\see ProgressCallback, Surface
	*/
	virtual bool get_frame(Surface &surface,Time time, ProgressCallback *callback=NULL)=0;

	//! Returns \c true if the importer pays attention to the \a time parameter of get_frame()
	virtual bool is_animated() { return false; }

	//! Attempts to open \a filename, and returns a handle to the associated Importer
	static Handle open(const String &filename);
};

}; // END of namespace sinfg

/* === E N D =============================================================== */

#endif
