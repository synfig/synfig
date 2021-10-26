/* === S Y N F I G ========================================================= */
/*!	\file mod_mng/main.cpp
**	\brief MNG plugin
**
**	\legal
**	Copyright (c) 2007 Paul Wise
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

#include <synfig/general.h>

#include <synfig/module.h>
#include "trgt_mng.h"
/*	FIXME: Implement MNG import
#include "mptr_mng.h"
*/
#endif

/* === E N T R Y P O I N T ================================================= */

MODULE_DESC_BEGIN(mod_mng)
	MODULE_NAME("MNG Module (libmng)")
/*	FIXME: Implement MNG import
	MODULE_DESCRIPTION("Provides an MNG target and importer")
*/
	MODULE_DESCRIPTION("Provides an MNG target")
	MODULE_AUTHOR("Paul Wise")
	MODULE_VERSION("1.0")
	MODULE_COPYRIGHT(SYNFIG_COPYRIGHT)
MODULE_DESC_END

MODULE_INVENTORY_BEGIN(mod_mng)
	BEGIN_TARGETS
		TARGET(mng_trgt)
	END_TARGETS
/* FIXME: implement MNG import
	BEGIN_IMPORTERS
		IMPORTER(mng_mptr)
	END_IMPORTERS
*/
MODULE_INVENTORY_END
