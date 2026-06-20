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

	ValueBase param_smoothness;

	ValueBase param_source_tl;
	ValueBase param_source_br;
	ValueBase param_source_angle;

	ValueBase param_source_points;
	ValueBase param_mesh_mode; // 0 = Grid, 1 = Custom Mesh
	ValueBase param_cull_threshold;

	bool needs_reset_;

	//! Get a control point with boundary clamping for edge/corner cells
	Point get_clamped_ctrl_point(const std::vector<Point>& ctrl_points, int gx, int gy, int cols, int rows) const;

	//! Evaluate a 1D Catmull-Rom spline at parameter t given 4 values
	static Real catmull_rom_1d(Real p0, Real p1, Real p2, Real p3, Real t);

public:
	Layer_FreeFormDeform();
	virtual ~Layer_FreeFormDeform();

	// Custom Mesh Triangulation Helpers
	static std::vector<rendering::Mesh::Triangle> triangulate(const std::vector<Point>& pts);

	//! Remove triangles whose circumradius exceeds the given threshold (Alpha Shape filter).
	//! Used for both canvas overlay display (duckmatic) and preview (state_ffd).
	static std::vector<rendering::Mesh::Triangle> cull_triangles(
		const std::vector<rendering::Mesh::Triangle>& tris,
		const std::vector<Point>& pts,
		Real threshold);

	virtual String get_local_name() const;
	virtual bool set_param(const String & param, const ValueBase & value);
	virtual ValueBase get_param(const String & param) const;
	virtual Vocab get_param_vocab() const;
	virtual void on_canvas_set();

	void prepare_mesh(); // The core math engine
	void regenerate_grid_points(); // Rebuild uniform grid when size changes
	std::vector<Point> get_interpolated_grid(int new_cols, int new_rows) const;
	synfig::Rect get_context_bounds() const;
	std::vector<synfig::Point> compute_grid_for_bounds(const synfig::Rect& bounds, int cols, int rows) const;
};
}
#endif
