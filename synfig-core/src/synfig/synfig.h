/* === S Y N F I G ========================================================= */
/*!	\file synfig.h
**	\brief Primary Header for Synfig
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

#ifndef __SYNFIG__
#define __SYNFIG__

#ifndef __cplusplus
#error Sorry, Synfig only supports C++ at this time. See README for details.
#endif

#warning The use of this header is deprecated. Please use the individual header files.

/* === M A C R O S ========================================================= */

#define SYNFIG_LEAN
#undef SYNFIG_LEAN
/*! \def SYNFIG_LEAN
**	\brief Define this to remove unused features, speeding up compile time.
**
**	Define SYNFIG_LEAN if your plug-in
**	or program doesn't use the synfig::Angle class
**	or the rendering subsystem. This can speed up
**	compiles. You may also wish to individually
**	use the macros SYNFIG_NO_ANGLE and
**	SYNFIG_NO_RENDER.
**	\see SYNFIG_NO_ANGLE, SYNFIG_NO_RENDER
*/
#ifdef SYNFIG_LEAN
# ifndef SYNFIG_NO_ANGLE
#  define SYNFIG_NO_ANGLE
# endif
# ifndef SYNFIG_NO_RENDER
#  define SYNFIG_NO_RENDER
# endif
#endif

/*!	\def SYNFIG_LAYER
**	The SYNFIG_LAYER macro is useful for when you
**	are compiling layers, and can help to improve
**	build time.
*/
#ifdef SYNFIG_LAYER
# ifndef SYNFIG_NO_RENDER
#  define SYNFIG_NO_RENDER
# endif
# define SYNFIG_NO_LOADSAVE
#endif

/*!	\def SYNFIG_TARGET
**	The SYNFIG_TARGET macro is useful for when you
**	are compiling render targets, and can help to
**	improve build time.
*/
#ifdef SYNFIG_TARGET
# ifdef SYNFIG_NO_RENDER
#  error You defined SYNFIG_TARGET, but also SYNFIG_NO_RENDER. This does not make sense.
# endif
# ifndef SYNFIG_NO_ANGLE
#  define SYNFIG_NO_ANGLE
# endif
# define SYNFIG_NO_LOADSAVE
#endif

/*!	\def SYNFIG_MODULE
**	\todo Writeme
*/
#ifdef SYNFIG_MODULE
# define SYNFIG_NO_LOADSAVE
#endif

/*! \namespace synfig
**	\brief Where every function and class of the synfig library can be found
**	\todo Writeme
*/

/* === H E A D E R S ======================================================= */

#include "version.h"
#include "module.h"
#include "color.h"
#include "canvas.h"
#include "layer.h"
#include "vector.h"
#include "types.h"
#include "segment.h"

#ifndef SYNFIG_NO_RENDER
# include "render.h"
#endif

#ifndef SYNFIG_LAYER
#include "target.h"
#endif

#include "valuenode.h"
#include "valuenodes/valuenode_subtract.h"
//#include "valuenodes/valuenode_animated.h"
#include "valuenodes/valuenode_composite.h"
#include "valuenodes/valuenode_const.h"
#include "valuenodes/valuenode_linear.h"
#include "valuenodes/valuenode_dynamiclist.h"
#include "valuenodes/valuenode_reference.h"

#ifndef SYNFIG_NO_LOADSAVE
# include "savecanvas.h"
# include "loadcanvas.h"
#endif

#include "importer.h"
#include "surface.h"

#include "string.h"

/* === C L A S S E S & S T R U C T S ======================================= */

/* === E N D =============================================================== */

#endif
