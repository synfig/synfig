/* === S I N F G =========================================================== */
/*!	\file editmode.h
**	\brief Template Header
**
**	$Id: editmode.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SINFG_EDITMODE_H
#define __SINFG_EDITMODE_H

/* === H E A D E R S ======================================================= */

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfgapp {
	
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
inline bool
operator<=(const EditMode& lhs, const EditMode& rhs)
{ return lhs&rhs==int(rhs); }

}; // END if namespace sinfgapp
/* === E N D =============================================================== */

#endif
