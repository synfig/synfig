/* === S Y N F I G ========================================================= */
/*!	\file state_minimal.h
**	\brief Transitional state context base class
**
**	$Id$
**
**	\legal
**	Copyright (c) 2016 caryoscelus
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

#ifndef __SYNFIG_STUDIO_STATE_MINIMAL_H
#define __SYNFIG_STUDIO_STATE_MINIMAL_H

/* === H E A D E R S ======================================================= */
#include "canvasview.h"

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

//! Transitional class
//! TODO: When it's not required anymore, merge into State_Context
class StateMinimal_Context : public sigc::trackable
{
public:
	virtual void enter() {}
	virtual void leave() {}
};

}

#endif
