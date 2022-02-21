/* === S Y N F I G ========================================================= */
/*!	\file lyr_std/main.cpp
**	\brief Entry point for Standard Layers module
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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
#include <synfig/string.h>
#include <synfig/rendering/renderer.h>

#include "zoom.h"
#include "import.h"
#include "translate.h"
#include "rotate.h"
#include "clamp.h"
#include "stretch.h"

#include "supersample.h"

#include "mandelbrot.h"
#include "julia.h"
#include "insideout.h"
#include "xorpattern.h"
#include "twirl.h"
#include "sphere_distort.h"

#include "shade.h"
#include "bevel.h"

#include "perspective.h"
#include "timeloop.h"
#include "curvewarp.h"
#include "stroboscope.h"
#include "freetime.h"

#endif

using namespace synfig;
using namespace modules;
using namespace lyr_std;

/* === E N T R Y P O I N T ================================================= */

MODULE_DESC_BEGIN(liblyr_std)
	MODULE_NAME("Standard Layers")
	MODULE_DESCRIPTION("Provides a basic set of standard layers")
	MODULE_AUTHOR("Robert B. Quattlebaum")
	MODULE_VERSION("1.0")
	MODULE_COPYRIGHT(SYNFIG_COPYRIGHT)
MODULE_DESC_END

MODULE_INVENTORY_BEGIN(liblyr_std)
	BEGIN_LAYERS
		LAYER(Zoom)				LAYER_ALIAS(Zoom,"Zoom")
		LAYER(Import)			LAYER_ALIAS(Import,"Import")
		LAYER(Translate)		LAYER_ALIAS(Translate,"Translate")
		LAYER(SuperSample)		LAYER_ALIAS(SuperSample,"SuperSample")
		LAYER(Rotate)			LAYER_ALIAS(Rotate,"Rotate")
		LAYER(Perspective)
		LAYER(Julia)
		LAYER(InsideOut)
		LAYER(Mandelbrot)
		LAYER(Layer_Clamp)
		LAYER(Layer_Stretch)
		LAYER(XORPattern)		LAYER_ALIAS(XORPattern,"XORPattern")
		LAYER(Twirl)
		LAYER(Layer_Shade)
		LAYER(Layer_Bevel)
		LAYER(Layer_TimeLoop)
		LAYER(Layer_Stroboscope)
		LAYER(Layer_SphereDistort)
		LAYER(CurveWarp)
		LAYER(Layer_FreeTime)
	END_LAYERS
MODULE_INVENTORY_END
