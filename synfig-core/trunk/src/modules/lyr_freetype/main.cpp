/*! ========================================================================
** Synfig
** bleh
** $Id: main.cpp,v 1.1.1.1 2005/01/04 01:23:09 darco Exp $
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

#include <string.h>
#include <synfig/module.h>
#include "lyr_freetype.h"
#include <iostream>
#include <ETL/stringf>

#include <ft2build.h>
#include FT_FREETYPE_H

#endif

using namespace etl;
using namespace std;
using namespace synfig;

FT_Library  ft_library;

/* === E N T R Y P O I N T ================================================= */

bool freetype_constructor(synfig::ProgressCallback *cb)
{
	int error;
	if(cb)cb->task("Initializing FreeType...");
	if ( (error = FT_Init_FreeType( &ft_library )) )
	{
		if(cb)cb->error(strprintf("lyr_freetype: FreeType initialization failed. (err=%d)",error));
		return false;
	}
	return true;
}

void freetype_destructor()
{
	std::cerr<<"freetype_destructor()"<<std::endl;
}

/* === E N T R Y P O I N T ================================================= */

MODULE_DESC_BEGIN(liblyr_freetype)
	MODULE_NAME("FreeType Font Layer")
	MODULE_DESCRIPTION("Provides a font rendering layer via FreeType")
	MODULE_AUTHOR("Robert B. Quattlebaum")
	MODULE_VERSION("1.0")
	MODULE_COPYRIGHT(SYNFIG_COPYRIGHT)

	MODULE_CONSTRUCTOR(freetype_constructor)
	MODULE_DESTRUCTOR(freetype_destructor)
MODULE_DESC_END

MODULE_INVENTORY_BEGIN(liblyr_freetype)
	BEGIN_LAYERS
		LAYER(lyr_freetype)
		LAYER_ALIAS(lyr_freetype,"Text")
	END_LAYERS
MODULE_INVENTORY_END
