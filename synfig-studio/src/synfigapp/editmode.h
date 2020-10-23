/* === S Y N F I G ========================================================= */
/*!	\file editmode.h
**	\brief Template Header
**
**	$Id$
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

#ifndef __SYNFIG_EDITMODE_H
#define __SYNFIG_EDITMODE_H

/* === H E A D E R S ======================================================= */

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

/*!	\enum EditMode
**	\brief \writeme
*/
enum EditMode
{
	MODE_NORMAL			=0,			//!< Normal editing mode. Place holder.

	MODE_ANIMATE		=(1<<0),	//!< Animated editing mode.
	MODE_ANIMATE_FUTURE	=(1<<1),	//!< Respect <i>future</i> keyframes
	MODE_ANIMATE_PAST	=(1<<2),	//!< Respect <i>past</i> keyframes
	MODE_ANIMATE_ALL	=(3<<1),	//!< Respect <i>all</i> keyframes

	MODE_UNDEFINED		=(~0)	//!< Undefined Mode
}; // END of enum EditMode

//! Combine Flags
inline EditMode
operator|(const EditMode& lhs, const EditMode& rhs)
{ return static_cast<EditMode>(int(lhs)|int(rhs)); }

//! Exclude Flags
inline EditMode
operator-(const EditMode& lhs, const EditMode& rhs)
{ return static_cast<EditMode>(int(lhs)&~int(rhs)); }

inline EditMode&
operator|=(EditMode& lhs, const EditMode& rhs)
{ *reinterpret_cast<int*>(&lhs)|=int(rhs); return lhs; }

//!	Flag Comparison. THIS IS NOT LESS-THAN-OR-EQUAL-TO.
/*!	This function will return true of all of the flags
**	in the \a rhs are set in the \a lhs */
inline EditMode
operator&(const EditMode& lhs, const EditMode& rhs)
{ return static_cast<EditMode>(int(lhs)&int(rhs)); }

//!	Flag Comparison. THIS IS NOT LESS-THAN-OR-EQUAL-TO.
/*!	This function will return true of all of the flags
**	in the \a rhs are set in the \a lhs */
// inline bool
// operator<=(const EditMode& lhs, const EditMode& rhs)
// { return (lhs&rhs)==int(rhs); }

}; // END if namespace synfigapp
/* === E N D =============================================================== */

#endif
