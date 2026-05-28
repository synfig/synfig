/* === S Y N F I G ========================================================= */
/*!	\file layer_freeformdeform.cpp
**	\brief Free-Form Deformation layer implementation
*/
/* ========================================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "layer_freeformdeform.h"

#include <synfig/localization.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/value.h>

#include <vector>

#endif

using namespace synfig;

SYNFIG_LAYER_INIT(Layer_FreeFormDeform);
SYNFIG_LAYER_SET_NAME(Layer_FreeFormDeform,"free_form_deform");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_FreeFormDeform,N_("Free-Form Deformation"));
SYNFIG_LAYER_SET_CATEGORY(Layer_FreeFormDeform,N_("Distortions"));
SYNFIG_LAYER_SET_VERSION(Layer_FreeFormDeform,"0.1");

Layer_FreeFormDeform::Layer_FreeFormDeform():
	Layer_MeshTransform(1.0, Color::BLEND_STRAIGHT),
	param_grid_size_x(3),
	param_grid_size_y(3),
	param_smoothness(Real(1.0)) // Default to full Catmull-Rom
{
	std::vector<ValueBase> grid_points;
	// 3x3 grid centered
	for (int y = 0; y < 3; ++y) {
		for (int x = 0; x < 3; ++x) {
			grid_points.push_back(ValueBase(Point(-2.0 + x * 2.0, 2.0 - y * 2.0)));
		}
	}
	param_grid_points.set_list_of(grid_points);
	
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

Layer_FreeFormDeform::~Layer_FreeFormDeform()
{
}

String
Layer_FreeFormDeform::get_local_name()const
{
	String s = Layer_MeshTransform::get_local_name();
	return s.empty() ? _("Free-Form Deformation") : '[' + s + ']';
}

bool
Layer_FreeFormDeform::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE_PLUS(param_grid_points, prepare_mesh());
	IMPORT_VALUE_PLUS(param_grid_size_x, { regenerate_grid_points(); prepare_mesh(); });
	IMPORT_VALUE_PLUS(param_grid_size_y, { regenerate_grid_points(); prepare_mesh(); });
	IMPORT_VALUE_PLUS(param_smoothness, prepare_mesh());
	return Layer_MeshTransform::set_param(param,value);
}

ValueBase
Layer_FreeFormDeform::get_param(const String& param)const
{
	EXPORT_VALUE(param_grid_points);
	EXPORT_VALUE(param_grid_size_x);
	EXPORT_VALUE(param_grid_size_y);
	EXPORT_VALUE(param_smoothness);
	
	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_MeshTransform::get_param(param);
}

Layer::Vocab
Layer_FreeFormDeform::get_param_vocab()const
{
	Layer::Vocab ret(Layer_MeshTransform::get_param_vocab());

	ret.push_back(ParamDesc("grid_points")
		.set_local_name(_("Grid Points"))
		.set_description(_("List of grid control points"))
	);

	ret.push_back(ParamDesc("grid_size_x")
		.set_local_name(_("Grid Size X"))
		.set_description(_("Grid columns"))
	);

	ret.push_back(ParamDesc("grid_size_y")
		.set_local_name(_("Grid Size Y"))
		.set_description(_("Grid rows"))
	);

	ret.push_back(ParamDesc("smoothness")
		.set_local_name(_("Smoothness"))
		.set_description(_("0.0 = Bilinear, 1.0 = Catmull-Rom Spline (blended)"))
	);

	return ret;
}

/* ---- Catmull-Rom helper methods ---- */

Point
Layer_FreeFormDeform::get_clamped_ctrl_point(
	const std::vector<Point>& ctrl_points,
	int gx, int gy, int cols, int rows) const
{
	// Clamp indices to grid boundaries (repeat-edge strategy)
	if (gx < 0) gx = 0;
	if (gx >= cols) gx = cols - 1;
	if (gy < 0) gy = 0;
	if (gy >= rows) gy = rows - 1;
	return ctrl_points[gy * cols + gx];
}

Real
Layer_FreeFormDeform::catmull_rom_1d(Real p0, Real p1, Real p2, Real p3, Real t)
{
	// Standard Catmull-Rom basis:
	// f(t) = 0.5 * [ (-t^3+2t^2-t)*P0 + (3t^3-5t^2+2)*P1
	//               + (-3t^3+4t^2+t)*P2 + (t^3-t^2)*P3 ]
	Real t2 = t * t;
	Real t3 = t2 * t;
	return 0.5 * (
		(-t3 + 2.0*t2 - t)       * p0 +
		( 3.0*t3 - 5.0*t2 + 2.0) * p1 +
		(-3.0*t3 + 4.0*t2 + t)   * p2 +
		( t3 - t2)               * p3
	);
}

/* ---- Grid regeneration ---- */

void Layer_FreeFormDeform::regenerate_grid_points()
{
	int cols = param_grid_size_x.get(int());
	int rows = param_grid_size_y.get(int());
	if (cols < 2 || rows < 2) return;

	std::vector<ValueBase> grid_points;
	for (int y = 0; y < rows; ++y) {
		for (int x = 0; x < cols; ++x) {
			Real px = -2.0 + x * 4.0 / (cols - 1);
			Real py =  2.0 - y * 4.0 / (rows - 1);
			grid_points.push_back(ValueBase(Point(px, py)));
		}
	}
	param_grid_points.set_list_of(grid_points);
}

/* ---- Core mesh preparation ---- */

void Layer_FreeFormDeform::prepare_mesh()
{
	rendering::Mesh::Handle mesh(new rendering::Mesh());

	int cols = param_grid_size_x.get(int());
	int rows = param_grid_size_y.get(int());
	Real smoothness = param_smoothness.get(Real());
	// Clamp smoothness to [0.0, 1.0]
	if (smoothness < 0.0) smoothness = 0.0;
	if (smoothness > 1.0) smoothness = 1.0;

	if (cols < 2 || rows < 2) return;

	std::vector<Point> ctrl_points;
	const ValueBase::List &points_list = param_grid_points.get_list();
	for(ValueBase::List::const_iterator i = points_list.begin(); i != points_list.end(); ++i) {
		if (i->can_get(Point())) {
			ctrl_points.push_back(i->get(Point()));
		}
	}

	if (ctrl_points.size() < (size_t)(cols * rows)) return;

	// Calculate un-deformed bounds based on the assumption it was initially a uniform grid
	// spanning [-2, 2] x [2, -2] (as initialized)
	Real min_x = -2.0, max_x = 2.0;
	Real min_y = 2.0, max_y = -2.0;
	Real cell_w = (max_x - min_x) / (cols - 1);
	Real cell_h = (max_y - min_y) / (rows - 1);

	int sub_steps = 10;

	// Calculate vertices for each cell
	for (int y = 0; y < rows - 1; ++y) {
		for (int x = 0; x < cols - 1; ++x) {

			// Subdivide this cell into a dense mesh for rendering
			for (int v_step = 0; v_step <= sub_steps; ++v_step) {
				for (int u_step = 0; u_step <= sub_steps; ++u_step) {
					Real u = (Real)u_step / sub_steps;
					Real v = (Real)v_step / sub_steps;

					// === BILINEAR INTERPOLATION ===
					Point P00 = ctrl_points[y * cols + x];
					Point P10 = ctrl_points[y * cols + (x + 1)];
					Point P01 = ctrl_points[(y + 1) * cols + x];
					Point P11 = ctrl_points[(y + 1) * cols + (x + 1)];

					Point bilinear_pos =
						P00 * ((1 - u)*(1 - v)) +
						P10 * (u*(1 - v)) +
						P01 * ((1 - u)*v) +
						P11 * (u*v);

					Point calculated_pos;

					if (smoothness > 0.0) {
						// === CATMULL-ROM SPLINE INTERPOLATION ===
						// Tensor-product: interpolate 4 rows along u,
						// then interpolate the 4 results along v.

						Real row_x[4], row_y[4];
						for (int ky = -1; ky <= 2; ++ky) {
							Point cp0 = get_clamped_ctrl_point(ctrl_points, x - 1, y + ky, cols, rows);
							Point cp1 = get_clamped_ctrl_point(ctrl_points, x,     y + ky, cols, rows);
							Point cp2 = get_clamped_ctrl_point(ctrl_points, x + 1, y + ky, cols, rows);
							Point cp3 = get_clamped_ctrl_point(ctrl_points, x + 2, y + ky, cols, rows);

							row_x[ky + 1] = catmull_rom_1d(cp0[0], cp1[0], cp2[0], cp3[0], u);
							row_y[ky + 1] = catmull_rom_1d(cp0[1], cp1[1], cp2[1], cp3[1], u);
						}

						Point catmull_pos = Point(
							catmull_rom_1d(row_x[0], row_x[1], row_x[2], row_x[3], v),
							catmull_rom_1d(row_y[0], row_y[1], row_y[2], row_y[3], v)
						);

						// Blend between bilinear and Catmull-Rom using smoothness
						calculated_pos = bilinear_pos * (1.0 - smoothness) + catmull_pos * smoothness;
					} else {
						calculated_pos = bilinear_pos;
					}

					// Original undeformed position (always bilinear — it's a uniform grid)
					Point P00_orig = Point(min_x + x * cell_w, min_y + y * cell_h);
					Point P10_orig = Point(min_x + (x + 1) * cell_w, min_y + y * cell_h);
					Point P01_orig = Point(min_x + x * cell_w, min_y + (y + 1) * cell_h);
					Point P11_orig = Point(min_x + (x + 1) * cell_w, min_y + (y + 1) * cell_h);

					Point initial_pos =
						P00_orig * ((1 - u)*(1 - v)) +
						P10_orig * (u*(1 - v)) +
						P01_orig * ((1 - u)*v) +
						P11_orig * (u*v);

					mesh->vertices.push_back(rendering::Mesh::Vertex(calculated_pos, initial_pos));
				}
			}

			// Add triangles for this cell
			int base_idx = mesh->vertices.size() - (sub_steps + 1) * (sub_steps + 1);
			for (int v_step = 0; v_step < sub_steps; ++v_step) {
				for (int u_step = 0; u_step < sub_steps; ++u_step) {
					int v0 = base_idx + v_step * (sub_steps + 1) + u_step;
					int v1 = base_idx + v_step * (sub_steps + 1) + (u_step + 1);
					int v2 = base_idx + (v_step + 1) * (sub_steps + 1) + u_step;
					int v3 = base_idx + (v_step + 1) * (sub_steps + 1) + (u_step + 1);

					mesh->triangles.push_back(rendering::Mesh::Triangle(v0, v1, v2));
					mesh->triangles.push_back(rendering::Mesh::Triangle(v1, v3, v2));
				}
			}
		}
	}

	// Create bounding masking contour for the FFD based on boundary points
	rendering::Contour::Handle mask(new rendering::Contour());
	mask->antialias = true;
	mask->move_to(ctrl_points[0]);
	for (int x = 1; x < cols; ++x) mask->line_to(ctrl_points[x]);
	for (int y = 1; y < rows; ++y) mask->line_to(ctrl_points[y * cols + (cols - 1)]);
	for (int x = cols - 2; x >= 0; --x) mask->line_to(ctrl_points[(rows - 1) * cols + x]);
	for (int y = rows - 2; y > 0; --y) mask->line_to(ctrl_points[y * cols]);
	mask->close();
	this->mask = mask;

	this->mesh = mesh;
}
