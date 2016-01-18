/* === S Y N F I G ========================================================= */
/*!	\file mod_noise/main.cpp
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include <synfig/module.h>
#include <synfig/string.h>
#include <synfig/canvas.h>
#include <synfig/valuenode.h>

#include "noise.h"
#include "distort.h"
#include "random_noise.h"
#include "valuenode_random.h"

#endif

/* === E N T R Y P O I N T ================================================= */

MODULE_DESC_BEGIN(libmod_noise)
	MODULE_NAME("Noise")
	MODULE_DESCRIPTION("writeme")
	MODULE_AUTHOR("Robert B. Quattlebaum")
	MODULE_VERSION("1.0")
	MODULE_COPYRIGHT(SYNFIG_COPYRIGHT)
MODULE_DESC_END

MODULE_INVENTORY_BEGIN(libmod_noise)
	BEGIN_LAYERS
		LAYER(Noise)
		LAYER(NoiseDistort)
	END_LAYERS

	BEGIN_VALUENODES
		VALUENODE(synfig::ValueNode_Random, "random", _("Random"), synfig::RELEASE_VERSION_0_61_08) // SVN r907
	END_VALUENODES
MODULE_INVENTORY_END
