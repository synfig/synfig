/* === S I N F G =========================================================== */
/*!	\file eventkey.h
**	\brief Template Header
**
**	$Id: eventkey.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SINFG_STUDIO_EVENTKEY_H
#define __SINFG_STUDIO_EVENTKEY_H

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
	EVENT_WORKAREA_MOUSE_BUTTON_DOWN,
	EVENT_WORKAREA_MOUSE_BUTTON_DRAG,
	EVENT_WORKAREA_MOUSE_BUTTON_UP,
	EVENT_WORKAREA_BOX,
	EVENT_WORKAREA_END,		//!< Not a valid event

	EVENT_WORKAREA_STROKE,

	EVENT_END		//!< Not a valid event	
};

};

/* === E N D =============================================================== */

#endif
