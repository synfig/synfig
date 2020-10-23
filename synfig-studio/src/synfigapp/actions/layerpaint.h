/* === S Y N F I G ========================================================= */
/*!	\file layerpaint.h
**	\brief Template File
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

#ifndef __SYNFIG_APP_ACTION_LAYERPAINT_H
#define __SYNFIG_APP_ACTION_LAYERPAINT_H

/* === H E A D E R S ======================================================= */

#include <synfig/guid.h>
#include <synfig/layers/layer_bitmap.h>

#include <synfigapp/action.h>

#include <brushlib.h>

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
public:
	struct PaintPoint {
		float x, y, pressure;
		double dtime;
		PaintPoint(): x(0), y(0), pressure(0), dtime(0) { }
		PaintPoint(float x, float y, float pressure, double dtime):
			x(x), y(y), pressure(pressure), dtime(dtime) { }
	};

	class PaintStroke {
	private:
		static PaintStroke *first, *last;

		PaintStroke *prev, *next;
		PaintStroke *prevSameLayer, *nextSameLayer;

		etl::handle<synfig::Layer_Bitmap> layer;
		brushlib::Brush brush_;

		synfig::Surface surface;
		synfig::Point tl;
		synfig::Point br;

		synfig::Point new_tl;
		synfig::Point new_br;

		std::vector<PaintPoint> points;
		bool prepared;
		bool applied;

		void paint_prev(synfig::Surface &surface);
		void paint_self(synfig::Surface &surface);
		void reset(const PaintPoint &point);

	public:
		PaintStroke();
		~PaintStroke();

		void set_layer(etl::handle<synfig::Layer_Bitmap> layer) { assert(!prepared); this->layer = layer; }
		etl::handle<synfig::Layer_Bitmap> get_layer() const { return layer; }

		brushlib::Brush &brush() { assert(!prepared); return brush_; }
		const brushlib::Brush &get_brush() const { return brush_; }

		bool is_prepared() const { return prepared; }

		void prepare();
		void undo();
		void apply();
		void add_point_and_apply(const PaintPoint &point);
	};

private:
	synfig::GUID id;
	bool applied;

public:
	PaintStroke stroke;

	LayerPaint();

	static ParamVocab get_param_vocab();
	static bool is_candidate(const ParamList &x);

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
