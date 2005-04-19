/*! ========================================================================
** Synfig
** bleh
** $Id: main.cpp,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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
#include <synfig/valuenode.h>

#include "noise.h"
#include "distort.h"
#include "random.h"

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
MODULE_INVENTORY_END
