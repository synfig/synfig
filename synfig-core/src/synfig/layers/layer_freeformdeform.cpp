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
#include <synfig/general.h>
#include "layer_pastecanvas.h"

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
	param_smoothness(Real(1.0)), // Default to full Catmull-Rom
	param_source_tl(Point(-2.0, 2.0)),
	param_source_br(Point(2.0, -2.0)),
	param_mesh_mode(0), // 0 = Grid, 1 = Custom Mesh
	needs_reset_(true)
{
	std::vector<ValueBase> grid_points;
	const int rows = param_grid_size_y.get(int());
	const int cols = param_grid_size_x.get(int());
	for (int y = 0; y < rows; ++y) {
		for (int x = 0; x < cols; ++x) {
			grid_points.push_back(ValueBase(Point(-2.0 + x * 2.0, 2.0 - y * 2.0)));
		}
	}
	param_grid_points.set_list_of(grid_points);
	
	std::vector<ValueBase> empty_points;
	param_source_points.set_list_of(empty_points);
	
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
	
	// Reset to true, as SET_STATIC_DEFAULTS triggers set_param which sets it to false
	needs_reset_ = true;
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
	IMPORT_VALUE_PLUS(param_grid_points, {
		needs_reset_ = false;
		prepare_mesh();
	});
	IMPORT_VALUE_PLUS(param_grid_size_x, { 
		int expected = param_grid_size_x.get(int()) * param_grid_size_y.get(int());
		if ((int)param_grid_points.get_list().size() < expected) regenerate_grid_points();
		prepare_mesh(); 
	});
	IMPORT_VALUE_PLUS(param_grid_size_y, { 
		int expected = param_grid_size_x.get(int()) * param_grid_size_y.get(int());
		if ((int)param_grid_points.get_list().size() < expected) regenerate_grid_points();
		prepare_mesh(); 
	});
	IMPORT_VALUE_PLUS(param_smoothness, prepare_mesh());
	IMPORT_VALUE_PLUS(param_source_tl, prepare_mesh());
	IMPORT_VALUE_PLUS(param_source_br, prepare_mesh());
	IMPORT_VALUE_PLUS(param_source_points, prepare_mesh());
	IMPORT_VALUE_PLUS(param_mesh_mode, prepare_mesh());
	return Layer_MeshTransform::set_param(param,value);
}

ValueBase
Layer_FreeFormDeform::get_param(const String& param)const
{
	EXPORT_VALUE(param_grid_points);
	EXPORT_VALUE(param_grid_size_x);
	EXPORT_VALUE(param_grid_size_y);
	EXPORT_VALUE(param_smoothness);
	EXPORT_VALUE(param_source_tl);
	EXPORT_VALUE(param_source_br);
	EXPORT_VALUE(param_source_points);
	EXPORT_VALUE(param_mesh_mode);
	
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

	ret.push_back(ParamDesc("source_tl")
		.set_local_name(_("Source Top Left"))
		.set_description(_("Top Left corner of the un-deformed grid"))
		.hidden()
	);

	ret.push_back(ParamDesc("source_br")
		.set_local_name(_("Source Bottom Right"))
		.set_description(_("Bottom Right corner of the un-deformed grid"))
		.hidden()
	);

	ret.push_back(ParamDesc("source_points")
		.set_local_name(_("Source Points"))
		.set_description(_("Original points of the custom mesh polygon"))
		.hidden()
	);

	ret.push_back(ParamDesc("mesh_mode")
		.set_local_name(_("Mesh Mode"))
		.set_description(_("0 = Grid, 1 = Custom Mesh"))
		.hidden()
	);

	return ret;
}

/* ---- Catmull-Rom helper methods ---- */

Point
Layer_FreeFormDeform::get_clamped_ctrl_point(
	const std::vector<Point>& ctrl_points,
	int gx, int gy, int cols, int rows) const
{
	// Extrapolate indices outside the grid to maintain straight edges
	auto get_clamped_x = [&](int x, int y) {
		if (x < 0) return ctrl_points[y * cols + 0] * 2.0 - ctrl_points[y * cols + 1];
		if (x >= cols) return ctrl_points[y * cols + cols - 1] * 2.0 - ctrl_points[y * cols + cols - 2];
		return ctrl_points[y * cols + x];
	};

	if (gy < 0) {
		Point p0 = get_clamped_x(gx, 0);
		Point p1 = get_clamped_x(gx, 1);
		return p0 * 2.0 - p1;
	} else if (gy >= rows) {
		Point p0 = get_clamped_x(gx, rows - 1);
		Point p1 = get_clamped_x(gx, rows - 2);
		return p0 * 2.0 - p1;
	} else {
		return get_clamped_x(gx, gy);
	}
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

std::vector<rendering::Mesh::Triangle>
Layer_FreeFormDeform::triangulate(const std::vector<Point>& pts)
{
	std::vector<rendering::Mesh::Triangle> triangles;
	if (pts.size() < 3) return triangles;

	Real min_x = pts[0][0], max_x = pts[0][0];
	Real min_y = pts[0][1], max_y = pts[0][1];
	for (const auto& p : pts) {
		min_x = std::min(min_x, p[0]);
		max_x = std::max(max_x, p[0]);
		min_y = std::min(min_y, p[1]);
		max_y = std::max(max_y, p[1]);
	}
	Real dx = max_x - min_x;
	Real dy = max_y - min_y;
	Real dmax = std::max(dx, dy);
	if (dmax < 1e-5) dmax = 1.0;
	Real mid_x = (min_x + max_x) * 0.5;
	Real mid_y = (min_y + max_y) * 0.5;

	std::vector<Point> all_pts = pts;
	// Super-triangle vertices
	all_pts.push_back(Point(mid_x - 20 * dmax, mid_y - dmax));
	all_pts.push_back(Point(mid_x, mid_y + 20 * dmax));
	all_pts.push_back(Point(mid_x + 20 * dmax, mid_y - dmax));

	int st1 = pts.size();
	int st2 = pts.size() + 1;
	int st3 = pts.size() + 2;

	triangles.push_back(rendering::Mesh::Triangle(st1, st2, st3));

	auto in_circumcircle = [&](int pi, int a_i, int b_i, int c_i) {
		Point p = all_pts[pi];
		Point a = all_pts[a_i];
		Point b = all_pts[b_i];
		Point c = all_pts[c_i];
		
		Real D = 2.0 * (a[0]*(b[1]-c[1]) + b[0]*(c[1]-a[1]) + c[0]*(a[1]-b[1]));
		if (std::abs(D) < 1e-6) return false;

		Real ux = ((a[0]*a[0] + a[1]*a[1]) * (b[1] - c[1]) + 
				   (b[0]*b[0] + b[1]*b[1]) * (c[1] - a[1]) + 
				   (c[0]*c[0] + c[1]*c[1]) * (a[1] - b[1])) / D;
		Real uy = ((a[0]*a[0] + a[1]*a[1]) * (c[0] - b[0]) + 
				   (b[0]*b[0] + b[1]*b[1]) * (a[0] - c[0]) + 
				   (c[0]*c[0] + c[1]*c[1]) * (b[0] - a[0])) / D;
				   
		Point center(ux, uy);
		Real r2 = (a - center).mag_squared();
		return (p - center).mag_squared() <= r2 + 1e-5;
	};

	for (size_t i = 0; i < pts.size(); ++i) {
		std::vector<int> bad_triangles;
		std::vector<std::pair<int, int>> polygon;

		for (size_t j = 0; j < triangles.size(); ++j) {
			auto& tri = triangles[j];
			if (in_circumcircle(i, tri.vertices[0], tri.vertices[1], tri.vertices[2])) {
				bad_triangles.push_back(j);
				polygon.push_back({tri.vertices[0], tri.vertices[1]});
				polygon.push_back({tri.vertices[1], tri.vertices[2]});
				polygon.push_back({tri.vertices[2], tri.vertices[0]});
			}
		}

		std::vector<std::pair<int, int>> boundary;
		for (size_t j = 0; j < polygon.size(); ++j) {
			bool shared = false;
			for (size_t k = 0; k < polygon.size(); ++k) {
				if (j != k && ((polygon[j].first == polygon[k].first && polygon[j].second == polygon[k].second) ||
							   (polygon[j].first == polygon[k].second && polygon[j].second == polygon[k].first))) {
					shared = true;
					break;
				}
			}
			if (!shared) {
				boundary.push_back(polygon[j]);
			}
		}

		for (auto it = bad_triangles.rbegin(); it != bad_triangles.rend(); ++it) {
			triangles.erase(triangles.begin() + *it);
		}

		for (const auto& edge : boundary) {
			triangles.push_back(rendering::Mesh::Triangle(edge.first, edge.second, i));
		}
	}

	std::vector<rendering::Mesh::Triangle> final_triangles;
	for (const auto& tri : triangles) {
		if (tri.vertices[0] == st1 || tri.vertices[0] == st2 || tri.vertices[0] == st3 ||
			tri.vertices[1] == st1 || tri.vertices[1] == st2 || tri.vertices[1] == st3 ||
			tri.vertices[2] == st1 || tri.vertices[2] == st2 || tri.vertices[2] == st3) {
			continue;
		}
		
		// Ensure consistent CCW orientation for rendering
		Point a = all_pts[tri.vertices[0]];
		Point b = all_pts[tri.vertices[1]];
		Point c = all_pts[tri.vertices[2]];
		if ((b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0]) > 0.0) {
		    final_triangles.push_back(rendering::Mesh::Triangle(tri.vertices[0], tri.vertices[2], tri.vertices[1]));
		} else {
		    final_triangles.push_back(tri);
		}
	}

	return final_triangles;
}

void
Layer_FreeFormDeform::on_canvas_set()
{
	if (needs_reset_ && get_canvas()) {
		needs_reset_ = false;

		Rect bounds = get_context_bounds();
		
		if (bounds.is_valid() && bounds.area() > 0.0001) {
			int cols = param_grid_size_x.get(int());
			int rows = param_grid_size_y.get(int());
			
			std::vector<Point> grid_points = compute_grid_for_bounds(bounds, cols, rows);
			std::vector<ValueBase> grid_points_vb;
			for (const auto& p : grid_points) grid_points_vb.push_back(ValueBase(p));

			param_grid_points.set_list_of(grid_points_vb);
			param_source_tl.set(Point(bounds.minx, bounds.maxy));
			param_source_br.set(Point(bounds.maxx, bounds.miny));
			prepare_mesh();
		}
	}
}

synfig::Rect
Layer_FreeFormDeform::get_context_bounds() const
{
	Rect bounds = Rect::zero();
	if (!get_canvas()) return bounds;

	Context context = get_canvas()->get_context(ContextParams());
	Context context_iter = context;
	
	while (!context_iter->empty() && (*context_iter).get() != this)
		context_iter++;
	
	if (!context_iter->empty()) {
		context_iter++; // move past ourselves to the context below
	} else {
		// We are not in the canvas yet (e.g. during LayerAdd), so the entire context is below us
		context_iter = context; 
	}
	
	bool first = true;
	
	while (!context_iter->empty()) {
		Rect layer_bounds;
		if (Layer_PasteCanvas::Handle pc = Layer_PasteCanvas::Handle::cast_dynamic(*context_iter)) {
			layer_bounds = pc->get_bounding_rect_context_dependent(ContextParams());
		} else {
			layer_bounds = (*context_iter)->get_bounding_rect();
		}

		if (layer_bounds.is_valid() && !layer_bounds.is_full_infinite() && layer_bounds.area() < 1e10) {
			if (first) {
				bounds = layer_bounds;
				first = false;
			} else {
				bounds |= layer_bounds;
			}
		}
		context_iter++;
	}

	// Fallback to canvas bounds if no finite layers are found
	if (first) {
		bounds = get_canvas()->rend_desc().get_rect();
	}

	return bounds;
}

std::vector<synfig::Point>
Layer_FreeFormDeform::compute_grid_for_bounds(const synfig::Rect& bounds, int cols, int rows) const
{
	std::vector<synfig::Point> grid_points;
	Real minx = bounds.minx;
	Real maxx = bounds.maxx;
	Real miny = bounds.miny;
	Real maxy = bounds.maxy;

	for (int y = 0; y < rows; ++y) {
		for (int x = 0; x < cols; ++x) {
			Real px = minx + x * (maxx - minx) / (cols - 1);
			Real py = maxy - y * (maxy - miny) / (rows - 1);
			grid_points.push_back(Point(px, py));
		}
	}
	return grid_points;
}

/* ---- Grid regeneration ---- */

void Layer_FreeFormDeform::regenerate_grid_points()
{
	int cols = param_grid_size_x.get(int());
	int rows = param_grid_size_y.get(int());
	if (cols < 2 || rows < 2) return;

	std::vector<ValueBase> grid_points;
	Point tl = param_source_tl.get(Point());
	Point br = param_source_br.get(Point());
	for (int y = 0; y < rows; ++y) {
		for (int x = 0; x < cols; ++x) {
			Real px = tl[0] + x * (br[0] - tl[0]) / (cols - 1);
			Real py = tl[1] - y * (tl[1] - br[1]) / (rows - 1);
			grid_points.push_back(ValueBase(Point(px, py)));
		}
	}
	param_grid_points.set_list_of(grid_points);
}

std::vector<Point> Layer_FreeFormDeform::get_interpolated_grid(int new_cols, int new_rows) const
{
	std::vector<Point> result;
	int old_cols = param_grid_size_x.get(int());
	int old_rows = param_grid_size_y.get(int());
	Real smoothness = param_smoothness.get(Real());
	if (smoothness < 0.0) smoothness = 0.0;
	if (smoothness > 1.0) smoothness = 1.0;

	std::vector<Point> ctrl_points;
	const ValueBase::List &points_list = param_grid_points.get_list();
	for(ValueBase::List::const_iterator i = points_list.begin(); i != points_list.end(); ++i) {
		if (i->can_get(Point())) ctrl_points.push_back(i->get(Point()));
	}

	if (new_cols < 2 || new_rows < 2 || ctrl_points.size() < (size_t)(old_cols * old_rows)) {
		// fallback
		Point tl = param_source_tl.get(Point());
		Point br = param_source_br.get(Point());
		for (int y = 0; y < new_rows; ++y) {
			for (int x = 0; x < new_cols; ++x) {
				Real px = tl[0] + x * (br[0] - tl[0]) / (new_cols - 1);
				Real py = tl[1] - y * (tl[1] - br[1]) / (new_rows - 1);
				result.push_back(Point(px, py));
			}
		}
		return result;
	}

	for (int ny = 0; ny < new_rows; ++ny) {
		for (int nx = 0; nx < new_cols; ++nx) {
			Real u = (Real)nx / (new_cols - 1);
			Real v = (Real)ny / (new_rows - 1);

			// Map (u,v) to the old grid space
			Real old_x = u * (old_cols - 1);
			Real old_y = v * (old_rows - 1);

			int x = (int)floor(old_x);
			int y = (int)floor(old_y);
			if (x >= old_cols - 1) x = old_cols - 2;
			if (y >= old_rows - 1) y = old_rows - 2;

			Real local_u = old_x - x;
			Real local_v = old_y - y;

			Point P00 = ctrl_points[y * old_cols + x];
			Point P10 = ctrl_points[y * old_cols + (x + 1)];
			Point P01 = ctrl_points[(y + 1) * old_cols + x];
			Point P11 = ctrl_points[(y + 1) * old_cols + (x + 1)];

			Point bilinear_pos =
				P00 * ((1 - local_u)*(1 - local_v)) +
				P10 * (local_u*(1 - local_v)) +
				P01 * ((1 - local_u)*local_v) +
				P11 * (local_u*local_v);

			Point calculated_pos;

			if (smoothness > 0.0) {
				Real row_x[4], row_y[4];
				for (int ky = -1; ky <= 2; ++ky) {
					Point cp0 = get_clamped_ctrl_point(ctrl_points, x - 1, y + ky, old_cols, old_rows);
					Point cp1 = get_clamped_ctrl_point(ctrl_points, x,     y + ky, old_cols, old_rows);
					Point cp2 = get_clamped_ctrl_point(ctrl_points, x + 1, y + ky, old_cols, old_rows);
					Point cp3 = get_clamped_ctrl_point(ctrl_points, x + 2, y + ky, old_cols, old_rows);

					row_x[ky + 1] = catmull_rom_1d(cp0[0], cp1[0], cp2[0], cp3[0], local_u);
					row_y[ky + 1] = catmull_rom_1d(cp0[1], cp1[1], cp2[1], cp3[1], local_u);
				}

				Point catmull_pos = Point(
					catmull_rom_1d(row_x[0], row_x[1], row_x[2], row_x[3], local_v),
					catmull_rom_1d(row_y[0], row_y[1], row_y[2], row_y[3], local_v)
				);

				calculated_pos = bilinear_pos * (1.0 - smoothness) + catmull_pos * smoothness;
			} else {
				calculated_pos = bilinear_pos;
			}

			result.push_back(calculated_pos);
		}
	}
	return result;
}

/* ---- Core mesh preparation ---- */

void Layer_FreeFormDeform::prepare_mesh()
{
	rendering::Mesh::Handle mesh(new rendering::Mesh());

	int mode = param_mesh_mode.get(int());

	if (mode == 1) {
		// --- CUSTOM MESH MODE ---
		std::vector<Point> deformed_pts;
		const ValueBase::List &grid_list = param_grid_points.get_list();
		for(auto i = grid_list.begin(); i != grid_list.end(); ++i) {
			if (i->can_get(Point())) deformed_pts.push_back(i->get(Point()));
		}

		std::vector<Point> initial_pts;
		const ValueBase::List &source_list = param_source_points.get_list();
		for(auto i = source_list.begin(); i != source_list.end(); ++i) {
			if (i->can_get(Point())) initial_pts.push_back(i->get(Point()));
		}

		if (deformed_pts.size() < 3 || initial_pts.size() != deformed_pts.size()) {
			this->mesh = mesh;
			return; // Not enough points or mismatch
		}

		// 1. Calculate the bounding box of the USER's points (e.g. The Mouth)
		Real mx = initial_pts[0][0], Mx = initial_pts[0][0];
		Real my = initial_pts[0][1], My = initial_pts[0][1];
		for (const auto& p : initial_pts) {
			mx = std::min(mx, p[0]); Mx = std::max(Mx, p[0]);
			my = std::min(my, p[1]); My = std::max(My, p[1]);
		}
		
		// 2. Create an "Invisible Fence" just outside the user's points.
		Real padding_x = (Mx - mx) * 0.5;
		Real padding_y = (My - my) * 0.5;
		if (padding_x < 0.05) padding_x = 0.05;
		if (padding_y < 0.05) padding_y = 0.05;

		Point f_tl(mx - padding_x, My + padding_y);
		Point f_tr(Mx + padding_x, My + padding_y);
		Point f_bl(mx - padding_x, my - padding_y);
		Point f_br(Mx + padding_x, my - padding_y);

		// 3. Get Safe Image Corners using the layer's actual bounds!
		Point stl = param_source_tl.get(Point());
		Point sbr = param_source_br.get(Point());
		
		Real min_x = std::min(stl[0], sbr[0]) - 1.0;
		Real max_x = std::max(stl[0], sbr[0]) + 1.0;
		Real max_y = std::max(stl[1], sbr[1]) + 1.0;
		Real min_y = std::min(stl[1], sbr[1]) - 1.0;

		// 8 points around the outer edge to guarantee perfectly stable triangles
		Point c_tl(min_x, max_y);
		Point c_tm((min_x + max_x) * 0.5, max_y);
		Point c_tr(max_x, max_y);
		Point c_rm(max_x, (min_y + max_y) * 0.5);
		Point c_br(max_x, min_y);
		Point c_bm((min_x + max_x) * 0.5, min_y);
		Point c_bl(min_x, min_y);
		Point c_lm(min_x, (min_y + max_y) * 0.5);

		// 4. Add the Fence and Outer Boundary to the mesh
		auto add_static_anchor = [&](const Point& p) {
			initial_pts.push_back(p);
			deformed_pts.push_back(p); // By keeping them identical, they NEVER move.
		};

		// Add Fence Anchors
		add_static_anchor(f_tl);
		add_static_anchor(f_tr);
		add_static_anchor(f_bl);
		add_static_anchor(f_br);
		
		// Add Outer Bounds Anchors
		add_static_anchor(c_tl);
		add_static_anchor(c_tm);
		add_static_anchor(c_tr);
		add_static_anchor(c_rm);
		add_static_anchor(c_br);
		add_static_anchor(c_bm);
		add_static_anchor(c_bl);
		add_static_anchor(c_lm);

		// Triangulate
		std::vector<rendering::Mesh::Triangle> triangles = triangulate(initial_pts);

		// Add vertices
		for (size_t i = 0; i < deformed_pts.size(); ++i) {
			mesh->vertices.push_back(rendering::Mesh::Vertex(deformed_pts[i], initial_pts[i]));
		}

		mesh->triangles = triangles;

		// --- BYPASS MASK TO PREVENT CRASH ---
		rendering::Contour::Handle mask(new rendering::Contour());
		mask->antialias = false; 
		mask->move_to(c_tl);
		mask->line_to(c_tr);
		mask->line_to(c_br);
		mask->line_to(c_bl);
		mask->close();

		this->mask = mask;
		this->mesh = mesh;
		return;
	}

	// --- GRID MODE ---
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

	// Calculate un-deformed bounds based on the initial grid bounds
	Point tl = param_source_tl.get(Point());
	Point br = param_source_br.get(Point());
	Real start_x = tl[0];
	Real start_y = tl[1];
	Real cell_w = (br[0] - tl[0]) / (cols - 1);
	Real cell_h = (br[1] - tl[1]) / (rows - 1);

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
					Point P00_orig = Point(start_x + x * cell_w,       start_y + y * cell_h);
					Point P10_orig = Point(start_x + (x + 1) * cell_w, start_y + y * cell_h);
					Point P01_orig = Point(start_x + x * cell_w,       start_y + (y + 1) * cell_h);
					Point P11_orig = Point(start_x + (x + 1) * cell_w, start_y + (y + 1) * cell_h);

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

	// Create the mask contour using the SOURCE (undeformed) bounding rectangle.
	rendering::Contour::Handle mask(new rendering::Contour());
	mask->antialias = true;
	mask->move_to(tl);
	mask->line_to(Point(br[0], tl[1]));
	mask->line_to(br);
	mask->line_to(Point(tl[0], br[1]));
	mask->close();
	this->mask = mask;

	this->mesh = mesh;
}
