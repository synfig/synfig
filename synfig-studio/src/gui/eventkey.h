/* === S Y N F I G ========================================================= */
/*!	\file eventkey.h
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

#ifndef __SYNFIG_STUDIO_EVENTKEY_H
#define __SYNFIG_STUDIO_EVENTKEY_H

/* === H E A D E R S ======================================================= */

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

enum EventKey
{
	EVENT_NIL,
	EVENT_REFRESH,
	EVENT_DIRTY,
	EVENT_STOP,
	EVENT_UNDO,
	EVENT_REDO,
	EVENT_REFRESH_DUCKS,
	EVENT_REFRESH_TOOL_OPTIONS,
	EVENT_YIELD_TOOL_OPTIONS,
	EVENT_INPUT_DEVICE_CHANGED,
	EVENT_TABLES_HIDE,
	EVENT_LAYER_SELECTION_CHANGED,
	EVENT_TABLES_SHOW,


	EVENT_WORKAREA_START=1000,		//!< Not a valid event
	EVENT_WORKAREA_LAYER_CLICKED,
	EVENT_WORKAREA_MULTIPLE_DUCKS_CLICKED,
	EVENT_WORKAREA_MOUSE_MOTION,
	EVENT_WORKAREA_KEY_DOWN,
	EVENT_WORKAREA_KEY_UP,
	EVENT_WORKAREA_MOUSE_BUTTON_DOWN,
	EVENT_WORKAREA_MOUSE_BUTTON_DRAG,
	EVENT_WORKAREA_MOUSE_BUTTON_UP,
	EVENT_WORKAREA_MOUSE_2BUTTON_DOWN,
	EVENT_WORKAREA_BOX,
	EVENT_WORKAREA_END,		//!< Not a valid event

	EVENT_WORKAREA_STROKE,

	EVENT_END		//!< Not a valid event
};

};

/* === E N D =============================================================== */

#endif
