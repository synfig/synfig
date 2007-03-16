/* === S Y N F I G ========================================================= */
/*!	\file mod_openexr/main.cpp
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
#else
#warning HAVE_CONFIG_H not defined!
#endif

#include <synfig/module.h>
#include "trgt_openexr.h"
#include "mptr_openexr.h"
#endif

/* === E N T R Y P O I N T ================================================= */

MODULE_DESC_BEGIN(mod_openexr)
	MODULE_NAME("OpenEXR Module")
	MODULE_DESCRIPTION("Provides support for the EXR image format.")
	MODULE_AUTHOR("Industrial Light & Magic")
	MODULE_VERSION("1.0.4")
	MODULE_COPYRIGHT("OpenEXR Library is Copyright (c) 2003 Lucas Digital Ltd. LLC.")
MODULE_DESC_END

MODULE_INVENTORY_BEGIN(mod_openexr)
	BEGIN_TARGETS
		TARGET(exr_trgt)
	END_TARGETS
	BEGIN_IMPORTERS
		IMPORTER(exr_mptr)
	END_IMPORTERS
MODULE_INVENTORY_END
