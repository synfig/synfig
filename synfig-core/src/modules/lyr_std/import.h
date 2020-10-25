/* === S Y N F I G ========================================================= */
/*!	\file import.h
**	\brief Header file for implementation of the "Import Image" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2012-2013 Carlos López
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

#ifndef __SYNFIG_IMPORT_H
#define __SYNFIG_IMPORT_H

/* === H E A D E R S ======================================================= */

#include <synfig/layers/layer_bitmap.h>
#include <synfig/color.h>
#include <synfig/vector.h>
#include <synfig/importer.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace modules
{
namespace lyr_std
{

class Import : public Layer_Bitmap
{
	SYNFIG_LAYER_MODULE_EXT

private:
	//!Parameter: (String)
	ValueBase param_filename;
	//!Parameter: (Time)
	ValueBase param_time_offset;

	String independent_filename;
	Importer::Handle importer;

protected:
	Import();

public:
	~Import();

	virtual bool set_param(const String & param, const ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Vocab get_param_vocab()const;

	virtual void on_canvas_set();

	virtual void set_time_vfunc(IndependentContext context, Time time)const;
	virtual void load_resources_vfunc(IndependentContext context, Time time)const;
};

}; // END of namespace lyr_std
}; // END of namespace modules
}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
