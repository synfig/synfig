/* === S Y N F I G ========================================================= */
/*!	\file ipc.h
**	\brief Template Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_IPC_H
#define __SYNFIG_IPC_H

/* === H E A D E R S ======================================================= */

#include <glibmm/iochannel.h>
#include <synfig/smartfile.h>
#include <synfig/string.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class IPC
{
private:

	int fd;
	synfig::SmartFILE file;

	bool fifo_activity(Glib::IOCondition cond);

public:
	IPC();
	~IPC();

	static synfig::String fifo_path();
	static synfig::SmartFILE make_connection();

	static bool process_command(const synfig::String& cmd);
}; // END of class IPC

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
