/* === S Y N F I G ========================================================= */
/*!	\file layer_mime.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_LAYER_MIME_H
#define __SYNFIG_LAYER_MIME_H

/* === H E A D E R S ======================================================= */

#include <map>

#include <synfig/string.h>

#include "layer_invisible.h"


/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class Layer_Mime
**	The mime layer is a layer that is used when an unknown
**	layer type is requested. This allows people without
**	all of the correct layers installed to still work with
**	that composition.
*/
class Layer_Mime : public Layer_Invisible
{
	std::map<String,ValueBase> param_list;
	String name;
public:
	Layer_Mime(String name);

	virtual String get_version()const;

	virtual bool set_version(const String &ver);

	virtual bool set_param(const String &param, const ValueBase &value);

	virtual ValueBase get_param(const String &param)const;

	virtual Vocab get_param_vocab()const;
	virtual String get_local_name()const;

};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
