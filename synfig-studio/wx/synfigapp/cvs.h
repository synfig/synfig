/* === S Y N F I G ========================================================= */
/*!	\file cvs.h
**	\brief Template Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_CVS_H
#define __SYNFIG_CVS_H

/* === H E A D E R S ======================================================= */

#include <synfig/string.h>
#include <time.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

class CVSInfo
{
	synfig::String file_name_;

	bool in_sandbox_;
	bool in_repository_;
	bool update_available_;

	synfig::String cvs_version_;
	time_t original_timestamp_;


public:
	void calc_repository_info();

	CVSInfo(const synfig::String& file_name);
	CVSInfo();
	~CVSInfo();

	void set_file_name(const synfig::String& file_name);

//	READ OPERATIONS --------------------------------------------------

	//! Returns TRUE if \a file_name is in a sandbox
	bool in_sandbox()const;

	//! Returns TRUE if \a file_name is in the repository
	bool in_repository()const;

	//! Returns TRUE if \a file_name has modifications not yet on the repository
	bool is_modified()const;

	//! Returns TRUE if there is a new version of \a file_name on the repository
	bool is_updated()const;

	//! Returns the CVS version string
	const synfig::String& get_cvs_version()const;

	//! Returns the unix timestamp of the repository file
	const time_t &get_original_timestamp()const;

	//! Returns the unix timestamp of the checked out file
	time_t get_current_timestamp()const;

	//! Returns the Root
	synfig::String get_cvs_root()const;

	//! Returns the name of the module
	synfig::String get_cvs_module()const;

//	WRITE OPERATIONS -------------------------------------------------

	void cvs_add(const synfig::String& message=synfig::String());

	void cvs_update();

	void cvs_commit(const synfig::String& message=synfig::String());
}; // END of class CVSInfo

}; // END of namespace synfigapp

/* === E N D =============================================================== */

#endif
