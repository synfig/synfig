/* === S Y N F I G ========================================================= */
/*!	\file mod_magickpp/main.cpp
**	\brief Magick++ plugin
**
**	$Id$
**
**	\legal
**	Copyright 2007 Chris Moore
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

#include <synfig/module.h>
#include "trgt_magickpp.h"
#endif

/* === E N T R Y P O I N T ================================================= */

MODULE_DESC_BEGIN(mod_magickpp)
	MODULE_NAME("Magick++ Module (libMagick++)")
	MODULE_DESCRIPTION("Provides an animated GIF target")
	MODULE_AUTHOR("Chris Moore")
	MODULE_VERSION("1.0")
	MODULE_COPYRIGHT(SYNFIG_COPYRIGHT)
MODULE_DESC_END

MODULE_INVENTORY_BEGIN(mod_magickpp)
	BEGIN_TARGETS
		TARGET(magickpp_trgt)
	END_TARGETS
MODULE_INVENTORY_END
