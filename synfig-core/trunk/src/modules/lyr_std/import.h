/* === S Y N F I G ========================================================= */
/*!	\file import.h
**	\brief Template Header
**
**	$Id: import.h,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_IMPORT_H
#define __SYNFIG_IMPORT_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer_bitmap.h>
#include <synfig/color.h>
#include <synfig/vector.h>
#include <synfig/importer.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class Import : public synfig::Layer_Bitmap
{
	SYNFIG_LAYER_MODULE_EXT

private:
	synfig::String filename;
	synfig::String abs_filename;
	synfig::Importer::Handle importer;
	synfig::Time time_offset;

protected:
	Import();

public:
	~Import();

	virtual bool set_param(const synfig::String & param, const synfig::ValueBase &value);

	virtual synfig::ValueBase get_param(const synfig::String & param)const;

	virtual Vocab get_param_vocab()const;

	virtual void on_canvas_set();

	virtual void set_time(synfig::Context context, synfig::Time time)const;

	virtual void set_time(synfig::Context context, synfig::Time time, const synfig::Point &point)const;
};

/* === E N D =============================================================== */

#endif
