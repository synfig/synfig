/* === S Y N F I G ========================================================= */
/*!	\file autorecover.h
**	\brief Template Header
**
**	$Id: autorecover.h,v 1.1.1.1 2005/01/07 03:34:35 darco Exp $
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

#ifndef __SYNFIG_AUTORECOVER_H
#define __SYNFIG_AUTORECOVER_H

/* === H E A D E R S ======================================================= */

#include <synfig/string.h>
#include <synfig/canvas.h>
#include <sigc++/connection.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class AutoRecover
{
	int timeout;
	SigC::Connection auto_backup_connect;
public:
	AutoRecover();
	~AutoRecover();

	static int pid();
	static synfig::String get_shadow_file_name(const synfig::String& filename);

	static bool auto_backup();

	static bool cleanup_pid(int pid);

	void set_timeout(int milliseconds);
	int get_timeout()const { return timeout; }
	
	static synfig::String get_shadow_directory();
	
	bool recovery_needed()const;
	bool recover();
	
	void normal_shutdown();
	
	void clear_backup(synfig::Canvas::Handle canvas);
}; // END of class AutoRecover

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
