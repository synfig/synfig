/* === S Y N F I G ========================================================= */
/*!	\file mod_png/main.cpp
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#define SYNFIG_MODULE

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/localization.h>
#include <synfig/general.h>

#include "png.h"
#include <synfig/module.h>
#include "trgt_png.h"
#include "trgt_png_spritesheet.h"
#include "trgt_cairo_png.h"
#include "mptr_png.h"
#include "mptr_cairo_png.h"
#endif

/* === E N T R Y P O I N T ================================================= */

MODULE_DESC_BEGIN(mod_png)
	MODULE_NAME("PNG Module (libpng)")
	MODULE_DESCRIPTION("Provides a PNG target and importer")
	MODULE_AUTHOR("Robert B. Quattlebaum Jr")
	MODULE_VERSION("1.0")
	MODULE_COPYRIGHT(SYNFIG_COPYRIGHT)
MODULE_DESC_END

MODULE_INVENTORY_BEGIN(mod_png)
	BEGIN_TARGETS
		TARGET(cairo_png_trgt)
		TARGET(png_trgt)
		TARGET(png_trgt_spritesheet)
		TARGET_EXT(png_trgt, "png")
	END_TARGETS
	BEGIN_IMPORTERS
		IMPORTER(png_mptr)
		CAIROIMPORTER(cairo_png_mptr)
	END_IMPORTERS
MODULE_INVENTORY_END
