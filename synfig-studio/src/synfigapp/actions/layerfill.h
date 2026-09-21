/* === S Y N F I G ========================================================= */
/*!	\file layerfill.h
**	\brief Raster fill action for StateFill
**
**	\legal
**	Copyright (c) 2026 Synfig Contributors
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef SYNFIG_APP_ACTION_LAYERFILL_H
#define SYNFIG_APP_ACTION_LAYERFILL_H

/* === H E A D E R S ======================================================= */

#include <synfig/layers/layer_bitmap.h>
#include <synfigapp/action.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

namespace Action {

class BitmapLayerFill :
	public Undoable,
	public CanvasSpecific
{
public:
	typedef etl::handle<BitmapLayerFill> Handle;
	typedef etl::loose_handle<BitmapLayerFill> LooseHandle;

private:
	synfig::GUID id;

	// params
	synfig::Layer_Bitmap::Handle layer;
	synfig::PointInt flood_point;
	synfig::Color color;
	synfig::Real tolerance;
	bool antialiasing;

	// state
	synfig::Surface original_surface;

	/**
	 *  Flood fill @a surface with @color starting from @a flood_point
	 *  @return the bounding box of filled area. {-1. -1} if no borders were found.
	 */
	synfig::RectInt fill(synfig::Surface& surface);

public:
	BitmapLayerFill();

	static ParamVocab get_param_vocab();
	static bool is_candidate(const ParamList& x);

	bool set_param(const synfig::String& name, const Param& param) override;
	bool is_ready() const override;

	void perform() override;
	void undo() override;

	ACTION_MODULE_EXT
};

}; // END of namespace Action
}; // END of namespace synfigapp

/* === E N D =============================================================== */

#endif
