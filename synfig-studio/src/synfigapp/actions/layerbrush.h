/* === S Y N F I G ========================================================= */
/*!	\file layerbrush.h
**	\brief Brush stroke action for StateBrush2 with live preview
**
**	\legal
**	......... ... 2024
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

#ifndef __SYNFIG_APP_ACTION_LAYERBRUSH_H
#define __SYNFIG_APP_ACTION_LAYERBRUSH_H

/* === H E A D E R S ======================================================= */

#include <synfig/guid.h>
#include <synfig/layers/layer_bitmap.h>
#include <synfig/surface.h>
#include <synfig/rect.h>
#include <synfig/vector.h>

#include <synfigapp/action.h>

#include <brushlib.h>
#include <glibmm/timeval.h>
#include <vector>

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
		synfig::Vector pos;
		synfig::Real pressure;
		Glib::TimeVal timestamp;
		StrokePoint(): pos(0,0), pressure(0.0) { }
		StrokePoint(const synfig::Vector& p, synfig::Real pr, const Glib::TimeVal& t):
			pos(p), pressure(pr), timestamp(t) { }
	};

	class BrushStroke {
	private:
		synfig::Layer_Bitmap::Handle layer;
		brushlib::Brush brush_;

		synfig::Surface original_surface;
		synfig::Point original_tl;
		synfig::Point original_br;

		std::vector<StrokePoint> points;
		bool prepared;
		bool applied;

		void reset_brush(const StrokePoint& point, int surface_w, int surface_h);
		void render_stroke_to_surface(synfig::Surface& surface);

	public:
		BrushStroke();
		~BrushStroke();

		void set_layer(synfig::Layer_Bitmap::Handle layer) { this->layer = layer; }
		synfig::Layer_Bitmap::Handle get_layer() const { return layer; }

		brushlib::Brush& brush() { return brush_; }
		const brushlib::Brush& get_brush() const { return brush_; }

		void add_point(const StrokePoint& point) { points.push_back(point); }
		const std::vector<StrokePoint>& get_points() const { return points; }

		bool is_prepared() const { return prepared; }
		bool is_applied() const { return applied; }

		void prepare();
		void apply();
		void undo();
	};

private:
	synfig::GUID id;
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