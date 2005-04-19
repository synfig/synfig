/* === S Y N F I G ========================================================= */
/*!	\file timeloop.h
**	\brief Template Header
**
**	$Id: timeloop.h,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
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

#ifndef __SYNFIG_TIMELOOP_H
#define __SYNFIG_TIMELOOP_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>
#include <synfig/color.h>
#include <synfig/time.h>
#include <synfig/context.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class Layer_TimeLoop : public synfig::Layer
{
	SYNFIG_LAYER_MODULE_EXT

private:
	synfig::Time	start_time;
	synfig::Time	end_time;

protected:
	Layer_TimeLoop();

public:
	~Layer_TimeLoop();
	
	virtual bool set_param(const synfig::String & param, const synfig::ValueBase &value);

	virtual synfig::ValueBase get_param(const synfig::String & param)const;	

	virtual Vocab get_param_vocab()const;
	virtual synfig::Color get_color(synfig::Context context, const synfig::Point &pos)const;

	virtual void set_time(synfig::Context context, synfig::Time time)const;
	virtual bool accelerated_render(synfig::Context context,synfig::Surface *surface,int quality, const synfig::RendDesc &renddesc, synfig::ProgressCallback *cb)const;
};

/* === E N D =============================================================== */

#endif
