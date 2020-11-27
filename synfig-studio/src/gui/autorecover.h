/* === S Y N F I G ========================================================= */
/*!	\file autorecover.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#ifndef __SYNFIG_AUTORECOVER_H
#define __SYNFIG_AUTORECOVER_H

/* === H E A D E R S ======================================================= */

#include <sigc++/sigc++.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class AutoRecover
{
	bool enabled;
	int timeout_ms;
	sigc::connection connection;

	void set_timer(bool enabled, int timeout_ms);
public:
	AutoRecover();
	~AutoRecover();

	bool get_enabled() const
		{ return enabled; }
	void set_enabled(bool value)
		{ set_timer(value, get_timeout_ms()); }

	int get_timeout_ms() const
		{ return timeout_ms; }
	void set_timeout_ms(int value)
		{ set_timer(get_enabled(), value); }

	void auto_backup();

	bool recovery_needed()const;
	bool recover(int& number_recovered);
	bool clear_backups();
}; // END of class AutoRecover

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
