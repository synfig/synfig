/*! ========================================================================
** Sinfg
** bleh
** $Id: main.cpp,v 1.1.1.1 2005/01/04 01:23:11 darco Exp $
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
#include "mptr_imagemagick.h"
#include "trgt_imagemagick.h"
#endif

/* === E N T R Y P O I N T ================================================= */

MODULE_DESC_BEGIN(mod_imagemagick)
	MODULE_NAME("ImageMagick Module")
	MODULE_DESCRIPTION("Provides targets and importers for nearly every format that ImageMagick supports")
	MODULE_AUTHOR("Robert B. Quattlebaum")
	MODULE_VERSION("1.0")
	MODULE_COPYRIGHT(SINFG_COPYRIGHT)
MODULE_DESC_END

MODULE_INVENTORY_BEGIN(mod_imagemagick)
	BEGIN_TARGETS
		TARGET(imagemagick_trgt)
		TARGET_EXT(imagemagick_trgt,"jpg")
		TARGET_EXT(imagemagick_trgt,"jpeg")
		TARGET_EXT(imagemagick_trgt,"png")
		TARGET_EXT(imagemagick_trgt,"tga")
		TARGET_EXT(imagemagick_trgt,"tif")
		TARGET_EXT(imagemagick_trgt,"tiff")
		TARGET_EXT(imagemagick_trgt,"pcx")
		TARGET_EXT(imagemagick_trgt,"ps")
		TARGET_EXT(imagemagick_trgt,"pdf")
		TARGET_EXT(imagemagick_trgt,"pgm")
		TARGET_EXT(imagemagick_trgt,"psd")
		TARGET_EXT(imagemagick_trgt,"xcf")
		TARGET_EXT(imagemagick_trgt,"svg")
		TARGET_EXT(imagemagick_trgt,"xpm")
		TARGET_EXT(imagemagick_trgt,"miff")
		TARGET_EXT(imagemagick_trgt,"eps")
		TARGET_EXT(imagemagick_trgt,"cmyk")
		TARGET_EXT(imagemagick_trgt,"gif")
	END_TARGETS
	BEGIN_IMPORTERS
		IMPORTER_EXT(imagemagick_mptr,"jpg")
		IMPORTER_EXT(imagemagick_mptr,"jpeg")
		IMPORTER_EXT(imagemagick_mptr,"png")
		IMPORTER_EXT(imagemagick_mptr,"bmp")
		IMPORTER_EXT(imagemagick_mptr,"gif")
		IMPORTER_EXT(imagemagick_mptr,"pcx")
		IMPORTER_EXT(imagemagick_mptr,"tif")
		IMPORTER_EXT(imagemagick_mptr,"tiff")
		IMPORTER_EXT(imagemagick_mptr,"tga")
		IMPORTER_EXT(imagemagick_mptr,"ps")
		IMPORTER_EXT(imagemagick_mptr,"pdf")
		IMPORTER_EXT(imagemagick_mptr,"pgm")
		IMPORTER_EXT(imagemagick_mptr,"psd")
		IMPORTER_EXT(imagemagick_mptr,"xcf")
		IMPORTER_EXT(imagemagick_mptr,"svg")
		IMPORTER_EXT(imagemagick_mptr,"tim")
		IMPORTER_EXT(imagemagick_mptr,"xpm")
		IMPORTER_EXT(imagemagick_mptr,"miff")
		IMPORTER_EXT(imagemagick_mptr,"ico")
		IMPORTER_EXT(imagemagick_mptr,"eps")
		IMPORTER_EXT(imagemagick_mptr,"ttf")
		IMPORTER_EXT(imagemagick_mptr,"pix")
		IMPORTER_EXT(imagemagick_mptr,"rla")
		IMPORTER_EXT(imagemagick_mptr,"mat")
		IMPORTER_EXT(imagemagick_mptr,"html")
		IMPORTER_EXT(imagemagick_mptr,"ept")
		IMPORTER_EXT(imagemagick_mptr,"dcm")
		IMPORTER_EXT(imagemagick_mptr,"fig")
	END_IMPORTERS
MODULE_INVENTORY_END
