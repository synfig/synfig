/* === S I N F G =========================================================== */
/*!	\file sinfg.h
**	\brief Primary Header for Sinfg
**
**	$Id: sinfg.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#ifndef __SINFG__
#define __SINFG__

#ifndef __cplusplus
#error Sorry, Sinfg only supports C++ at this time. See README for details.
#endif

#warning The use of this header is deprecated. Please use the individual header files.

/* === M A C R O S ========================================================= */

/*! \def SINFG_LEAN
**	\brief Define this to remove unused features, speeding up compile time.
**
**	Define SINFG_LEAN if your plug-in
**	or program doesn't use the sinfg::Angle class
**	or the rendering subsystem. This can speed up
**	compiles. You may also wish to individualy
**	use the macros SINFG_NO_ANGLE and
**	SINFG_NO_RENDER.
**	\see SINFG_NO_ANGLE, SINFG_NO_RENDER
*/
#ifdef SINFG_LEAN
# ifndef SINFG_NO_ANGLE
#  define SINFG_NO_ANGLE
# endif
# ifndef SINFG_NO_RENDER
#  define SINFG_NO_RENDER
# endif
#endif

/*!	\def SINFG_LAYER
**	The SINFG_LAYER macro is useful for when you
**	are compiling layers, and can help to improve
**	build time.
*/
#ifdef SINFG_LAYER
# ifndef SINFG_NO_RENDER
#  define SINFG_NO_RENDER
# endif
# define SINFG_NO_LOADSAVE
#endif

/*!	\def SINFG_TARGET
**	The SINFG_TARGET macro is useful for when you
**	are compiling render targets, and can help to
**	improve build time.
*/
#ifdef SINFG_TARGET
# ifdef SINFG_NO_RENDER
#  error You defined SINFG_TARGET, but also SINFG_NO_RENDER. This doesnt make sense.
# endif
# ifndef SINFG_NO_ANGLE
#  define SINFG_NO_ANGLE
# endif
# define SINFG_NO_LOADSAVE
#endif

/*!	\def SINFG_MODULE
**	\todo Writeme
*/
#ifdef SINFG_MODULE
# define SINFG_NO_LOADSAVE
#endif

/*! \namespace sinfg
**	\brief Where every function and class of the sinfg library can be found
**	\todo Writeme
*/

/* === H E A D E R S ======================================================= */

#include "version.h"
#include "general.h"
#include "module.h"
#include "color.h"
#include "canvas.h"
#include "layer.h"
#include "vector.h"
#include "types.h"
#include "segment.h"

#ifndef SINFG_NO_RENDER
# include "render.h"
#endif

#ifndef SINFG_LAYER
#include "target.h"
#endif

#include "valuenode.h"
#include "valuenode_subtract.h"
//#include "valuenode_animated.h"
#include "valuenode_composite.h"
#include "valuenode_const.h"
#include "valuenode_linear.h"
#include "valuenode_dynamiclist.h"
#include "valuenode_reference.h"

#ifndef SINFG_NO_LOADSAVE
# include "savecanvas.h"
# include "loadcanvas.h"
#endif

#include "importer.h"
#include "surface.h"

#include "string.h"

/* === C L A S S E S & S T R U C T S ======================================= */

/* === E N D =============================================================== */

#endif
