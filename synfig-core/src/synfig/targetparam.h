/* === S Y N F I G ========================================================= */
/*!	\file synfig/targetparam.h
**	\brief Class for extra parameters of the target modules
**
**	$Id$
**
**	\legal
**	Copyright (c) 2010 Diego Barrios Romero
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

#ifndef __SYNFIG_TARGETPARAM_H
#define __SYNFIG_TARGETPARAM_H

#include <string>

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

struct TargetParam
{
	std::string video_codec;
	int bitrate;
};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif

