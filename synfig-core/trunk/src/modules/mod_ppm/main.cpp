/*! ========================================================================
** Sinfg
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

#define SINFG_MODULE

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <sinfg/module.h>
#include "trgt_ppm.h"
#include "mptr_ppm.h"
#endif

/* === E N T R Y P O I N T ================================================= */

MODULE_DESC_BEGIN(mod_ppm)
	MODULE_NAME("PPM Target")
	MODULE_DESCRIPTION("Provides a PPM target")
	MODULE_AUTHOR("Robert B. Quattlebaum")
	MODULE_VERSION("1.0")
	MODULE_COPYRIGHT(SINFG_COPYRIGHT)
MODULE_DESC_END

MODULE_INVENTORY_BEGIN(mod_ppm)
	BEGIN_TARGETS
		TARGET(ppm)
	END_TARGETS
	BEGIN_IMPORTERS
		IMPORTER(ppm_mptr)
	END_IMPORTERS
MODULE_INVENTORY_END
