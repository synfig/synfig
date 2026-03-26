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
	param_grid_size_y(3)
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
	IMPORT_VALUE_PLUS(param_grid_size_x, prepare_mesh());
	IMPORT_VALUE_PLUS(param_grid_size_y, prepare_mesh());
	return Layer_MeshTransform::set_param(param,value);
}

ValueBase
Layer_FreeFormDeform::get_param(const String& param)const
{
	EXPORT_VALUE(param_grid_points);
	EXPORT_VALUE(param_grid_size_x);
	EXPORT_VALUE(param_grid_size_y);
	
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

	return ret;
}

void Layer_FreeFormDeform::prepare_mesh()
{
	rendering::Mesh::Handle mesh(new rendering::Mesh());

	int cols = param_grid_size_x.get(int());
	int rows = param_grid_size_y.get(int());

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
	// Calculate vertices
	for (int y = 0; y < rows - 1; ++y) {
		for (int x = 0; x < cols - 1; ++x) {
			Point P00 = ctrl_points[y * cols + x];
			Point P10 = ctrl_points[y * cols + (x + 1)];
			Point P01 = ctrl_points[(y + 1) * cols + x];
			Point P11 = ctrl_points[(y + 1) * cols + (x + 1)];

			Point P00_orig = Point(min_x + x * cell_w, min_y + y * cell_h);
			Point P10_orig = Point(min_x + (x + 1) * cell_w, min_y + y * cell_h);
			Point P01_orig = Point(min_x + x * cell_w, min_y + (y + 1) * cell_h);
			Point P11_orig = Point(min_x + (x + 1) * cell_w, min_y + (y + 1) * cell_h);

			// Subdivide this cell into a dense mesh for rendering
			for (int v_step = 0; v_step <= sub_steps; ++v_step) {
				for (int u_step = 0; u_step <= sub_steps; ++u_step) {
					Real u = (Real)u_step / sub_steps;
					Real v = (Real)v_step / sub_steps;

					// APPLY BILINEAR INTERPOLATION FOR DEFORMED POSITION
					Point calculated_pos =
						P00 * ((1 - u)*(1 - v)) +
						P10 * (u*(1 - v)) +
						P01 * ((1 - u)*v) +
						P11 * (u*v);

					// APPLY BILINEAR INTERPOLATION FOR ORIGINAL UNDEFORMED POSITION
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
