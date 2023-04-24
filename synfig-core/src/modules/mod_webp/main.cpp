/* === S Y N F I G ========================================================= */
/*!	\file mod_webp/main.cpp
**	\brief Entry Point for WEBP (via FFMPEG) module (mod_webp)
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2022 BobSynfig
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

#include <synfig/module.h>
#include "mptr_webp.h"
#include "trgt_webp.h"
#endif

/* === E N T R Y P O I N T ================================================= */

MODULE_DESC_BEGIN(mod_webp)
	MODULE_NAME("WEBP (via FFMPEG) Module")
	MODULE_DESCRIPTION("Provides output targets for format WEBP (via FFMPEG)")
	MODULE_AUTHOR("BobSynfig")
	MODULE_VERSION("1.0")
	MODULE_COPYRIGHT(SYNFIG_COPYRIGHT)
MODULE_DESC_END

MODULE_INVENTORY_BEGIN(mod_webp)
	BEGIN_TARGETS
		TARGET(webp_trgt)
		TARGET_EXT(webp_trgt, "webp")
	END_TARGETS
	BEGIN_IMPORTERS
		IMPORTER_EXT(webp_mptr, "webp")
	END_IMPORTERS
MODULE_INVENTORY_END
