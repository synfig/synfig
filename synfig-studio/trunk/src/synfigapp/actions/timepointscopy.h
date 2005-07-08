/* === S Y N F I G ========================================================= */
/*!	\file timepointscopy.h
**	\brief Copy the Time Points Header
**
**	$Id: timepointscopy.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
**	\legal
**	Copyright (c) 2004 Adrian Bentley
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

#ifndef __SYNFIG_APP_ACTION_TIMEPOINTSCOPY_H
#define __SYNFIG_APP_ACTION_TIMEPOINTSCOPY_H

/* === H E A D E R S ======================================================= */

#include <synfigapp/action.h>
#include <synfig/time.h>
#include <synfig/layer.h>
#include <synfig/canvas.h>
#include <synfigapp/value_desc.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

class Instance;

namespace Action {

class TimepointsCopy :
	public Super
{
private:
	
	//process all the value descriptions that are selected (or are in subselections)
	std::vector<synfig::Layer::Handle>	sel_layers;
	std::vector<synfig::Canvas::Handle>	sel_canvases;
	std::vector<synfigapp::ValueDesc>	sel_values;
	std::set<synfig::Time>				sel_times;
	
	synfig::Time							timedelta;

public:

	TimepointsCopy();

	static ParamVocab get_param_vocab();
	static bool is_canidate(const ParamList &x);

	virtual bool set_param(const synfig::String& name, const Param &);
	virtual bool is_ready()const;

	virtual void prepare();
	virtual void perform();

	ACTION_MODULE_EXT
};

}; // END of namespace action
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
