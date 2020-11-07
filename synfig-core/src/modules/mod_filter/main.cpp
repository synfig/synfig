/* === S Y N F I G ========================================================= */
/*!	\file mod_filter/main.cpp
**	\brief Entry Point for Filters module (libmod_filter)
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

#include <synfig/localization.h>
#include <synfig/general.h>

#include <synfig/module.h>
#include <synfig/string.h>
#include <synfig/canvas.h>

#include <synfig/rendering/renderer.h>

#include "blur.h"
#include "colorcorrect.h"
#include "halftone2.h"
#include "halftone3.h"
#include "lumakey.h"
#include "radialblur.h"

#endif

using namespace synfig;
using namespace modules;
using namespace mod_filter;

/* === E N T R Y P O I N T ================================================= */

MODULE_DESC_BEGIN(libmod_filter)
	MODULE_NAME("Filters")
	MODULE_DESCRIPTION("Writeme")
	MODULE_AUTHOR("Robert B. Quattlebaum")
	MODULE_VERSION("1.0")
	MODULE_COPYRIGHT(SYNFIG_COPYRIGHT)
MODULE_DESC_END

MODULE_INVENTORY_BEGIN(libmod_filter)
	BEGIN_LAYERS
		LAYER(Blur_Layer)
		LAYER(Halftone2)
		LAYER(Halftone3)
		LAYER(LumaKey)
		LAYER(RadialBlur)
		LAYER(Layer_ColorCorrect)
	END_LAYERS
MODULE_INVENTORY_END
