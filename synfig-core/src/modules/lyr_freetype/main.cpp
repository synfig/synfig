/* === S Y N F I G ========================================================= */
/*!	\file lyr_freetype/main.cpp
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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
		if(cb)cb->error(strprintf("Layer_Freetype: FreeType initialization failed. (err=%d)",error));
		return false;
	}
	return true;
}

void freetype_destructor()
{
	FT_Done_FreeType(ft_library);
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
		LAYER(Layer_Freetype)
		LAYER_ALIAS(Layer_Freetype,"Text")
	END_LAYERS
MODULE_INVENTORY_END
