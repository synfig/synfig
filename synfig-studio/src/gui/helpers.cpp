/* === S Y N F I G ========================================================= */
/*!	\file helpers.cpp
**	\brief Helpers File
**
**	$Id$
**
**	\legal
**	......... ... 2018 Ivan Mahonin
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <gtk/gtk.h>

#include <synfig/general.h>

#include "helpers.h"

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

static bool
is_old_gtk_adjustment() {
	static bool is_old = gtk_check_version(3, 18, 0) != NULL;
	return is_old;
}

/* === M E T H O D S ======================================================= */

void
ConfigureAdjustment::emit_changed()
	{ if (is_old_gtk_adjustment()) adjustment->changed(); }

void
ConfigureAdjustment::emit_value_changed()
{ if (is_old_gtk_adjustment()) adjustment->value_changed(); }
