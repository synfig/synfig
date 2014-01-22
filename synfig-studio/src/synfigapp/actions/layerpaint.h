/* === S Y N F I G ========================================================= */
/*!	\file layerpaint.h
**	\brief Template File
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

#ifndef __SYNFIG_APP_ACTION_LAYERPAINT_H
#define __SYNFIG_APP_ACTION_LAYERPAINT_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer_bitmap.h>
#include <synfigapp/action.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

class Instance;

namespace Action {

class LayerPaint :
	public Undoable,
	public CanvasSpecific
{
private:
	etl::handle<synfig::Layer_Bitmap> layer;

	synfig::Surface undo_surface;
	synfig::Point undo_tl;
	synfig::Point undo_br;

	synfig::Surface surface;
	synfig::Point tl;
	synfig::Point br;

	bool applied;

public:

	LayerPaint();

	static ParamVocab get_param_vocab();
	static bool is_candidate(const ParamList &x);

	void mark_as_already_applied(
		etl::handle<synfig::Layer_Bitmap> layer,
		const synfig::Surface &undo_surface,
		const synfig::Point &undo_tl,
		const synfig::Point &undo_br );

	virtual bool set_param(const synfig::String& name, const Param &);
	virtual bool is_ready()const;

	virtual void perform();
	virtual void undo();

	ACTION_MODULE_EXT
};

}; // END of namespace action
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
