/* === S I N F G =========================================================== */
/*!	\file main.cpp
**	\brief writeme
**
**	$Id: main.cpp,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#define SINFG_NO_ANGLE

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <sinfg/module.h>
#include <sinfg/layer.h>
#include "trgt_bmp.h"
#include "mptr_bmp.h"

#endif

/* === E N T R Y P O I N T ================================================= */

MODULE_DESC_BEGIN(mod_bmp)
	MODULE_NAME("Microsoft BMP File Format Module")
	MODULE_DESCRIPTION("Provides a Microsoft BMP output target and importer")
	MODULE_AUTHOR("Robert B. Quattlebaum")
	MODULE_VERSION("1.0")
	MODULE_COPYRIGHT(SINFG_COPYRIGHT)
MODULE_DESC_END

MODULE_INVENTORY_BEGIN(mod_bmp)
	BEGIN_TARGETS
		TARGET(bmp)
	END_TARGETS
	BEGIN_IMPORTERS
		IMPORTER(bmp_mptr)
	END_IMPORTERS
MODULE_INVENTORY_END
