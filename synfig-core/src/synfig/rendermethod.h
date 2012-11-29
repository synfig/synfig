/* === S Y N F I G ========================================================= */
/*!	\file rendermethod.h
 **	\brief Enumeration to define the render method used
 **
 **	$Id$
 **
 **	\legal
 **	Copyright (c) 2012 Nikita Kitaev
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

#ifndef __SYNFIG_RENDERMETHOD_H
#define __SYNFIG_RENDERMETHOD_H

/* === H E A D E R S ======================================================= */

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */
namespace synfig
{
//! Available rendering methods
enum RenderMethod {
	SOFTWARE = 0x100,               //!< Software rendering
	OPENGL,                         //!< OpenGL rendering (not supported)
	CAIRO                           //!< Cairo rendering
};
}; // end namespace synfig
/* === E N D =============================================================== */

#endif
