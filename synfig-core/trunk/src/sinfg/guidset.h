/* === S I N F G =========================================================== */
/*!	\file guidset.h
**	\brief Template Header
**
**	$Id: guidset.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#ifndef __SINFG_GUIDSET_H
#define __SINFG_GUIDSET_H

#define HASH_SET_H <ext/hash_set>

/* === H E A D E R S ======================================================= */

#include "guid.h"

#ifdef HASH_SET_H
#include HASH_SET_H
#else
#include <set>
#endif

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {
	
class GUIDSet : public
#ifdef HASH_SET_H
std::set<sinfg::GUID>
#else
std::hash_set<sinfg::GUID,sinfg::GUIDHash>
#endif
{
}; // END of class GUIDSet
	
};

/* === E N D =============================================================== */

#endif
