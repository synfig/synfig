/* === S Y N F I G ========================================================= */
/*!	\file synfigapp/localization.h
**	\brief Localization
**
**	$Id$
**
**	\legal
**	Copyright (c) 2007 Paul Wise
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

#ifndef __SYNFIGAPP_LOCALIZATION_H
#define __SYNFIGAPP_LOCALIZATION_H

/* === H E A D E R S ======================================================= */

#ifdef ENABLE_NLS
#include <libintl.h>
#endif

/* === M A C R O S ========================================================= */

#ifdef ENABLE_NLS
#define _(x) dgettext("synfigstudio",x)
#define gettext_noop(x) x
#define N_(x) gettext_noop(x)
#else
#define _(x) (x)
#define N_(x) (x)
#define gettext(x) (x)
#endif

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

/* === E N D =============================================================== */

#endif
