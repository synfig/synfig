/* === S I N F G =========================================================== */
/*!	\file timepointsmove.h
**	\brief Move the Time Points Header
**
**	$Id: timepointsmove.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SINFG_APP_ACTION_TIMEPOINTSMOVE_H
#define __SINFG_APP_ACTION_TIMEPOINTSMOVE_H

/* === H E A D E R S ======================================================= */

#include <sinfgapp/action.h>
#include <sinfg/time.h>
#include <sinfg/layer.h>
#include <sinfg/canvas.h>
#include <sinfgapp/value_desc.h>

#include <vector>
#include <set>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfgapp {

class Instance;

namespace Action {

class TimepointsMove :
	public Super
{
private:
	
	//process all the value descriptions that are selected (or are in subselections)
	std::vector<sinfg::Layer::Handle>	sel_layers;
	std::vector<sinfg::Canvas::Handle>	sel_canvases;
	std::vector<sinfgapp::ValueDesc>	sel_values;
	std::set<sinfg::Time>				sel_times;
	
	sinfg::Time							timemove;

public:

	TimepointsMove();

	static ParamVocab get_param_vocab();
	static bool is_canidate(const ParamList &x);

	virtual bool set_param(const sinfg::String& name, const Param &);
	virtual bool is_ready()const;

	virtual void prepare();
	virtual void perform();

	ACTION_MODULE_EXT
};

}; // END of namespace action
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
