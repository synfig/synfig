#if 0							// this file is not used
/* === S Y N F I G ========================================================= */
/*!	\file nodebase.h
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

#ifndef __SYNFIG_NODEBASE_H
#define __SYNFIG_NODEBASE_H

/* === H E A D E R S ======================================================= */

#include "protocol.h"
#include "string.h"
#include "guid.h"
#include <sigc++/slot.h>

/* === M A C R O S ========================================================= */

#define PX_DEFINE_DATA(name,type) \
    PX_DEFINE_FUNC_CONST0(get_##name, type) \
    PX_DEFINE_FUNC1(set_##name, void, type)

#define PX_DEFINE_FUNC0(name,ret) \
	sigc::slot< ret > _slot_##name; \
	ret name() { \
		return _slot_##name(); \
	}

#define PX_DEFINE_FUNC1(name,ret,type) \
	sigc::slot< ret, type > _slot_##name; \
	ret name(type v1) { \
		return _slot_##name(v1); \
	}
#define PX_DEFINE_FUNC2(name,ret,type1,type2) \
	sigc::slot< ret, type1, type2 > _slot_##name; \
	ret name(type1 v1, type2 v2) { \
		return _slot_##name(v1,v2); \
	}
#define PX_DEFINE_FUNC_CONST0(name,ret) \
	sigc::slot< ret > _slot_##name##_const; \
	ret name()const { \
		return _slot_##name##_const(); \
	}

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {
namespace Proto {

typedef int Query;
typedef int NodeList;

class NodeBase : public Protocol
{
public:

	PX_DEFINE_DATA(guid, GUID)

	PX_DEFINE_FUNC2(func_test, float, int, int)

	PX_DEFINE_DATA(id, String)

	PX_DEFINE_DATA(root, NodeHandle)

	PX_DEFINE_FUNC0(signal_changed, sigc::signal<void>)
	PX_DEFINE_FUNC0(signal_deleted, sigc::signal<void>)

	PX_DEFINE_FUNC_CONST0(get_parents, NodeList)
	PX_DEFINE_FUNC_CONST0(get_children, NodeList)

	PX_DEFINE_FUNC1(query_children, NodeList, Query)

}; // END of class Proto::NodeBase

}; // END of namespace Proto
}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
#endif
