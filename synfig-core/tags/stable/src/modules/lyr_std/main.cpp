/*! ========================================================================
** Sinfg
** bleh
** $Id: main.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
** This software and associated documentation
** are CONFIDENTIAL and PROPRIETARY property of
** the above-mentioned copyright holder.
**
** You may not copy, print, publish, or in any
** other way distribute this software without
** a prior written agreement with
** the copyright holder.
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#define SINFG_MODULE

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <sinfg/module.h>
#include <sinfg/string.h>

#include "zoom.h"
//#include "blur.h"
#include "import.h"
#include "translate.h"
#include "rotate.h"
#include "clamp.h"
#include "stretch.h"

//#include "colorcorrect.h"

#include "supersample.h"

#include "mandelbrot.h"
#include "julia.h"
#include "insideout.h"
#include "xorpattern.h"
#include "twirl.h"
#include "sphere_distort.h"



#include "shade.h"
#include "bevel.h"
//#include "halftone2.h"

//#include "radialblur.h"

#include "warp.h"
#include "timeloop.h"

#endif

/* === E N T R Y P O I N T ================================================= */

MODULE_DESC_BEGIN(liblyr_std)
	MODULE_NAME("Standard Layers")
	MODULE_DESCRIPTION("Provides a basic set of standard layers")
	MODULE_AUTHOR("Robert B. Quattlebaum")
	MODULE_VERSION("1.0")
	MODULE_COPYRIGHT(SINFG_COPYRIGHT)
MODULE_DESC_END

MODULE_INVENTORY_BEGIN(liblyr_std)
	BEGIN_LAYERS
		LAYER(Zoom)
//		LAYER(Blur_Layer)
//		LAYER(RadialBlur)
		LAYER(Import)
		LAYER(Translate)
		LAYER(SuperSample)
		LAYER(Rotate)
		LAYER(Warp)

		LAYER_ALIAS(Zoom,"Zoom")
		LAYER_ALIAS(Translate,"Translate")
		LAYER_ALIAS(SuperSample,"SuperSample")
		LAYER_ALIAS(Rotate,"Rotate")
		LAYER_ALIAS(Import,"Import")
//		LAYER_ALIAS(Blur_Layer,"Blur")

//		LAYER(Halftone2)

		LAYER(Julia)
		LAYER(InsideOut)
		LAYER(Mandelbrot)
		LAYER(Layer_Clamp)
		LAYER(Layer_Stretch)
//		LAYER(Layer_ColorCorrect)
//		LAYER(XORPattern)
		LAYER(Twirl)
		LAYER(Layer_Shade)
		LAYER(Layer_Bevel)
		LAYER(Layer_TimeLoop)
		LAYER(Layer_SphereDistort)
	END_LAYERS
MODULE_INVENTORY_END
