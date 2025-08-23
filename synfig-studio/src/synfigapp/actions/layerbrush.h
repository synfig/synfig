/* === S Y N F I G ========================================================= */
/*!	\file layerbrush.h
**	\brief Template File
**
**	\legal
**	Copyright (c) 2025 Abdelhadi Wael
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

#ifndef SYNFIG_APP_ACTION_LAYERBRUSH_H
#define SYNFIG_APP_ACTION_LAYERBRUSH_H

/* === H E A D E R S ======================================================= */

#include <synfig/layers/layer_bitmap.h>
#include <synfigapp/action.h>
#include <brushlib.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

class Instance;

namespace Action {

class LayerBrush :
	public Undoable,
	public CanvasSpecific
{
public:
	struct StrokePoint {
		float x, y;
		synfig::Real pressure;
		double dtime;
		StrokePoint(): x(0), y(0), pressure(0.0) { }
		StrokePoint(float x, float y, synfig::Real pr, double dtime):
			x(x), y(y), pressure(pr), dtime(dtime) { }
	};

	class BrushStroke {
	public:
		enum UndoMode {
			REDRAW,
			CHECKPOINTING,
			SURFACE_SAVING
		};
	private:
		synfig::Layer_Bitmap::Handle layer;
		std::unique_ptr<brushlib::Brush> brush_;

		synfig::Surface original_surface;

		std::vector<StrokePoint> points;
		bool prepared;
		bool applied;
		int stroke_index;
		UndoMode undo_mode;
		void reset_brush(const StrokePoint& point);
		void paint_stroke(synfig::Surface& surface);

	public:
		BrushStroke();
		~BrushStroke();

		void set_layer(synfig::Layer_Bitmap::Handle layer) { this->layer = layer; }
		synfig::Layer_Bitmap::Handle get_layer() const { return layer; }
		void set_undo_mode(UndoMode mode) { undo_mode = mode; }

		brushlib::Brush& brush() { return *brush_; }
		const brushlib::Brush& get_brush() const { return *brush_; }

		void add_point(const StrokePoint& point) { points.push_back(point); }
		const std::vector<StrokePoint>& get_points() const { return points; }

		bool is_prepared() const { return prepared; }
		bool is_applied() const { return applied; }

		void prepare();
		void apply();
		void undo();
	};

private:
	bool applied;

public:
	BrushStroke stroke;

	LayerBrush();

	static ParamVocab get_param_vocab();
	static bool is_candidate(const ParamList& x);

	virtual bool set_param(const synfig::String& name, const Param& param);
	virtual bool is_ready() const;

	virtual void perform();
	virtual void undo();

	ACTION_MODULE_EXT
};

}; // END of namespace Action
}; // END of namespace synfigapp

/* === E N D =============================================================== */

#endif