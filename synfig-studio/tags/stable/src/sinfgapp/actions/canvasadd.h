/* === S I N F G =========================================================== */
/*!	\file canvasadd.h
**	\brief Template File
**
**	$Id: canvasadd.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SINFG_APP_ACTION_CANVASADD_H
#define __SINFG_APP_ACTION_CANVASADD_H

/* === H E A D E R S ======================================================= */

#include <sinfg/renddesc.h>
#include <sinfgapp/action.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfgapp {

class Instance;

namespace Action {

class CanvasAdd :
	public Undoable,
	public CanvasSpecific
{
private:

	sinfg::Canvas::Handle new_canvas;
	sinfg::String id;

	sinfg::Canvas::Handle inline_parent;

public:

	CanvasAdd();

	static ParamVocab get_param_vocab();
	static bool is_canidate(const ParamList &x);

	virtual bool set_param(const sinfg::String& name, const Param &);
	virtual bool is_ready()const;

	virtual void perform();
	virtual void undo();

	ACTION_MODULE_EXT
};

}; // END of namespace action
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
