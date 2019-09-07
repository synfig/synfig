/* === S Y N F I G ========================================================= */
/*!	\file outline.h
**	\brief Header file for implementation of the "Advanced Outline" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2011-2013 Carlos López
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

#ifndef __SYNFIG_ADVANCED_OUTLINE_H
#define __SYNFIG_ADVANCED_OUTLINE_H

/* === H E A D E R S ======================================================= */

#include <synfig/layers/layer_shape.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class Advanced_Outline : public synfig::Layer_Shape
{
	SYNFIG_LAYER_MODULE_EXT
private:
	//! Parameter: type list of BlinePoint
	synfig::ValueBase param_bline;
	//! Parameter: type list of WidthPoint
	synfig::ValueBase param_wplist;
	//! Parameter: type list of DashItem
	synfig::ValueBase param_dilist;
	//! Parameter: (WidthPoint::SideType)
	synfig::ValueBase param_start_tip;
	//! Parameter: (WidthPoint::SideType)
	synfig::ValueBase param_end_tip;
	//! Parameter: (int)
	synfig::ValueBase param_cusp_type;
	//! Parameter: (Real)
	synfig::ValueBase param_width;
	//! Parameter: (Real)
	synfig::ValueBase param_expand;
	//! Parameter: (Real)
	synfig::ValueBase param_smoothness;
	//! Parameter: (bool)
	synfig::ValueBase param_homogeneous;
	//! Parameter: (Real)
	synfig::ValueBase param_dash_offset;
	//! Parameter: (bool)
	synfig::ValueBase param_dash_enabled;

public:
	enum CuspType
	{
		TYPE_SHARP     = 0,
		TYPE_ROUNDED   = 1,
		TYPE_BEVEL     = 2
	};

	Advanced_Outline();
	~Advanced_Outline();

	virtual bool set_shape_param(const synfig::String &param, const synfig::ValueBase &value);
	virtual synfig::ValueBase get_param(const synfig::String & param)const;
	virtual Vocab get_param_vocab()const;

	//! Connects the parameter to another Value Node. Implementation for this layer
	virtual bool connect_dynamic_param(const synfig::String& param, etl::loose_handle<synfig::ValueNode> x );

private:
	bool connect_bline_to_wplist(etl::loose_handle<synfig::ValueNode> x);
	bool connect_bline_to_dilist(etl::loose_handle<synfig::ValueNode> x);
	
protected:
	virtual void sync_vfunc();
};

/* === E N D =============================================================== */

#endif
