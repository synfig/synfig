/*! ========================================================================
** Synfig
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

#define SYNFIG_MODULE

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/module.h>
#include <synfig/string.h>
#include <synfig/canvas.h>

#include "checkerboard.h"
#include "circle.h"
#include "region.h"
#include "outline.h"
#include "star.h"
#include "rectangle.h"

#endif

/* === E N T R Y P O I N T ================================================= */

MODULE_DESC_BEGIN(libmod_geometry)
	MODULE_NAME("Geometry")
	MODULE_DESCRIPTION("writeme")
	MODULE_AUTHOR("Robert B. Quattlebaum")
	MODULE_VERSION("1.0")
	MODULE_COPYRIGHT(SYNFIG_COPYRIGHT)
MODULE_DESC_END

MODULE_INVENTORY_BEGIN(libmod_geometry)
	BEGIN_LAYERS
		LAYER(CheckerBoard)
		LAYER(Circle)
		LAYER(Region)
		LAYER(Outline)
		LAYER(Star)
		LAYER(Rectangle)

		LAYER_ALIAS(Outline,"BLine")
		LAYER_ALIAS(Outline,"Bezier")
		LAYER_ALIAS(Region,"Region")
		LAYER_ALIAS(Circle,"Circle")
		LAYER_ALIAS(CheckerBoard,"CheckerBoard")

	END_LAYERS
MODULE_INVENTORY_END
