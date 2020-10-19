/* === S Y N F I G ========================================================= */
/*!	\file mod_svg/main.cpp
**	\brief writeme
**
**	$Id:$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2009 Carlos A. Sosa Navarro
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
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/localization.h>
#include <synfig/general.h>

#include <synfig/module.h>
#include <synfig/layer.h>
#include <synfig/canvas.h>
#include <synfig/string.h>
#include "layer_svg.h"

#endif

/* === E N T R Y P O I N T ================================================= */

MODULE_DESC_BEGIN(mod_svg)
	MODULE_NAME("SVG Importer")
	MODULE_DESCRIPTION("Provides a svg importer")
	MODULE_AUTHOR("Carlos Sosa Navarro")
	MODULE_VERSION("0.1")
	MODULE_COPYRIGHT(SYNFIG_COPYRIGHT)
MODULE_DESC_END

MODULE_INVENTORY_BEGIN(mod_svg)
	BEGIN_LAYERS
		LAYER(svg_layer)
	END_LAYERS
MODULE_INVENTORY_END
