/* === S Y N F I G ========================================================= */
/*!	\file boneweightpair.h
**	\brief A weighted bone
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#ifndef __SYNFIG_BONE_WEIGHT_PAIR_H
#define __SYNFIG_BONE_WEIGHT_PAIR_H

/* === H E A D E R S ======================================================= */

#include "real.h"
#include "string.h"
#include "bone.h"
// #include <ETL/handle>
#include <ETL/stringf>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

//class ValueNode_Bone;

/*!	\class BoneWeightPair
**	\todo writeme
*/
class BoneWeightPair
{
private:
	// etl::handle<ValueNode_Bone> bone;
	Bone bone;
	Real weight;

public:
	BoneWeightPair(): weight() {}
	BoneWeightPair(Bone bone, Real weight): bone(bone), weight(weight) { }

	Real get_weight()const { return weight; }
	Bone get_bone()const { return bone; }

	//!Get the string of the BoneWeightPair
	//!@return String type. A string representation of the two components.
	String get_string()const { return etl::strprintf("(%.2f) %s", weight, bone.get_name().c_str()); }
};

}; // END of namespace synfig

#endif
