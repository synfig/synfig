/* === S Y N F I G ========================================================= */
/*!	\file uniqueid.h
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

#ifndef __SYNFIG_UNIQUEID_H
#define __SYNFIG_UNIQUEID_H

/* === H E A D E R S ======================================================= */

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*! \class UniqueID
**	\brief \todo
*/
class UniqueID
{
	int id_;

	explicit UniqueID(int id_):id_(id_) { }

	static int next_id();

public:

	//! Returns the internal unique identifier for this object.
	/*! The return value from this isn't really useful for
	**	much other than debug output. Nonetheless, that is
	**	one step above useless, so here it is. */
	const int &get_uid()const { return id_; }

	UniqueID():id_(next_id()) { }

	void make_unique() { id_=next_id(); }

	static const UniqueID nil() { return UniqueID(0); }

	operator bool()const { return static_cast<bool>(id_); }

	void mimic(const UniqueID& x) { id_=x.id_; }

	bool operator==(const UniqueID &rhs)const { return id_==rhs.id_; }
	bool operator!=(const UniqueID &rhs)const { return id_!=rhs.id_; }
	bool operator<(const UniqueID &rhs)const { return id_<rhs.id_; }
}; // END of class UniqueID

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
