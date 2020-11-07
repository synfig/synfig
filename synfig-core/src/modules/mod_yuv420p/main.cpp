/* === S Y N F I G ========================================================= */
/*!	\file mod_yuv420p/main.cpp
**	\brief Entry Point for YUV420p module (mod_yuv420p)
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

#include <synfig/general.h>

#include <synfig/module.h>
#include "trgt_yuv.h"
#endif

/* === E N T R Y P O I N T ================================================= */

MODULE_DESC_BEGIN(mod_yuv420p)
	MODULE_NAME("YUV420P Target")
	MODULE_DESCRIPTION("writeme")
	MODULE_AUTHOR("Robert B. Quattlebaum")
	MODULE_VERSION("1.0")
	MODULE_COPYRIGHT(SYNFIG_COPYRIGHT)
MODULE_DESC_END

MODULE_INVENTORY_BEGIN(mod_yuv420p)
	BEGIN_TARGETS
		TARGET(yuv)
	END_TARGETS
MODULE_INVENTORY_END
