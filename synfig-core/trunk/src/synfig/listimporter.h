/* === S Y N F I G ========================================================= */
/*!	\file listimporter.h
**	\brief Template Header
**
**	$Id: listimporter.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#ifndef __SYNFIG_LISTIMPORTER_H
#define __SYNFIG_LISTIMPORTER_H

/* === H E A D E R S ======================================================= */

#include "importer.h"
#include "surface.h"
#include <ETL/smart_ptr>
#include <vector>
//#include <deque>
#include <list>
#include <utility>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class ListImporter
**	\todo Write more detailed description
*/
class ListImporter : public Importer
{
	float fps;
	std::vector<String> filename_list;
	std::list<std::pair<int,Surface> > frame_cache;
protected:
	ListImporter(const String &filename);

public:

	virtual ~ListImporter();

	virtual bool get_frame(Surface &surface,Time time, ProgressCallback *callback=NULL);

	virtual bool is_animated();

	static Importer* create(const char *filename);
};
	
}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
