/* === S I N F G =========================================================== */
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

#ifndef __SINFG_TIMELOOP_H
#define __SINFG_TIMELOOP_H

/* === H E A D E R S ======================================================= */

#include <sinfg/layer.h>
#include <sinfg/color.h>
#include <sinfg/time.h>
#include <sinfg/context.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class Layer_TimeLoop : public sinfg::Layer
{
	SINFG_LAYER_MODULE_EXT

private:
	sinfg::Time	start_time;
	sinfg::Time	end_time;

protected:
	Layer_TimeLoop();

public:
	~Layer_TimeLoop();
	
	virtual bool set_param(const sinfg::String & param, const sinfg::ValueBase &value);

	virtual sinfg::ValueBase get_param(const sinfg::String & param)const;	

	virtual Vocab get_param_vocab()const;
	virtual sinfg::Color get_color(sinfg::Context context, const sinfg::Point &pos)const;

	virtual void set_time(sinfg::Context context, sinfg::Time time)const;
	virtual bool accelerated_render(sinfg::Context context,sinfg::Surface *surface,int quality, const sinfg::RendDesc &renddesc, sinfg::ProgressCallback *cb)const;
};

/* === E N D =============================================================== */

#endif
