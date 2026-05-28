#ifndef __SYNFIG_LAYER_FREEFORMDEFORM_H
#define __SYNFIG_LAYER_FREEFORMDEFORM_H

#include "layer_meshtransform.h"

namespace synfig {

class Layer_FreeFormDeform : public Layer_MeshTransform
{
	SYNFIG_LAYER_MODULE_EXT
private:
	//! Parameter : (list of Points) The control lattice
	ValueBase param_grid_points;

	//! Parameter : (Integer) e.g., 3 for a 3x3 grid
	ValueBase param_grid_size_x;
	ValueBase param_grid_size_y;

	//! Parameter : (Real) Smoothness: 0.0=Bilinear, 1.0=Catmull-Rom, blended in between
	ValueBase param_smoothness;

	//! Get a control point with boundary clamping for edge/corner cells
	Point get_clamped_ctrl_point(const std::vector<Point>& ctrl_points, int gx, int gy, int cols, int rows) const;

	//! Evaluate a 1D Catmull-Rom spline at parameter t given 4 values
	static Real catmull_rom_1d(Real p0, Real p1, Real p2, Real p3, Real t);

public:
	Layer_FreeFormDeform();
	virtual ~Layer_FreeFormDeform();

	virtual String get_local_name() const;
	virtual bool set_param(const String & param, const ValueBase & value);
	virtual ValueBase get_param(const String & param) const;
	virtual Vocab get_param_vocab() const;

	void prepare_mesh(); // The core math engine
	void regenerate_grid_points(); // Rebuild uniform grid when size changes
};
}
#endif
