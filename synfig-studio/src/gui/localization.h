/* === S Y N F I G ========================================================= */
/*!	\file gui/localization.h
**	\brief Localization
**
**	\legal
**	Copyright (c) 2007 Paul Wise
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

#ifndef __SYNFIGSTUDIO_LOCALIZATION_H
#define __SYNFIGSTUDIO_LOCALIZATION_H

/* === H E A D E R S ======================================================= */

#ifdef ENABLE_NLS
#include <libintl.h>
#endif

/* === M A C R O S ========================================================= */

#ifdef ENABLE_NLS
#define _(x) gettext(x)
#define gettext_noop(x) x
#define N_(x) gettext_noop(x)
#else
#define _(x) (x)
#define N_(x) (x)
#endif

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

inline const char* synfiggui_localize(const char *x)
	{ return _(x); }

/* === E N D =============================================================== */

#endif
