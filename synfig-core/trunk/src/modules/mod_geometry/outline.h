/* === S I N F G =========================================================== */
/*!	\file bline.h
**	\brief Template Header
**
**	$Id: outline.h,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SINFG_OUTLINE_H
#define __SINFG_OUTLINE_H

/* === H E A D E R S ======================================================= */

#include <list>
#include <vector>
#include <sinfg/layer_polygon.h>
#include <sinfg/segment.h>
#include <sinfg/value.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

using namespace sinfg;
using namespace std;
using namespace etl;

class Outline : public sinfg::Layer_Polygon
{
	SINFG_LAYER_MODULE_EXT
private:

	sinfg::ValueBase bline;

	std::vector<sinfg::Segment> segment_list;
	std::vector<sinfg::Real> width_list;
	
	bool round_tip[2];
	
	bool sharp_cusps;
	
	bool loop_;

	sinfg::Real width;

	sinfg::Real expand;

	Real loopyness;
	bool old_version;

	bool needs_sync;

	bool homogeneous_width;

public:

	Outline();

	//! Updates the polygon data to match the parameters.
	void sync();
	
	virtual bool set_param(const String & param, const sinfg::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Vocab get_param_vocab()const;
	virtual void set_time(Context context, Time time)const;
	virtual void set_time(Context context, Time time, Vector pos)const;
	virtual bool set_version(const String &ver){if(ver=="0.1")old_version=true; return true;}
	virtual void reset_version(){old_version=false;}
	
};

/* === E N D =============================================================== */

#endif
