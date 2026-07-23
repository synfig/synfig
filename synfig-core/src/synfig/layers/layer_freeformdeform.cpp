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
#include <synfig/surface.h>
#include <synfig/renddesc.h>
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
	param_source_angle(Angle::deg(0.0)),
	param_mesh_mode(0), // 0 = Grid, 1 = Custom Mesh
	param_cull_threshold(Real(1.0)), // 0 = disabled
	param_auto_mesh_margin(Real(5)),     // pixels
	param_auto_mesh_edge_length(Real(0.5)), // canvas units
	param_auto_mesh_dpi(300),
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
	
	std::vector<ValueBase> empty_triangles;
	param_triangles.set_list_of(empty_triangles);
	
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
	IMPORT_VALUE_PLUS(param_source_angle, prepare_mesh());
	IMPORT_VALUE_PLUS(param_source_points, prepare_mesh());
	IMPORT_VALUE_PLUS(param_mesh_mode, prepare_mesh());
	IMPORT_VALUE_PLUS(param_cull_threshold, prepare_mesh());
	IMPORT_VALUE_PLUS(param_triangles, prepare_mesh());
	IMPORT_VALUE(param_auto_mesh_margin);
	IMPORT_VALUE(param_auto_mesh_edge_length);
	IMPORT_VALUE(param_auto_mesh_dpi);
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
	EXPORT_VALUE(param_source_angle);
	EXPORT_VALUE(param_source_points);
	EXPORT_VALUE(param_mesh_mode);
	EXPORT_VALUE(param_cull_threshold);
	EXPORT_VALUE(param_triangles);
	EXPORT_VALUE(param_auto_mesh_margin);
	EXPORT_VALUE(param_auto_mesh_edge_length);
	EXPORT_VALUE(param_auto_mesh_dpi);
	
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

	ret.push_back(ParamDesc("source_angle")
		.set_local_name(_("Source Angle"))
		.set_description(_("Angle of the un-deformed grid"))
		.hidden()
	);

	ret.push_back(ParamDesc("source_points")
		.set_local_name(_("Source Points"))
		.set_description(_("Original points of the custom mesh polygon"))
		.hidden()
	);

	ret.push_back(ParamDesc("triangles")
		.set_local_name(_("Triangles"))
		.set_description(_("Explicit triangle list for custom mesh topology"))
		.hidden()
	);

	ret.push_back(ParamDesc("mesh_mode")
		.set_local_name(_("Mesh Mode"))
		.set_description(_("0 = Grid, 1 = Custom Mesh"))
		.hidden()
	);

	ret.push_back(ParamDesc("cull_threshold")
		.set_local_name(_("Cull Threshold"))
		.set_description(_("Threshold for removing long triangles (Alpha Shape)"))
	);

	ret.push_back(ParamDesc("auto_mesh_margin")
		.set_local_name(_("Auto Mesh Margin"))
		.set_description(_("Dilation radius in pixels for auto-mesh edge contour"))
		.hidden()
	);

	ret.push_back(ParamDesc("auto_mesh_edge_length")
		.set_local_name(_("Auto Mesh Edge Length"))
		.set_description(_("Spacing between auto-mesh edge points in canvas units"))
		.hidden()
	);

	ret.push_back(ParamDesc("auto_mesh_dpi")
		.set_local_name(_("Auto Mesh DPI"))
		.set_description(_("Rasterization DPI for auto-mesh alpha mask"))
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

std::vector<rendering::Mesh::Triangle>
Layer_FreeFormDeform::cull_triangles(
	const std::vector<rendering::Mesh::Triangle>& tris,
	const std::vector<Point>& pts,
	Real threshold)
{
	if (threshold <= 0.0)
		return tris;

	const Real cull_sq = threshold * threshold;
	std::vector<rendering::Mesh::Triangle> result;
	result.reserve(tris.size());

	for (const auto& tri : tris) {
		const Point& a = pts[tri.vertices[0]];
		const Point& b = pts[tri.vertices[1]];
		const Point& c = pts[tri.vertices[2]];

		Real D = 2.0 * (a[0]*(b[1]-c[1]) + b[0]*(c[1]-a[1]) + c[0]*(a[1]-b[1]));
		if (std::abs(D) > 1e-6) {
			Real ux = ((a[0]*a[0] + a[1]*a[1]) * (b[1] - c[1]) +
			           (b[0]*b[0] + b[1]*b[1]) * (c[1] - a[1]) +
			           (c[0]*c[0] + c[1]*c[1]) * (a[1] - b[1])) / D;
			Real uy = ((a[0]*a[0] + a[1]*a[1]) * (c[0] - b[0]) +
			           (b[0]*b[0] + b[1]*b[1]) * (a[0] - c[0]) +
			           (c[0]*c[0] + c[1]*c[1]) * (b[0] - a[0])) / D;
			if ((a - Point(ux, uy)).mag_squared() > cull_sq)
				continue;
		}
		result.push_back(tri);
	}
	return result;
}

/* ---- Contour polygon generation (mask → dilate → trace → canvas coords) ---- */

std::vector<Point>
Layer_FreeFormDeform::generate_contour_polygon(
	const Surface &alpha_surface,
	const Rect &bounds,
	int margin)
{
	std::vector<Point> result;
	int w = alpha_surface.get_w();
	int h = alpha_surface.get_h();
	if (!w || !h || !bounds.is_valid()) return result;

	// 1. Build alpha mask
	std::vector<bool> mask(w * h, false);
	for (int y = 0; y < h; ++y)
		for (int x = 0; x < w; ++x)
			if (alpha_surface[y][x].get_a() > 0.05f)
				mask[y * w + x] = true;

	// 2. Dilate by margin (circular kernel)
	std::vector<bool> dilated = mask;
	if (margin > 0) {
		std::fill(dilated.begin(), dilated.end(), false);
		int m2 = margin * margin;
		for (int y = 0; y < h; ++y) {
			for (int x = 0; x < w; ++x) {
				if (!mask[y * w + x]) continue;
				int my0 = std::max(0, y - margin);
				int my1 = std::min(h - 1, y + margin);
				int mx0 = std::max(0, x - margin);
				int mx1 = std::min(w - 1, x + margin);
				for (int my = my0; my <= my1; ++my)
					for (int mx = mx0; mx <= mx1; ++mx)
						if ((mx - x) * (mx - x) + (my - y) * (my - y) <= m2)
							dilated[my * w + mx] = true;
			}
		}
	}

	// 3. Find start pixel for contour trace
	int start_x = -1, start_y = -1;
	for (int y = 0; y < h && start_y == -1; ++y)
		for (int x = 0; x < w; ++x)
			if (dilated[y * w + x]) { start_x = x; start_y = y; break; }
	if (start_x == -1) return result;

	// 4. Moore Neighborhood contour trace
	static const int dx[8] = {1, 1, 0, -1, -1, -1, 0, 1};
	static const int dy[8] = {0, 1, 1, 1, 0, -1, -1, -1};
	auto is_inside = [&](int cx, int cy) -> bool {
		if (cx < 0 || cx >= w || cy < 0 || cy >= h) return false;
		return (bool)dilated[cy * w + cx];
	};

	std::vector<Point> contour_px;
	int x = start_x, y = start_y, dir = 7;
	int iters = 0, max_iters = w * h * 4;
	do {
		contour_px.push_back(Point(x, y));
		int start_dir = (dir + 5) % 8;
		int next_dir = -1;
		for (int i = 0; i < 8; ++i) {
			int test_dir = (start_dir + i) % 8;
			if (is_inside(x + dx[test_dir], y + dy[test_dir])) { next_dir = test_dir; break; }
		}
		if (next_dir == -1) break;
		x += dx[next_dir]; y += dy[next_dir]; dir = next_dir;
	} while ((x != start_x || y != start_y) && ++iters < max_iters);

	if (contour_px.size() < 3) return result;

	// 5. Convert pixel coords to canvas coords
	double units_w = bounds.maxx - bounds.minx;
	double units_h = bounds.maxy - bounds.miny;
	double inv_w = 1.0 / w;
	double inv_h = 1.0 / h;
	result.reserve(contour_px.size());
	for (auto& p : contour_px) {
		double u = p[0] * inv_w;
		double v = p[1] * inv_h;
		result.push_back(Point(bounds.minx + u * units_w,
		                       bounds.maxy - v * units_h));
	}
	return result;
}

/* ---- auto-mesh edge contour tracing ---- */

std::vector<Point>
Layer_FreeFormDeform::generate_edge_points(
	const Surface &alpha_surface,
	const Rect &bounds,
	Real edge_length,
	int margin)
{
	// Get the dense contour polygon first
	std::vector<Point> contour = generate_contour_polygon(alpha_surface, bounds, margin);
	if (contour.size() < 3) return contour;

	double bbox_size = std::max(
		(double)(bounds.maxx - bounds.minx),
		(double)(bounds.maxy - bounds.miny));
	if (bbox_size < 1e-6) bbox_size = 1.0;

	double target_dist = bbox_size / (edge_length * 8.0);
	double step_min = bbox_size / 300.0;
	if (target_dist < step_min) target_dist = step_min;
	if (target_dist < 1e-6) target_dist = 1e-6;

	std::vector<Point> result;
	result.push_back(contour[0]);
	double dist_accum = 0;
	for (size_t i = 1; i < contour.size(); ++i) {
		double d = (contour[i] - contour[i - 1]).mag();
		dist_accum += d;
		if (dist_accum >= target_dist) {
			result.push_back(contour[i]);
			dist_accum = 0;
		}
	}
	return result;
}

/* ---- interior Steiner point generation ---- */

// Forward declaration (defined below in "Centroid-in-polygon filter" section)
static bool point_in_polygon(const synfig::Point &p, const std::vector<synfig::Point> &poly);

std::vector<Point>
Layer_FreeFormDeform::generate_interior_points(
	const std::vector<Point> &contour_polygon,
	Real edge_length)
{
	std::vector<Point> result;
	if (contour_polygon.size() < 3 || edge_length < 1e-6)
		return result;

	// Compute bounding box of the contour
	Real minx = contour_polygon[0][0], maxx = contour_polygon[0][0];
	Real miny = contour_polygon[0][1], maxy = contour_polygon[0][1];
	for (const auto &p : contour_polygon) {
		minx = std::min(minx, p[0]); maxx = std::max(maxx, p[0]);
		miny = std::min(miny, p[1]); maxy = std::max(maxy, p[1]);
	}

	double bbox_size = std::max((double)(maxx - minx), (double)(maxy - miny));
	if (bbox_size < 1e-6) return result;

	double step = bbox_size / (edge_length * 2.0);
	double step_min = bbox_size / 25.0; // max 25 divisions per axis
	if (step < step_min) step = step_min;
	if (step < 1e-6) step = 1e-6;

	int row = 0;
	for (double y = miny + step * 0.5; y < maxy - step * 0.1; y += step, ++row) {
		double x_offset = (row % 2 == 0) ? 0.0 : step * 0.5;
		for (double x = minx + step * 0.5 + x_offset; x < maxx - step * 0.1; x += step) {
			Point p(x, y);
			if (point_in_polygon(p, contour_polygon))
				result.push_back(p);
		}
	}
	return result;
}

/* ---- Centroid-in-polygon filter ---- */

static bool point_in_polygon(const Point &p, const std::vector<Point> &poly)
{
	bool inside = false;
	size_t n = poly.size();
	for (size_t i = 0, j = n - 1; i < n; j = i++) {
		if (((poly[i][1] > p[1]) != (poly[j][1] > p[1])) &&
		    (p[0] < (poly[j][0] - poly[i][0]) * (p[1] - poly[i][1])
		            / (poly[j][1] - poly[i][1]) + poly[i][0]))
			inside = !inside;
	}
	return inside;
}

std::vector<rendering::Mesh::Triangle>
Layer_FreeFormDeform::filter_triangles_by_polygon(
	const std::vector<rendering::Mesh::Triangle> &tris,
	const std::vector<Point> &pts,
	const std::vector<Point> &polygon)
{
	if (polygon.size() < 3)
		return tris; // nothing to filter against

	std::vector<rendering::Mesh::Triangle> result;
	result.reserve(tris.size());
	for (const auto &tri : tris) {
		const Point &a = pts[tri.vertices[0]];
		const Point &b = pts[tri.vertices[1]];
		const Point &c = pts[tri.vertices[2]];
		Point centroid((a[0] + b[0] + c[0]) / 3.0,
		               (a[1] + b[1] + c[1]) / 3.0);
		if (point_in_polygon(centroid, polygon))
			result.push_back(tri);
	}
	return result;
}


bool
Layer_FreeFormDeform::render_context_below(Surface &out_surface, Rect &out_bounds, int max_resolution) const
{
	if (!get_canvas()) return false;

	// Find context below this layer (same logic as get_context_bounds)
	Context context = get_canvas()->get_context(ContextParams());
	Context context_iter = context;
	while (!context_iter->empty() && (*context_iter).get() != this)
		context_iter++;
	if (!context_iter->empty())
		context_iter++; // move past ourselves

	if (context_iter->empty()) return false;

	// Compute bounding box of the context below
	out_bounds = Rect::zero();
	bool first = true;
	Context bounds_iter = context_iter;
	while (!bounds_iter->empty()) {
		Rect layer_bounds;
		if (Layer_PasteCanvas::Handle pc = Layer_PasteCanvas::Handle::cast_dynamic(*bounds_iter))
			layer_bounds = pc->get_bounding_rect_context_dependent(ContextParams());
		else
			layer_bounds = (*bounds_iter)->get_bounding_rect();
		if (layer_bounds.is_valid() && !layer_bounds.is_full_infinite() && layer_bounds.area() < 1e10) {
			if (first) { out_bounds = layer_bounds; first = false; }
			else out_bounds |= layer_bounds;
		}
		bounds_iter++;
	}
	if (first) return false;

	// Expand bounds slightly to avoid clipping
	Vector size(out_bounds.maxx - out_bounds.minx, out_bounds.maxy - out_bounds.miny);
	out_bounds.minx -= size[0] * 0.1; out_bounds.maxx += size[0] * 0.1;
	out_bounds.miny -= size[1] * 0.1; out_bounds.maxy += size[1] * 0.1;

	// Set up render at target resolution
	double aspect = (double)std::max(out_bounds.maxx - out_bounds.minx, 1e-6) /
	                std::max(out_bounds.maxy - out_bounds.miny, 1e-6);
	int res_w, res_h;
	if (aspect >= 1.0) { res_w = max_resolution; res_h = std::max(10, (int)(max_resolution / aspect)); }
	else { res_h = max_resolution; res_w = std::max(10, (int)(max_resolution * aspect)); }

	RendDesc desc;
	desc.set_tl(Point(out_bounds.minx, out_bounds.maxy));
	desc.set_br(Point(out_bounds.maxx, out_bounds.miny));
	desc.set_w(res_w);
	desc.set_h(res_h);
	desc.set_antialias(1);

	out_surface.set_wh(res_w, res_h);
	out_surface.clear();

	return context_iter.accelerated_render(&out_surface, 0, desc, 0);
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
	Angle angle = param_source_angle.get(Angle());
	Point center = (tl + br) * 0.5;
	Real angle_cos = Angle::cos(angle).get();
	Real angle_sin = Angle::sin(angle).get();

	for (int y = 0; y < rows; ++y) {
		for (int x = 0; x < cols; ++x) {
			Real px = tl[0] + x * (br[0] - tl[0]) / (cols - 1);
			Real py = tl[1] - y * (tl[1] - br[1]) / (rows - 1);
			
			Real dx = px - center[0];
			Real dy = py - center[1];
			Real rx = center[0] + dx * angle_cos - dy * angle_sin;
			Real ry = center[1] + dx * angle_sin + dy * angle_cos;
			
			grid_points.push_back(ValueBase(Point(rx, ry)));
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
		Angle angle = param_source_angle.get(Angle());
		Point center = (tl + br) * 0.5;
		Real angle_cos = Angle::cos(angle).get();
		Real angle_sin = Angle::sin(angle).get();

		for (int y = 0; y < new_rows; ++y) {
			for (int x = 0; x < new_cols; ++x) {
				Real px = tl[0] + x * (br[0] - tl[0]) / (new_cols - 1);
				Real py = tl[1] - y * (tl[1] - br[1]) / (new_rows - 1);
				
				Real dx = px - center[0];
				Real dy = py - center[1];
				Real rx = center[0] + dx * angle_cos - dy * angle_sin;
				Real ry = center[1] + dx * angle_sin + dy * angle_cos;
				
				result.push_back(Point(rx, ry));
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

		// Save user point count before adding fence/outer anchors
		const int original_pt_count = initial_pts.size();

		// 5. Add fence and outer boundary anchors (they never move)
		auto add_static_anchor = [&](const Point& p) {
			initial_pts.push_back(p);
			deformed_pts.push_back(p);
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

		// Use explicit triangles if provided, else triangulate
		std::vector<rendering::Mesh::Triangle> triangles;
		const std::vector<ValueBase>& param_tris = param_triangles.get_list();
		if (!param_tris.empty() && param_tris.size() % 3 == 0) {
			for (size_t i = 0; i < param_tris.size(); i += 3) {
				rendering::Mesh::Triangle tri;
				tri.vertices[0] = param_tris[i].get(int());
				tri.vertices[1] = param_tris[i+1].get(int());
				tri.vertices[2] = param_tris[i+2].get(int());
				triangles.push_back(tri);
			}
		} else {
			triangles = triangulate(initial_pts);
		}

		// Apply Cull Threshold (Alpha Shape)
		Real cull_threshold = param_cull_threshold.get(Real());
		if (cull_threshold > 0.0) {
			Real cull_sq = cull_threshold * cull_threshold;
			std::vector<rendering::Mesh::Triangle> culled_triangles;
			
			for (const auto& tri : triangles) {
				// Only check triangles formed entirely by user points
				if (tri.vertices[0] < original_pt_count && 
					tri.vertices[1] < original_pt_count && 
					tri.vertices[2] < original_pt_count) {
					
					Point a = initial_pts[tri.vertices[0]];
					Point b = initial_pts[tri.vertices[1]];
					Point c = initial_pts[tri.vertices[2]];
					
					Real D = 2.0 * (a[0]*(b[1]-c[1]) + b[0]*(c[1]-a[1]) + c[0]*(a[1]-b[1]));
					if (std::abs(D) > 1e-6) {
						Real ux = ((a[0]*a[0] + a[1]*a[1]) * (b[1] - c[1]) + 
								   (b[0]*b[0] + b[1]*b[1]) * (c[1] - a[1]) + 
								   (c[0]*c[0] + c[1]*c[1]) * (a[1] - b[1])) / D;
						Real uy = ((a[0]*a[0] + a[1]*a[1]) * (c[0] - b[0]) + 
								   (b[0]*b[0] + b[1]*b[1]) * (a[0] - c[0]) + 
								   (c[0]*c[0] + c[1]*c[1]) * (b[0] - a[0])) / D;
						Point center(ux, uy);
						Real r2 = (a - center).mag_squared();
						
						if (r2 > cull_sq) {
							// FREEZE THIS TRIANGLE
							// Instead of culling (which leaves a hole and causes clipping),
							// we lock its vertices to their initial positions.
							// We duplicate the vertices so we don't freeze adjacent valid triangles.
							rendering::Mesh::Triangle frozen_tri;
							frozen_tri.vertices[0] = deformed_pts.size();
							deformed_pts.push_back(a);
							initial_pts.push_back(a);
							
							frozen_tri.vertices[1] = deformed_pts.size();
							deformed_pts.push_back(b);
							initial_pts.push_back(b);
							
							frozen_tri.vertices[2] = deformed_pts.size();
							deformed_pts.push_back(c);
							initial_pts.push_back(c);
							
							culled_triangles.push_back(frozen_tri);
							continue;
						}
					}
				}
				culled_triangles.push_back(tri);
			}
			triangles = culled_triangles;
		}

		// Add vertices
		for (size_t i = 0; i < deformed_pts.size(); ++i) {
			mesh->vertices.push_back(rendering::Mesh::Vertex(deformed_pts[i], initial_pts[i]));
		}

		mesh->triangles = triangles;

		// Mask covers the full canvas so outer boundary triangles sample the original image
		{
			rendering::Contour::Handle mask(new rendering::Contour());
			mask->antialias = false;
			mask->move_to(c_tl);
			mask->line_to(c_tr);
			mask->line_to(c_br);
			mask->line_to(c_bl);
			mask->close();
			this->mask = mask;
		}

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
	Angle angle = param_source_angle.get(Angle());
	Point center = (tl + br) * 0.5;
	Real angle_cos = Angle::cos(angle).get();
	Real angle_sin = Angle::sin(angle).get();

	Real start_x = tl[0];
	Real start_y = tl[1];
	Real cell_w = (br[0] - tl[0]) / (cols - 1);
	Real cell_h = (br[1] - tl[1]) / (rows - 1);

	int sub_steps = 10;

	// Helper: rotate a source-space point by source_angle around center
	auto rotate_src = [&](const Point& pt) {
		Real dx = pt[0] - center[0];
		Real dy = pt[1] - center[1];
		return Point(
			center[0] + dx * angle_cos - dy * angle_sin,
			center[1] + dx * angle_sin + dy * angle_cos
		);
	};
	
	int edge_len_h = (cols - 1) * sub_steps + 1; 
	int edge_len_v = (rows - 1) * sub_steps + 1;
	std::vector<int> edge_top(edge_len_h, -1);
	std::vector<int> edge_bottom(edge_len_h, -1);
	std::vector<int> edge_left(edge_len_v, -1);
	std::vector<int> edge_right(edge_len_v, -1);

	// Calculate vertices for each cell
	for (int y = 0; y < rows - 1; ++y) {
		for (int x = 0; x < cols - 1; ++x) {

			// Subdivide this cell into a dense mesh for rendering
			for (int v_step = 0; v_step <= sub_steps; ++v_step) {
				for (int u_step = 0; u_step <= sub_steps; ++u_step) {
					Real u = (Real)u_step / sub_steps;
					Real v = (Real)v_step / sub_steps;

					// Bilinear interpolation of deformed control points
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
						// Catmull-Rom spline: interpolate along u then v

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

						// Blend bilinear and Catmull-Rom by smoothness
						calculated_pos = bilinear_pos * (1.0 - smoothness) + catmull_pos * smoothness;
					} else {
						calculated_pos = bilinear_pos;
					}

					// Undeformed (source) position on the uniform grid
					Point P00_orig = Point(start_x + x * cell_w,       start_y + y * cell_h);
					Point P10_orig = Point(start_x + (x + 1) * cell_w, start_y + y * cell_h);
					Point P01_orig = Point(start_x + x * cell_w,       start_y + (y + 1) * cell_h);
					Point P11_orig = Point(start_x + (x + 1) * cell_w, start_y + (y + 1) * cell_h);

					Point initial_pos =
						P00_orig * ((1 - u)*(1 - v)) +
						P10_orig * (u*(1 - v)) +
						P01_orig * ((1 - u)*v) +
						P11_orig * (u*v);

					// Rotate the source position to match the rotated grid
					initial_pos = rotate_src(initial_pos);

					int vtx_idx = mesh->vertices.size();
					mesh->vertices.push_back(rendering::Mesh::Vertex(calculated_pos, initial_pos));

					// Track edge vertices for boundary connection
					int global_u = x * sub_steps + u_step;
					int global_v = y * sub_steps + v_step;

					if (y == 0 && v_step == 0)
						edge_top[global_u] = vtx_idx;
					if (y == rows - 2 && v_step == sub_steps)
						edge_bottom[global_u] = vtx_idx;
					if (x == 0 && u_step == 0)
						edge_left[global_v] = vtx_idx;
					if (x == cols - 2 && u_step == sub_steps)
						edge_right[global_v] = vtx_idx;
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

	auto unrotate_src = [&](const Point& pt) {
		Real dx = pt[0] - center[0];
		Real dy = pt[1] - center[1];
		return Point(
			center[0] + dx * angle_cos + dy * angle_sin,
			center[1] - dx * angle_sin + dy * angle_cos
		);
	};

	Real end_x = start_x + (cols - 1) * cell_w;
	Real end_y = start_y + (rows - 1) * cell_h;

	Real grid_min_x = std::min(start_x, end_x);
	Real grid_max_x = std::max(start_x, end_x);
	Real grid_min_y = std::min(start_y, end_y);
	Real grid_max_y = std::max(start_y, end_y);

	Real bound_min_x = grid_min_x;
	Real bound_max_x = grid_max_x;
	Real bound_min_y = grid_min_y;
	Real bound_max_y = grid_max_y;

	// Expand to cover all deformed vertices
	for (const auto& vtx : mesh->vertices) {
		Point u = unrotate_src(vtx.position);
		bound_min_x = std::min(bound_min_x, u[0]);
		bound_max_x = std::max(bound_max_x, u[0]);
		bound_min_y = std::min(bound_min_y, u[1]);
		bound_max_y = std::max(bound_max_y, u[1]);
	}

	Real pad_x = (bound_max_x - bound_min_x) * 0.5;
	Real pad_y = (bound_max_y - bound_min_y) * 0.5;
	if (pad_x < 1.0) pad_x = 1.0;
	if (pad_y < 1.0) pad_y = 1.0;

	bound_min_x -= pad_x;
	bound_max_x += pad_x;
	bound_min_y -= pad_y;
	bound_max_y += pad_y;

	// Determine which boundary corresponds to which grid edge
	Real outer_top_y = (start_y == grid_min_y) ? bound_min_y : bound_max_y;
	Real outer_bottom_y = (end_y == grid_max_y) ? bound_max_y : bound_min_y;
	Real outer_left_x = (start_x == grid_min_x) ? bound_min_x : bound_max_x;
	Real outer_right_x = (end_x == grid_max_x) ? bound_max_x : bound_min_x;

	// Helper: add an identity vertex (takes unrotated point)
	auto add_identity_vtx = [&](const Point& unrot_pt) -> int {
		Point rotated = rotate_src(unrot_pt);
		int idx = mesh->vertices.size();
		mesh->vertices.push_back(rendering::Mesh::Vertex(rotated, rotated));
		return idx;
	};

	int c_tl_idx = add_identity_vtx(Point(outer_left_x, outer_top_y));
	int c_tr_idx = add_identity_vtx(Point(outer_right_x, outer_top_y));
	int c_bl_idx = add_identity_vtx(Point(outer_left_x, outer_bottom_y));
	int c_br_idx = add_identity_vtx(Point(outer_right_x, outer_bottom_y));

	// TOP edge strip
	std::vector<int> border_top;
	for (int i = 0; i < edge_len_h; ++i) {
		Point u_pt = unrotate_src(mesh->vertices[edge_top[i]].tex_coords);
		border_top.push_back(add_identity_vtx(Point(u_pt[0], outer_top_y)));
	}
	for (int i = 0; i < edge_len_h - 1; ++i) {
		mesh->triangles.push_back(rendering::Mesh::Triangle(border_top[i], edge_top[i], border_top[i+1]));
		mesh->triangles.push_back(rendering::Mesh::Triangle(edge_top[i], edge_top[i+1], border_top[i+1]));
	}

	// BOTTOM edge strip
	std::vector<int> border_bottom;
	for (int i = 0; i < edge_len_h; ++i) {
		Point u_pt = unrotate_src(mesh->vertices[edge_bottom[i]].tex_coords);
		border_bottom.push_back(add_identity_vtx(Point(u_pt[0], outer_bottom_y)));
	}
	for (int i = 0; i < edge_len_h - 1; ++i) {
		mesh->triangles.push_back(rendering::Mesh::Triangle(edge_bottom[i], border_bottom[i], border_bottom[i+1]));
		mesh->triangles.push_back(rendering::Mesh::Triangle(edge_bottom[i], border_bottom[i+1], edge_bottom[i+1]));
	}

	// LEFT edge strip
	std::vector<int> border_left;
	for (int i = 0; i < edge_len_v; ++i) {
		Point u_pt = unrotate_src(mesh->vertices[edge_left[i]].tex_coords);
		border_left.push_back(add_identity_vtx(Point(outer_left_x, u_pt[1])));
	}
	for (int i = 0; i < edge_len_v - 1; ++i) {
		mesh->triangles.push_back(rendering::Mesh::Triangle(edge_left[i], border_left[i], border_left[i+1]));
		mesh->triangles.push_back(rendering::Mesh::Triangle(edge_left[i], border_left[i+1], edge_left[i+1]));
	}

	// RIGHT edge strip
	std::vector<int> border_right;
	for (int i = 0; i < edge_len_v; ++i) {
		Point u_pt = unrotate_src(mesh->vertices[edge_right[i]].tex_coords);
		border_right.push_back(add_identity_vtx(Point(outer_right_x, u_pt[1])));
	}
	for (int i = 0; i < edge_len_v - 1; ++i) {
		mesh->triangles.push_back(rendering::Mesh::Triangle(border_right[i], edge_right[i], edge_right[i+1]));
		mesh->triangles.push_back(rendering::Mesh::Triangle(border_right[i], edge_right[i+1], border_right[i+1]));
	}

	// CORNER RECTANGLES
	// Top-left
	mesh->triangles.push_back(rendering::Mesh::Triangle(c_tl_idx, border_top[0], border_left[0]));
	mesh->triangles.push_back(rendering::Mesh::Triangle(border_top[0], edge_top[0], border_left[0]));
	// Top-right
	mesh->triangles.push_back(rendering::Mesh::Triangle(c_tr_idx, border_right[0], border_top[edge_len_h-1]));
	mesh->triangles.push_back(rendering::Mesh::Triangle(border_top[edge_len_h-1], border_right[0], edge_top[edge_len_h-1]));
	// Bottom-left
	mesh->triangles.push_back(rendering::Mesh::Triangle(c_bl_idx, border_left[edge_len_v-1], border_bottom[0]));
	mesh->triangles.push_back(rendering::Mesh::Triangle(border_left[edge_len_v-1], edge_left[edge_len_v-1], border_bottom[0]));
	// Bottom-right
	mesh->triangles.push_back(rendering::Mesh::Triangle(c_br_idx, border_bottom[edge_len_h-1], border_right[edge_len_v-1]));
	mesh->triangles.push_back(rendering::Mesh::Triangle(border_bottom[edge_len_h-1], edge_bottom[edge_len_h-1], border_right[edge_len_v-1]));

	// Mask
	rendering::Contour::Handle mask(new rendering::Contour());
	mask->antialias = false;

	Point mask_tl(bound_min_x, bound_max_y);
	Point mask_tr(bound_max_x, bound_max_y);
	Point mask_br(bound_max_x, bound_min_y);
	Point mask_bl(bound_min_x, bound_min_y);

	mask->move_to(rotate_src(mask_tl));
	mask->line_to(rotate_src(mask_tr));
	mask->line_to(rotate_src(mask_br));
	mask->line_to(rotate_src(mask_bl));
	mask->close();
	this->mask = mask;

	this->mesh = mesh;
}
