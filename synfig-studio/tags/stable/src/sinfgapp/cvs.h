/* === S I N F G =========================================================== */
/*!	\file cvs.h
**	\brief Template Header
**
**	$Id: cvs.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SINFG_CVS_H
#define __SINFG_CVS_H

/* === H E A D E R S ======================================================= */

#include <sinfg/string.h>
#include <time.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfgapp {

class CVSInfo
{
	sinfg::String file_name_;

	bool in_sandbox_;
	bool in_repository_;
	bool update_available_;
	
	sinfg::String cvs_version_;
	time_t original_timestamp_;
	
	
public:
	void calc_repository_info();
	
	CVSInfo(const sinfg::String& file_name);
	CVSInfo();
	~CVSInfo();

	void set_file_name(const sinfg::String& file_name);

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
	const sinfg::String& get_cvs_version()const;
	
	//! Returns the unix timestamp of the repository file
	const time_t &get_original_timestamp()const;

	//! Returns the unix timestamp of the checked out file
	time_t get_current_timestamp()const;

	//! Returns the Root
	sinfg::String get_cvs_root()const;

	//! Returns the name of the module
	sinfg::String get_cvs_module()const;
			
//	WRITE OPERATIONS -------------------------------------------------
	
	void cvs_add(const sinfg::String& message=sinfg::String());
	
	void cvs_update();
	
	void cvs_commit(const sinfg::String& message=sinfg::String());
}; // END of class CVSInfo
	
}; // END of namespace sinfgapp

/* === E N D =============================================================== */

#endif
