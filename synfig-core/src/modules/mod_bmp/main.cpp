/* === S Y N F I G ========================================================= */
/*!	\file mod_bmp/main.cpp
**	\brief writeme
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
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/localization.h>
#include <synfig/general.h>

#include <synfig/module.h>
#include <synfig/layer.h>
#include "trgt_bmp.h"
#include "mptr_bmp.h"

#endif

/* === E N T R Y P O I N T ================================================= */

MODULE_DESC_BEGIN(mod_bmp)
	MODULE_NAME("Microsoft BMP File Format Module")
	MODULE_DESCRIPTION("Provides a Microsoft BMP output target and importer")
	MODULE_AUTHOR("Robert B. Quattlebaum")
	MODULE_VERSION("1.0")
	MODULE_COPYRIGHT(SYNFIG_COPYRIGHT)
MODULE_DESC_END

MODULE_INVENTORY_BEGIN(mod_bmp)
	BEGIN_TARGETS
		TARGET(bmp)
	END_TARGETS
	BEGIN_IMPORTERS
		IMPORTER(bmp_mptr)
	END_IMPORTERS
MODULE_INVENTORY_END
