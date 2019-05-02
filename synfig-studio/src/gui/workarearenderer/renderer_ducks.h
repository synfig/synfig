/* === S Y N F I G ========================================================= */
/*!	\file renderer_ducks.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#ifndef __SYNFIG_RENDERER_DUCKS_H
#define __SYNFIG_RENDERER_DUCKS_H

/* === H E A D E R S ======================================================= */

#include "workarearenderer.h"
#include <vector>

/* === M A C R O S ========================================================= */

/** DUCK_COLOR_NOT_EDITABLE : light grey - for parameter (handle) like converted (linked also?) for example*/
#define DUCK_COLOR_NOT_EDITABLE	Gdk::Color("#cfcfcf")
/** DUCK_COLOR_ORIGIN : green */
#define DUCK_COLOR_ORIGIN        Gdk::Color("#00ff00") // green
/** DUCK_COLOR_ANGLE : blue */
#define DUCK_COLOR_ANGLE		Gdk::Color("#0000ff") // blue
/** DUCK_COLOR_RADIUS : cyan */
#define DUCK_COLOR_RADIUS		Gdk::Color("#00ffff") // cyan
/** DUCK_COLOR_LINEAR : cyan for linear radius ducks */
#define DUCK_COLOR_LINEAR		Gdk::Color("#00ffff") // cyan // for linear radius ducks
/** DUCK_COLOR_TANGENT_1 : yellow */
#define DUCK_COLOR_TANGENT_1	Gdk::Color("#ffff00") // yellow
/** DUCK_COLOR_TANGENT_2 : red (not used) */
#define DUCK_COLOR_TANGENT_2	Gdk::Color("#ff0000") // red
/** DUCK_COLOR_SKEW : red */
#define DUCK_COLOR_SKEW         Gdk::Color("#ff0000") // red
/** DUCK_COLOR_VERTEX : orange */
#define DUCK_COLOR_VERTEX		Gdk::Color("#ff7f00") // orange
/** DUCK_COLOR_WIDTH : magenta */
#define DUCK_COLOR_WIDTH		Gdk::Color("#ff00ff") // magenta
/** DUCK_COLOR_WIDTHPOINT_POSITION : purple */
#define DUCK_COLOR_WIDTHPOINT_POSITION	Gdk::Color("#d3afff") // purple
/** DUCK_COLOR_OTHER : green */
#define DUCK_COLOR_OTHER		Gdk::Color("#00ff00") // green
/** DUCK_COLOR_OUTLINE : black , the outline around each duck*/
#define DUCK_COLOR_OUTLINE		Gdk::Color("#000000") // the outline around each duck
/** DUCK_COLOR_BEZIER_1 : black, the 2 colors used to draw bezier curves */
#define DUCK_COLOR_BEZIER_1		Gdk::Color("#000000") // black // the 2 colors used to draw bezier curves
/** DUCK_COLOR_BEZIER_2 : grey , the second colors used to draw bezier curves*/
#define DUCK_COLOR_BEZIER_2		Gdk::Color("#afafaf") // grey
/** DUCK_COLOR_COLOR_BOX_1 : white , the first color used to draw boxes*/
#define DUCK_COLOR_BOX_1		Gdk::Color("#ffffff") // white // the 2 colors used to draw boxes
/** DUCK_COLOR_BOX_2 : black , the second color used to draw boxes*/
#define DUCK_COLOR_BOX_2		Gdk::Color("#000000") // black
/** DUCK_COLOR_SELECTED : red , the color of the box drawn when a valuenode is selected*/
#define DUCK_COLOR_SELECTED		Gdk::Color("#ff0000") // red // the color of the box drawn when a valuenode is selected
/** DUCK_COLOR_CONNECT_INSIDE : the color of the inside of the line connecting a vertex duck to the tangent ducks */
#define DUCK_COLOR_CONNECT_INSIDE	Gdk::Color("#9fefef") // the color of the inside of the line connecting a vertex duck to the tangent ducks
/** DUCK_COLOR_CONNECT_OUTSIDE : black, the color of the outside of the line connecting a vertex duck to the tangent ducks*/
#define DUCK_COLOR_CONNECT_OUTSIDE	Gdk::Color("#000000") // the color of the outside of the line connecting a vertex duck to the tangent ducks
/** DUCK_COLOR_WIDTH_TEXT_1 : black, the color of the text's shadow when hovering over a width duck*/
#define DUCK_COLOR_WIDTH_TEXT_1	Gdk::Color("#000000") // the color of the text's shadow when hovering over a width duck
/** DUCK_COLOR_WIDTH_TEXT_2 : magenta, the color of the text when hovering over a width duck*/
#define DUCK_COLOR_WIDTH_TEXT_2	Gdk::Color("#ff00ff") // the color of the text when hovering over a width duck
/** DUCK_COLOR_TRANSFO_TEXT_1 : black , the color of the text's shadow when hovering over any duck of a transformation widget*/
#define DUCK_COLOR_TRANSFO_TEXT_1 Gdk::Color("#000000") // the color of the text's shadow when hovering over any duck of a transformation widget


/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Renderer_Ducks : public studio::WorkAreaRenderer
{

public:
	~Renderer_Ducks();

	//! \brief	Renderer for the handles (ducks) and associated (tangents, beziers, hover, tooltip ...)
	//! \TODO immense function !! break into parts and clean
	//! \Param[in] drawable
	//! \Param[in] expose_area
	void render_vfunc(const Glib::RefPtr<Gdk::Window>& drawable,const Gdk::Rectangle& expose_area	);

protected:
//	bool get_enabled_vfunc()const;
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
