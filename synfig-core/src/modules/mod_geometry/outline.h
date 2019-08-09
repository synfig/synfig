/* === S Y N F I G ========================================================= */
/*!	\file outline.h
**	\brief Header file for implementation of the "Outline" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2012-2013 Carlos López
**	......... ... 2018-2019 Ivan Mahonin
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

#ifndef __SYNFIG_OUTLINE_H
#define __SYNFIG_OUTLINE_H

/* === H E A D E R S ======================================================= */

#include <list>
#include <vector>
#include <synfig/layers/layer_shape.h>
#include <synfig/segment.h>
#include <synfig/value.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class Outline : public synfig::Layer_Shape
{
	SYNFIG_LAYER_MODULE_EXT
private:
	//! Parameter: type list of BLinePoints
	synfig::ValueBase param_bline;
	//! Parameter: (bool)
	synfig::ValueBase param_round_tip[2];
	//! Parameter: (bool)
	synfig::ValueBase param_sharp_cusps;
	//! Parameter: (bool)
	synfig::ValueBase param_loop;
	//! Parameter: (Real)
	synfig::ValueBase param_width;
	//! Parameter: (Real)
	synfig::ValueBase param_expand;
	//! Parameter: (bool)
	synfig::ValueBase param_homogeneous_width;

	bool old_version;

public:
	Outline();

	virtual bool set_shape_param(const synfig::String & param, const synfig::ValueBase &value);
	virtual synfig::ValueBase get_param(const synfig::String & param)const;
	virtual Vocab get_param_vocab()const;
	virtual bool set_version(const synfig::String &ver)
		{ if (ver=="0.1") old_version = true; return true; }
	virtual void reset_version()
		{ old_version = false; }

protected:
	virtual void sync_vfunc();
};

/* === E N D =============================================================== */

#endif
