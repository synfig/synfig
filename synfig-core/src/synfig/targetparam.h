/* === S Y N F I G ========================================================= */
/*!	\file synfig/targetparam.h
**	\brief Class for extra parameters of the target modules
**
**	$Id$
**
**	\legal
**	Copyright (c) 2010 Diego Barrios Romero
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

#ifndef __SYNFIG_TARGETPARAM_H
#define __SYNFIG_TARGETPARAM_H

#include <string>

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

struct TargetParam
{
	//Spritesheet render direction
	enum Direction {
		HR = 0, //Horizontal 
		VR = 1  //Vertical
	};
	
	//! Constructor
	/*! Not valid default values, if they are not modified before
	 *  passing them to the target module, it would override them with
	 *  its own valid default settings.
	 */
	TargetParam (const std::string& Video_codec = "none", int Bitrate = -1):
		video_codec(Video_codec), bitrate(Bitrate), sequence_separator("."), offset_x(0), offset_y(0),rows(0),columns(0),append(true),dir(HR)
	{ }

	std::string video_codec;
	int bitrate;
	std::string sequence_separator;
	//TODO: It is a spike. Need to separate this class.
	int offset_x;
	int offset_y;
	int rows;
	int columns;
	bool append;
	Direction dir;
};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif

