/* === S I N F G =========================================================== */
/*!	\file boolean_curve.h
**	\brief Boolean Curve Header
**
**	$Id: booleancurve.h,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
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

#ifndef __SINFG_BOOLEAN_CURVE_H
#define __SINFG_BOOLEAN_CURVE_H

/* === H E A D E R S ======================================================= */
#include <sinfg/layer_shape.h>
#include <sinfg/blinepoint.h>

#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */
namespace sinfg
{
	
class BooleanCurve : public Layer_Shape
{
	//dynamic list of regions and such
	typedef std::vector< std::vector<BLinePoint> >	region_list_type;
	region_list_type	regions;
	
	enum BOOLEAN_OP
	{
		Union = 0,
		Intersection,
		MutualExclude,
		Num_Boolean_Ops
	};
	
	int operation;
	
public:

	BooleanCurve();
	~BooleanCurve();
		
	virtual bool set_param(const String & param, const sinfg::ValueBase &value);
	virtual ValueBase get_param(const String & param)const;
	
	virtual Vocab get_param_vocab()const;

	virtual Color get_color(Context context, const Point &pos)const;
	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;		
};

} //end of namespace sinfg
/* === E N D =============================================================== */

#endif
