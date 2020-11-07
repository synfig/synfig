/* === S Y N F I G ========================================================= */
/*!	\file perspective.cpp
**	\brief Implementation of the "Perspective" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2011-2013 Carlos López
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
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <ETL/misc>

#include <synfig/localization.h>

#include <synfig/string.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/transform.h>

#include <synfig/rendering/common/task/tasktransformation.h>
#include <synfig/rendering/common/task/taskcontour.h>
#include <synfig/rendering/common/task/taskblend.h>
#include <synfig/rendering/software/task/tasksw.h>

#include "perspective.h"

#endif

using namespace synfig;
using namespace modules;
using namespace lyr_std;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Perspective);
SYNFIG_LAYER_SET_NAME(Perspective,"warp");
SYNFIG_LAYER_SET_LOCAL_NAME(Perspective,N_("Perspective"));
SYNFIG_LAYER_SET_CATEGORY(Perspective,N_("Distortions"));
SYNFIG_LAYER_SET_VERSION(Perspective,"0.2");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

namespace {
	bool truncate_line(
		Point *out_points,
		const Rect &bounds,
		Real a,
		Real b,
		Real c )
	{
		// equatation of line is a*x + b*y + c = 0
		
		if (!bounds.valid()) return false;
		
		int count = 0;
		
		if (approximate_not_zero(a)) {
			const Real x0 = -(c + bounds.miny*b)/a;
			if ( approximate_greater_or_equal(x0, bounds.minx)
			  && approximate_less_or_equal   (x0, bounds.maxx) )
			{
				if (out_points) out_points[count] = Point(x0, bounds.miny);
				if (count++) return true;
			}
			
			const Real x1 = -(c + bounds.maxy*b)/a;
			if ( approximate_greater_or_equal(x1, bounds.minx)
			  && approximate_less_or_equal   (x1, bounds.maxx) )
			{
				if (out_points) out_points[count] = Point(x1, bounds.maxy);
				if (count++) return true;
			}
		}

		if (approximate_not_zero(b)) {
			const Real y0 = -(c + bounds.minx*a)/b;
			if ( approximate_greater_or_equal(y0, bounds.miny)
			  && approximate_less_or_equal   (y0, bounds.maxy) )
			{
				if (out_points) out_points[count] = Point(bounds.minx, y0);
				if (count++) return true;
			}
			
			const Real y1 = -(c + bounds.maxx*a)/b;
			if ( approximate_greater_or_equal(y1, bounds.miny)
			  && approximate_less_or_equal   (y1, bounds.maxy) )
			{
				if (out_points) out_points[count] = Point(bounds.maxx, y1);
				if (count++) return true;
			}
		}
		
		return false;
	}
	
	Matrix3
	make_matrix(
		const Vector &p0,
		const Vector &px,
		const Vector &py,
		const Vector &p1 )
	{
		// source rect corners are:          (0, 0), (1, 0), (1, 1), (0, 1)
		// target quadrilateral corners are:     p0,     px,     p1,     py
		// so
		// vector will (1, 0) mapped to p0->px
		// vector will (0, 1) mapped to p0->py
		// vector will (1, 1) mapped to p0->p1
		
		const Vector A = px - p1;
		const Vector B = py - p1;
		const Vector C = p0 + p1 - px - py;
		
		Real cw = A[1]*B[0] - A[0]*B[1];
		Real aw = B[0]*C[1] - B[1]*C[0];
		Real bw = A[1]*C[0] - A[0]*C[1];
		
		// normalize and force cw to be positive
		Real k = aw*aw + bw*bw + cw*cw;
		k = k > real_precision<Real>() * real_precision<Real>() ? 1/sqrt(k) : 1;
		if (cw < 0) k = -k;
		aw *= k;
		bw *= k;
		cw *= k;
		
		const Vector c = p0*cw;
		const Vector a = px*(cw + aw) - c;
		const Vector b = py*(cw + bw) - c;

		return Matrix3( a[0], a[1], aw,
						b[0], b[1], bw,
						c[0], c[1], cw );
	}

	Vector3 make_alpha_matrix_col(
		Real w0,
		Real w1,
		const Vector3 &w_col )
	{
		Real k = w1 - w0;
		if (approximate_zero(k))
			return w_col;
		k = w1/k;
		return Vector3(
			k*w_col[0],
			k*w_col[1],
			k*(w_col[2] - w0) );
	}


	Matrix make_alpha_matrix(
		Real aw0, Real aw1,
		Real bw0, Real bw1,
		const Vector3 &w_col )
	{
		const Vector3 a_col = make_alpha_matrix_col(aw0, aw1, w_col);
		const Vector3 b_col = make_alpha_matrix_col(bw0, bw1, w_col);
		return Matrix3(
			a_col[0], b_col[0], w_col[0],
			a_col[1], b_col[1], w_col[1],
			a_col[2], b_col[2], w_col[2] );
	}
	
	
	class OptimalResolutionSolver {
	private:
		Matrix matrix;
		bool affine;
		Vector affine_resolution;
		Vector focus_a;
		Vector focus_b;
		Vector focus_m;
		Vector fp_kw;
		Vector dir;
		Real len;
		
	public:
		explicit OptimalResolutionSolver(const Matrix &matrix):
			affine(), len()
		{
			this->matrix = matrix;
			
			// w-horizon line equatation is A[2]*x + B[2]*y + C[2] = w
			const Vector3 &A = this->matrix.row_x();
			const Vector3 &B = this->matrix.row_y();
			const Vector3 &C = this->matrix.row_z();
			
			const Real wsqr = A[2]*A[2] + B[2]*B[2];
			affine = (wsqr <= real_precision<Real>()*real_precision<Real>());
			affine_resolution = approximate_zero(C[2])
							  ? Vector()
							  : rendering::TransformationAffine::calc_optimal_resolution(Matrix2(
									this->matrix.row_x().to_2d()/C[2],
									this->matrix.row_y().to_2d()/C[2] ));
			const Real wsqr_div = !affine ? 1/wsqr : Real(0);
			
			// focus points
			bool invertible = this->matrix.is_invertible();
			const Matrix back_matrix = this->matrix.get_inverted();
			const bool focus_a_exists = invertible && approximate_not_zero(back_matrix.m02);
			const bool focus_b_exists = invertible && approximate_not_zero(back_matrix.m12);
			const bool focus_m_exists = focus_a_exists && focus_b_exists;
			assert(focus_a_exists || focus_b_exists);
			focus_a = focus_a_exists ? back_matrix.row_x().to_2d()/back_matrix.m02 : Vector();
			focus_b = focus_b_exists ? back_matrix.row_y().to_2d()/back_matrix.m12 : Vector();
			focus_m = focus_m_exists ? (focus_a + focus_b)*0.5
									 : focus_a_exists ? focus_a : focus_b;

			const Vector dist = focus_m_exists ? focus_b - focus_a : Vector();
			len = dist.mag()*0.5;
			dir = approximate_zero(len) ? Vector() : dist/(2*len);
			
			// projection of focus points to w-horizon line
			fp_kw = Vector(A[2], B[2])*wsqr_div;
		}
		
	private:
		Real ratio_for_point(const Vector &point, Real w) const {
			const Vector v = matrix.get_transformed(point);
			const Vector ox( matrix.m00 - matrix.m02*v[0]*w,
							 matrix.m10 - matrix.m12*v[0]*w );
			const Vector oy( matrix.m01 - matrix.m02*v[1]*w,
							 matrix.m11 - matrix.m12*v[1]*w );
			return -ox.mag()-oy.mag();
		}

		Vector resolution_for_point(const Vector &point, Real w) const {
			const Vector v = matrix.get_transformed(point);
			const Matrix2 m(
				Vector( (matrix.m00 - matrix.m02*v[0]*w)*w,
						(matrix.m01 - matrix.m02*v[1]*w)*w ),
				Vector( (matrix.m10 - matrix.m12*v[0]*w)*w,
						(matrix.m11 - matrix.m12*v[1]*w)*w ));
			return rendering::TransformationAffine::calc_optimal_resolution(m);
		}
		
		// returns Vector(l, ratio)
		Vector find_max(const Vector &point, const Vector &dir, Real maxl, Real w) const {
			if (maxl <= 1 || maxl >= 1e+10)
				return Vector(0, ratio_for_point(point, w));
			
			Real l0  = 0;
			Real l1  = maxl;
			Real ll0 = (l0 + l1)*0.5;
			Real vv0 = ratio_for_point(point + dir*ll0, w);
			while(l1 - l0 > 1) {
				Real ll1, vv1;
				if (ll0 - l0 < l1 - ll0) {
					ll1 = (ll0 + l1)*0.5;
					vv1 = ratio_for_point(point + dir*ll1, w);
				} else {
					ll1 = ll0;
					vv1 = vv0;
					ll0 = (l0 + ll0)*0.5;
					vv0 = ratio_for_point(point + dir*ll0, w);
				}
				
				if (vv0 > vv1) {
					l1 = ll1;
				} else {
					l0 = ll0;
					ll0 = ll1;
					vv0 = vv1;
				}
			}
			
			return Vector(ll0, vv0);
		}
	
	public:
		Vector solve(Real w) const {
			if (affine)
				return affine_resolution;
			if (w < real_precision<Real>())
				return Vector();
			
			Vector center;
			const Vector offset_w = fp_kw/w;
			if (len <= 1) {
				center = focus_m + offset_w;
			} else {
				const Vector solution_a = find_max(focus_a + offset_w,  dir, len, w);
				const Vector solution_b = find_max(focus_b + offset_w, -dir, len, w);
				center = solution_a[1] > solution_b[1]
					   ? focus_a + offset_w + dir*solution_a[0]
					   : focus_b + offset_w - dir*solution_b[0];
			}
			
			return resolution_for_point(center, w);
		}
	};
	
	
	class TransformationPerspective: public rendering::Transformation
	{
	public:
		typedef etl::handle<TransformationPerspective> Handle;
		
		class Layer {
		public:
			Rect orig_rect;
			Bounds bounds;
			Matrix alpha_matrix;

			explicit Layer(
				const Rect &orig_rect = Rect(),
				const Bounds &bounds = Bounds(),
				const Matrix &alpha_matrix = Matrix()
			):
				orig_rect(orig_rect),
				bounds(bounds),
				alpha_matrix(alpha_matrix)
			{ }
		};
		
		typedef std::vector<Layer> LayerList;
		
		Matrix matrix;
		
		TransformationPerspective() { }
		explicit TransformationPerspective(const Matrix &matrix): matrix(matrix) { }
		TransformationPerspective& operator= (const TransformationPerspective& other) { matrix = other.matrix; return *this; }
		
	protected:
		virtual Transformation* clone_vfunc() const
			{ return new TransformationPerspective(matrix); }
		
		virtual Transformation* create_inverted_vfunc() const
			{ return new TransformationPerspective(matrix.get_inverted()); }
		
		virtual Point transform_vfunc(const Point &x) const
			{ return (matrix*Vector3(x[0], x[1], 1)).divide_z().to_2d(); }
		
		virtual Matrix2 derivative_vfunc(const Point &x) const {
			Real w = matrix.m02*x[0] + matrix.m12*x[1] + matrix.m22;
			w = approximate_zero(w) ? Real(0) : 1/w;
			return Matrix2(matrix.m00*w, matrix.m01*w, matrix.m10*w, matrix.m11*w);
		}
		
		virtual Bounds transform_bounds_vfunc(const Bounds &bounds) const
			{ return transform_bounds_perspective(matrix, bounds); }
		
		virtual bool can_merge_outer_vfunc(const Transformation &other) const {
			return (bool)dynamic_cast<const TransformationPerspective*>(&other)
				|| (bool)dynamic_cast<const rendering::TransformationAffine*>(&other);
		}
		
		virtual bool can_merge_inner_vfunc(const Transformation &other) const
			{ return can_merge_outer_vfunc(other); }
			
		virtual void merge_outer_vfunc(const Transformation &other) {
			if (const TransformationPerspective *perspective = dynamic_cast<const TransformationPerspective*>(&other)) {
				matrix = perspective->matrix * matrix;
			} else
			if (const rendering::TransformationAffine *affine = dynamic_cast<const rendering::TransformationAffine*>(&other)) {
				matrix = affine->matrix * matrix;
			} else {
				assert(false);
			}
		}
		
		virtual void merge_inner_vfunc(const Transformation &other) {
			if (const TransformationPerspective *perspective = dynamic_cast<const TransformationPerspective*>(&other)) {
				matrix *= perspective->matrix;
			} else
			if (const rendering::TransformationAffine *affine = dynamic_cast<const rendering::TransformationAffine*>(&other)) {
				matrix *= affine->matrix;
			} else {
				assert(false);
			}
		}
		
	public:
		void transform_bounds_layered(
			LayerList &out_layers,
			const Bounds &bounds,
			Real step ) const
		{
			transform_bounds_layered(out_layers, matrix, bounds, step);
		}
		
		static Bounds transform_bounds_perspective(
			const Matrix &matrix,
			const Bounds &bounds )
		{
			if (!bounds.rect.valid())
				return Bounds();
			
			const Matrix norm_matrix = matrix.get_normalized_by_det();
			const Real A = norm_matrix.m02;
			const Real B = norm_matrix.m12;
			const Real C = norm_matrix.m22;
			if (A*A + B*B <= real_precision<Real>() * real_precision<Real>()) {
				const Real C_inv = approximate_zero(C) ? Real(0) : 1/C;
				return rendering::TransformationAffine::transform_bounds_affine(norm_matrix*C_inv, bounds);
			}
			
			const Real horizonw = real_precision<Real>();
			
			Bounds out_bounds;
			
			Real minw =  INFINITY;
			Real maxw = -INFINITY;
			bool found = false;
			const Vector3 in_corners[4] = {
				Vector3(bounds.rect.minx, bounds.rect.miny, 1),
				Vector3(bounds.rect.minx, bounds.rect.maxy, 1),
				Vector3(bounds.rect.maxx, bounds.rect.miny, 1),
				Vector3(bounds.rect.maxx, bounds.rect.maxy, 1) };
			for(int i = 0; i < 4; ++i) {
				const Vector3 v = norm_matrix*in_corners[i];
				const Real w = v[2];
				if (w > horizonw) {
					const Real k = 1/w;
					const Vector p(v[0]*k, v[1]*k);
					if (found) out_bounds.rect.expand(p);
						  else out_bounds.rect.set_point(p);
					found = true;
					if (w < minw) minw = w;
					if (w > maxw) maxw = w;
				}
			}
			if (!found)
				return Bounds();
			
			if (horizonw >= real_precision<Real>()) {
				Vector line[2];
				if (truncate_line(line, bounds.rect, A, B, C - horizonw)) {
					const Real horizonw_div = 1/horizonw;
					for(int i = 0; i < 2; ++i) {
						Vector3 v = norm_matrix*Vector3(line[i][0], line[i][1], 1);
						out_bounds.rect.expand( Vector(v[0]*horizonw_div, v[1]*horizonw_div) );
					}
					minw = horizonw;
				}
			}
			
			const Real midw = exp((log(minw) + log(maxw))*0.5);
			const Real krx = 1/bounds.resolution[0];
			const Real kry = 1/bounds.resolution[1];
			const OptimalResolutionSolver resolution_solver( norm_matrix*Matrix().set_scale(krx, kry) );
			out_bounds.resolution = resolution_solver.solve(midw);
			
			return out_bounds;
		}
		
		static void transform_bounds_layered(
			LayerList &out_layers,
			const Matrix &matrix,
			const Bounds &bounds,
			Real step )
		{
			if (!bounds.is_valid())
				return;
			
			const Matrix norm_matrix = matrix.get_normalized_by_det();
			step = std::max(Real(1.1), step);
			
			// calc coefficient for equatation of "horizontal" line: A*x + B*y + C = 1/w (aka A*x + B*y = 1/w - C)
			//                     equatation of line of horizon is: A*x + B*y + C = 0   (aka A*x + B*y = -C)
			const Real A = norm_matrix.m02;
			const Real B = norm_matrix.m12;
			const Real C = norm_matrix.m22;
			
			const Real krx = 1/bounds.resolution[0];
			const Real kry = 1/bounds.resolution[1];
			
			const Real hd = sqrt(A*A*krx*krx + B*B*kry*kry);
			if (hd <= real_precision<Real>()) {
				if (fabs(C) < real_precision<Real>())
					return; // matrix is not invertible
				
				// orthogonal projection, no perspective, no subdiviosions, just make single layer
				out_layers.push_back(Layer(
					bounds.rect,
					rendering::TransformationAffine::transform_bounds_affine(norm_matrix*(1/C), bounds),
					Matrix3( 0, 0, 0, // alpha always one
							 0, 0, 0,
							 1, 1, 1 ) ));
				return;
			}

			// resolution solver
			const OptimalResolutionSolver resolution_solver( norm_matrix*Matrix().set_scale(krx, kry) );

			// corners
			Vector3 corners[4] = {
				Vector3(bounds.rect.minx, bounds.rect.miny, 1),
				Vector3(bounds.rect.minx, bounds.rect.maxy, 1),
				Vector3(bounds.rect.maxx, bounds.rect.miny, 1),
				Vector3(bounds.rect.maxx, bounds.rect.maxy, 1) };
			
			// find visible w range
			const Real horizonw1_inv = hd;
			const Real horizonw1 = 1/hd;
			const Real horizonw2 = 1/(hd*std::min(Real(2), step));
			const Real horizonw3 = 1/(hd*step);
			Real maxw = -INFINITY, minw = INFINITY;
			Vector3 transformed_corners[4];
			for(int i = 0; i < 4; ++i) {
				Vector3 p = norm_matrix * corners[i];
				if (approximate_greater(p[2], horizonw1_inv)) {
					Real w = 1/p[2];
					transformed_corners[i] = Vector3(p[0]*w, p[1]*w, w);
					if (minw > w) minw = w;
					if (maxw < w) maxw = w;
				} else {
					maxw = horizonw1;
				}
			}
			if (approximate_greater_or_equal(minw, maxw))
				return; // all bounds too thin
			const Real maxw3 = std::min(maxw, horizonw3);

			// steps
			const Real stepLog = log(step);
			int minlog = (int)approximate_floor( log(minw)/stepLog );
			int maxlog = (int)approximate_ceil( log(maxw3)/stepLog );
			if (maxlog < minlog) maxlog = minlog;
			
			Real w = pow(step, Real(minlog));
			for(int i = minlog; i <= maxlog; ++i, w *= step) {
				// w range
				const Real w0  = w/step;
				const Real w1  = std::min(w*step, horizonw1);
				
				// alpha ranges
				const Real aw0 = w0;
				const Real aw1 = w;
				const Real bw0 = i == maxlog ? horizonw1 : w1;
				const Real bw1 = i == maxlog ? horizonw2 : w;
				
				// find corners of layer region
				int corners_count = 0;
				Vector layer_corners[8];
				Vector layer_corners_orig[8];
				for(int j = 0; j < 4; ++j) {
					if ( transformed_corners[j][2]
					  && approximate_greater_or_equal(transformed_corners[j][2], w0)
					  && approximate_less_or_equal(transformed_corners[j][2], w1) )
					{
						layer_corners[corners_count] = transformed_corners[j].to_2d();
						layer_corners_orig[corners_count] = corners[j].to_2d();
						++corners_count;
					}
				}
				if (truncate_line(layer_corners_orig + corners_count, bounds.rect, A, B, C - 1/w0)) {
					for(int j = 0; j < 2; ++j, ++corners_count)
						layer_corners[corners_count] =
							( norm_matrix * Vector3(
								layer_corners_orig[corners_count][0],
								layer_corners_orig[corners_count][1], 1 )).to_2d() * w0;
				}
				if (truncate_line(layer_corners_orig + corners_count, bounds.rect, A, B, C - 1/w1)) {
					for(int j = 0; j < 2; ++j, ++corners_count)
						layer_corners[corners_count] =
							( norm_matrix * Vector3(
								layer_corners_orig[corners_count][0],
								layer_corners_orig[corners_count][1], 1 )).to_2d() * w1;
				}
				if (!corners_count)
					continue;
				
				// calc bounds
				Rect layer_rect(layer_corners[0]);
				Rect layer_rect_orig(layer_corners_orig[0]);
				for(int j = 1; j < corners_count; ++j) {
					layer_rect.expand(layer_corners[j]);
					layer_rect_orig.expand(layer_corners_orig[j]);
				}
				if (!layer_rect.valid() || !layer_rect_orig.valid())
					continue;
				
				// calc resolution
				Vector resolution = resolution_solver.solve(w);
				if (resolution[0] <= real_precision<Real>() || resolution[1] <= real_precision<Real>())
					continue;
				
				// make layer
				out_layers.push_back( Layer(
					layer_rect_orig,
					Bounds(
						layer_rect,
						resolution ),
					make_alpha_matrix(
						1/aw0, 1/aw1, 1/bw0, 1/bw1,
						Vector3(norm_matrix.m02, norm_matrix.m12, norm_matrix.m22) ) ));
			}
		}
	};
	
	
	class TaskTransformationPerspective: public rendering::TaskTransformation
	{
	public:
		typedef etl::handle<TaskTransformationPerspective> Handle;
		static Token token;
		virtual Token::Handle get_token() const { return token.handle(); }

	protected:
		TransformationPerspective::LayerList layers;

	public:
		rendering::Holder<TransformationPerspective> transformation;

		virtual rendering::Transformation::Handle get_transformation() const
			{ return transformation.handle(); }

		virtual int get_pass_subtask_index() const {
			if (is_simple() && transformation->matrix.is_identity())
				return 0;
			return TaskTransformation::get_pass_subtask_index();
		}
		
		virtual void set_coords_sub_tasks() {
			const Real step = 2;

			const Task::Handle task = sub_task();
			const rendering::Transformation::Bounds bounds(
				source_rect, get_pixels_per_unit().multiply_coords(supersample));
			const Matrix back_matrix = transformation->matrix
									  .get_normalized_by_det()
									  .get_inverted();

			sub_tasks.clear();
			layers.clear();
			
			TransformationPerspective::LayerList new_layers;
			
			if ( task
			  && is_valid_coords()
			  && approximate_greater(supersample[0], Real(0))
			  && approximate_greater(supersample[1], Real(0)) )
			{
				TransformationPerspective::transform_bounds_layered(new_layers, back_matrix, bounds, step);
			}
			
			Rect sum_rect;
			for(TransformationPerspective::LayerList::const_iterator i = new_layers.begin(); i != new_layers.end(); ++i) {
				rendering::Transformation::DiscreteBounds discrete_bounds =
					rendering::Transformation::make_discrete_bounds( i->bounds );
				if (discrete_bounds.is_valid()) {
					Task::Handle t = task->clone_recursive();
					sub_tasks.push_back(t);
					layers.push_back(*i);
					t->set_coords(discrete_bounds.rect, discrete_bounds.size);
					
					sum_rect |= i->orig_rect;
				}
			}
			trunc_source_rect(sum_rect);
			
			if (sub_tasks.empty()) trunc_to_zero();
		}
	};
	
	
	class TaskTransformationPerspectiveSW: public TaskTransformationPerspective, public rendering::TaskSW
	{
	public:
		typedef etl::handle<TaskTransformationPerspectiveSW> Handle;
		static Token token;
		virtual Token::Handle get_token() const { return token.handle(); }

	private:
		template<synfig::Surface::sampler_cook::func func, bool interpolate = true>
		static void process_layer(
			synfig::Surface &dst_surface,
			const synfig::Surface &src_surface,
			const RectInt &rect,
			const Matrix &matrix,
			const Matrix &alpha_matrix )
		{
			const int width = rect.get_width();
			
			const Vector3 coord_dx = matrix.row_x();
			const Vector3 coord_dy = matrix.row_y() - coord_dx*width;
			Vector3 coord = matrix*Vector3(rect.minx, rect.miny, 1);
			
			// assume that alpha w coord is equal to coord w
			const Vector alpha_dx = alpha_matrix.row_x().to_2d();
			const Vector alpha_dy = alpha_matrix.row_y().to_2d() - alpha_dx*width;
			Vector alpha = (alpha_matrix*Vector3(rect.minx, rect.miny, 1)).to_2d();
			
			const int dr = dst_surface.get_pitch()/sizeof(Color) - width;
			Color *c = &dst_surface[rect.miny][rect.minx];
			
			for(int r = rect.miny; r < rect.maxy; ++r, c += dr, coord += coord_dy, alpha += alpha_dy) {
				for(Color *end = c + width; c != end; ++c, coord += coord_dx, alpha += alpha_dx) {
					if (coord[2] > real_precision<Real>()) {
						const Real w = 1/coord[2];
						const Real a = clamp(alpha[0]*w, Real(0), Real(1)) * clamp(alpha[1]*w, Real(0), Real(1));
						if (interpolate) {
							if (a > real_precision<Real>())
								*c += func(&src_surface, coord[0]*w, coord[1]*w).premult_alpha() * a;
						} else {
							if (approximate_greater(a, Real(0.5)))
								*c = func(&src_surface, coord[0]*w, coord[1]*w);
						}
					}
				}
			}
		}

	public:
		virtual bool run(RunParams&) const
		{
			if (!is_valid())
				return true;

			LockWrite ldst(this);
			if (!ldst)
				return false;
			synfig::Surface &dst_surface = ldst->get_surface();

			// target rect
			const RectInt base_rect = RectInt(0, 0, dst_surface.get_w(), dst_surface.get_h()) & target_rect;
			if (!base_rect.is_valid())
				return true;

			// transformation matrix
			const Matrix from_pixels_matrix =
			    Matrix3().set_translate( source_rect.get_min() )
			  * Matrix3().set_scale( get_units_per_pixel() )
			  * Matrix3().set_translate( -target_rect.minx, -target_rect.miny );
			const Matrix to_pixels_matrix = from_pixels_matrix.get_inverted();
			const Matrix back_matrix =
				transformation->matrix
			   .get_normalized_by_det()
			   .get_inverted();
			const Matrix base_matrix =
				back_matrix
			  * from_pixels_matrix;

			// process layers
			bool demult = false;
			TransformationPerspective::LayerList::const_iterator li = layers.begin();
			for(List::const_iterator i = sub_tasks.begin(); i != sub_tasks.end() && li != layers.end(); ++i, ++li) {
				if (!*i) continue;
				
				// rect
				const Rect orig_rect = li->orig_rect & source_rect;
				Rect rect_float(
					to_pixels_matrix.get_transformed(orig_rect.get_min()),
					to_pixels_matrix.get_transformed(orig_rect.get_max()) );
				if (!rect_float.is_valid())
					continue;
				
				// get source
				LockRead lsrc(*i);
				if (!lsrc)
					continue;
				const synfig::Surface &src_surface = lsrc->get_surface();
				
				// matrix
				const Matrix matrix =
					Matrix3().set_translate( (*i)->target_rect.minx, (*i)->target_rect.miny )
				  * Matrix3().set_scale( (*i)->get_pixels_per_unit() )
				  * Matrix3().set_translate( -(*i)->source_rect.get_min() )
				  * base_matrix;
				const Matrix alpha_matrix =
					li->alpha_matrix
				  * from_pixels_matrix;
				
				// truncate rect by sub_task rect
				rect_float &= TransformationPerspective::transform_bounds_perspective(
						matrix.get_inverted(),
						rendering::Transformation::Bounds(
							Rect( (*i)->target_rect.minx, (*i)->target_rect.miny,
								  (*i)->target_rect.maxx, (*i)->target_rect.maxy ),
							Vector(1, 1) )
					).rect;
				
				const RectInt rect = RectInt(
					(int)approximate_floor(rect_float.minx),
					(int)approximate_floor(rect_float.miny),
					(int)approximate_ceil (rect_float.maxx),
					(int)approximate_ceil (rect_float.maxy) ) & base_rect;
				if (!rect.is_valid())
					continue;
				  
				// process
				switch(interpolation) {
					case Color::INTERPOLATION_LINEAR:
						process_layer<synfig::Surface::sampler_cook::linear_sample>(
							dst_surface, src_surface, rect, matrix, alpha_matrix );
						demult = true;
						break;
					case Color::INTERPOLATION_COSINE:
						process_layer<synfig::Surface::sampler_cook::cosine_sample>(
							dst_surface, src_surface, rect, matrix, alpha_matrix );
						demult = true;
						break;
					case Color::INTERPOLATION_CUBIC:
						process_layer<synfig::Surface::sampler_cook::cubic_sample>(
							dst_surface, src_surface, rect, matrix, alpha_matrix );
						demult = true;
						break;
					default: // nearest
						process_layer<synfig::Surface::sampler_cook::nearest_sample, false>(
							dst_surface, src_surface, rect, matrix, alpha_matrix );
						break;
				};
			}
			
			// demult alpha
			if (demult) {
				const int width = base_rect.get_width();
				const int dr = dst_surface.get_pitch()/sizeof(Color) - width;
				Color *c = &dst_surface[base_rect.miny][base_rect.minx];
				for(int r = base_rect.miny; r < base_rect.maxy; ++r, c += dr)
					for(Color *end = c + width; c != end; ++c)
						*c = c->demult_alpha();
			}
			
			return true;
		}
	};
	
	
	rendering::Task::Token TaskTransformationPerspective::token(
		DescAbstract<TaskTransformationPerspective>("TransformationPerspective") );
	rendering::Task::Token TaskTransformationPerspectiveSW::token(
		DescReal< TaskTransformationPerspectiveSW,
				  TaskTransformationPerspective >
					("TaskTransformationPerspectiveSW") );
}


class lyr_std::Perspective_Trans: public Transform
{
private:
	etl::handle<const Perspective> layer;
public:
	Perspective_Trans(const Perspective* x):
		Transform(x->get_guid()), layer(x) { }
	Vector perform(const Vector& x) const
		{ return layer->transform(x); }
	Vector unperform(const Vector& x) const
		{ return layer->back_transform(x); }
	Rect perform(const Rect& x) const
		{ return layer->transform(x); }
	Rect unperform(const Rect& x) const
		{ return layer->back_transform(x); }
	String get_string() const
		{ return "perspective"; }
};


Perspective::Perspective():
	param_src_tl  ( Point(-2  ,  2  ) ),
	param_src_br  ( Point( 2  , -2  ) ),
	param_dest_tl ( Point(-1.8,  2.1) ),
	param_dest_tr ( Point( 1.8,  2.1) ),
	param_dest_bl ( Point(-2.2, -2  ) ),
	param_dest_br ( Point( 2.2, -2  ) ),
	param_clip    ( true ),
	param_interpolation( int(3) ),
	valid         ( ),
	affine        ( ),
	clip          ( )
{
	sync();
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

Perspective::~Perspective()
	{ }

void
Perspective::sync()
{
	valid = false;
	
	const Point src_tl  = param_src_tl.get(Point());
	const Point src_br  = param_src_br.get(Point());
	const Point dest_tl = param_dest_tl.get(Point());
	const Point dest_tr = param_dest_tr.get(Point());
	const Point dest_bl = param_dest_bl.get(Point());
	const Point dest_br = param_dest_br.get(Point());
	
	clip = param_clip.get(bool());
	clip_rect = Rect(src_tl, src_br);
	
	const Vector src_dist = src_br - src_tl;
	if (approximate_not_zero(src_dist[0]) && approximate_not_zero(src_dist[1])) {
		matrix = make_matrix( dest_tl, dest_tr, dest_bl, dest_br )
			   * Matrix().set_scale( 1/src_dist[0], 1/src_dist[1] ) 
			   * Matrix().set_translate( -src_tl );
		if (matrix.is_invertible()) {
			back_matrix = matrix.get_normalized_by_det().get_inverted();
			affine = approximate_zero(matrix.m02) && approximate_zero(matrix.m12);
			valid = true;
		}
	}
	
	if (clip && !clip_rect.is_valid() && !clip_rect.is_nan_or_inf())
		valid = false;
}

Point
Perspective::transform(const Point &x) const
{
	if (!valid) return Vector::nan();
	Vector3 p = matrix*Vector3(x[0], x[1], 1);
	return p[2] > real_precision<Real>() ? p.to_2d()/p[2] : Vector::nan();
}

Point
Perspective::back_transform(const Point &x) const
{
	if (!valid) return Vector::nan();
	Vector3 p = back_matrix*Vector3(x[0], x[1], 1);
	return p[2] > real_precision<Real>() ? p.to_2d()/p[2] : Vector::nan();
}

Rect
Perspective::transform(const Rect &x) const
{
	return valid
		 ? TransformationPerspective::transform_bounds_perspective(
			 matrix, rendering::Transformation::Bounds(x)).rect
		 : Rect();
}

Rect
Perspective::back_transform(const Rect &x) const
{
	return valid
		 ? TransformationPerspective::transform_bounds_perspective(
			 back_matrix, rendering::Transformation::Bounds(x)).rect
		 : Rect();
}

Layer::Handle
Perspective::hit_check(Context context, const Point &p)const
{
	if (!valid) return Layer::Handle();
	const Point pp = back_transform(p);
	if (clip && !clip_rect.is_inside(pp)) return Layer::Handle();
	return context.hit_check(pp);
}

Color
Perspective::get_color(Context context, const Point &p)const
{
	if (!valid) return Color::alpha();
	const Point pp = back_transform(p);
	if (clip && !clip_rect.is_inside(pp)) return Color::alpha();
	return context.get_color(pp);
}

Rect
Perspective::get_bounding_rect() const
	{ return Rect(); }

Rect
Perspective::get_full_bounding_rect(Context context)const
{
	if (!valid)
		return Rect();
	Rect sub_rect = context.get_full_bounding_rect();
	sub_rect |= get_bounding_rect();
	if (clip)
		sub_rect &= clip_rect;
	return transform(sub_rect);
}

bool
Perspective::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE_PLUS(param_src_tl ,sync());
	IMPORT_VALUE_PLUS(param_src_br ,sync());
	IMPORT_VALUE_PLUS(param_dest_tl,sync());
	IMPORT_VALUE_PLUS(param_dest_tr,sync());
	IMPORT_VALUE_PLUS(param_dest_bl,sync());
	IMPORT_VALUE_PLUS(param_dest_br,sync());
	IMPORT_VALUE_PLUS(param_clip   ,sync());
	IMPORT_VALUE(param_interpolation);
	return false;
}

ValueBase
Perspective::get_param(const String &param)const
{
	EXPORT_VALUE(param_src_tl);
	EXPORT_VALUE(param_src_br);
	EXPORT_VALUE(param_dest_tl);
	EXPORT_VALUE(param_dest_tr);
	EXPORT_VALUE(param_dest_bl);
	EXPORT_VALUE(param_dest_br);
	EXPORT_VALUE(param_clip);
	EXPORT_VALUE(param_interpolation);

	EXPORT_NAME();
	EXPORT_VERSION();

	return ValueBase();
}

Layer::Vocab
Perspective::get_param_vocab()const
{
	Layer::Vocab ret;

	ret.push_back(ParamDesc("src_tl")
		.set_local_name(_("Source TL"))
		.set_box("src_br")
		.set_description(_("Top Left corner of the source to perspective"))
		.set_is_distance()
	);

	ret.push_back(ParamDesc("src_br")
		.set_local_name(_("Source BR"))
		.set_description(_("Bottom Right corner of the source to perspective"))
		.set_is_distance()
	);

	ret.push_back(ParamDesc("dest_tl")
		.set_local_name(_("Dest TL"))
		.set_connect("dest_tr")
		.set_description(_("Top Left corner of the destination"))
		.set_is_distance()
	);

	ret.push_back(ParamDesc("dest_tr")
		.set_local_name(_("Dest TR"))
		.set_connect("dest_br")
		.set_description(_("Top Right corner of the destination"))
		.set_is_distance()
	);

	ret.push_back(ParamDesc("dest_br")
		.set_local_name(_("Dest BR"))
		.set_connect("dest_bl")
		.set_description(_("Bottom Right corner of the destination"))
		.set_is_distance()
	);

	ret.push_back(ParamDesc("dest_bl")
		.set_local_name(_("Dest BL"))
		.set_connect("dest_tl")
		.set_description(_("Bottom Left corner of the destination"))
		.set_is_distance()
	);

	ret.push_back(ParamDesc("clip")
		.set_local_name(_("Clip"))
	);
	
	ret.push_back(ParamDesc("interpolation")
		.set_local_name(_("Interpolation"))
		.set_description(_("Type of interpolation to use"))
		.set_hint("enum")
		.add_enum_value(0,"nearest",_("Nearest Neighbor"))
		.add_enum_value(1,"linear",_("Linear"))
		.add_enum_value(2,"cosine",_("Cosine"))
		.add_enum_value(3,"cubic",_("Cubic"))
		.set_static(true)
	);

	return ret;
}

etl::handle<Transform>
Perspective::get_transform()const
	{ return new Perspective_Trans(this); }

rendering::Task::Handle
Perspective::build_rendering_task_vfunc(Context context) const
{
	const Color::Interpolation interpolation = (Color::Interpolation)param_interpolation.get(int());
	
	if (!valid)
		return rendering::Task::Handle();
	
	rendering::Task::Handle sub_task = context.build_rendering_task();
	if (!sub_task)
		return rendering::Task::Handle();
	
	if (clip) {
		rendering::TaskContour::Handle task_contour(new rendering::TaskContour());
		task_contour->contour = new rendering::Contour();
		task_contour->contour->move_to( Vector(clip_rect.minx, clip_rect.miny) );
		task_contour->contour->line_to( Vector(clip_rect.minx, clip_rect.maxy) );
		task_contour->contour->line_to( Vector(clip_rect.maxx, clip_rect.maxy) );
		task_contour->contour->line_to( Vector(clip_rect.maxx, clip_rect.miny) );
		task_contour->contour->close();
		task_contour->contour->color = Color(1, 1, 1, 1);
		task_contour->contour->invert = true;
		task_contour->contour->antialias = interpolation != Color::INTERPOLATION_NEAREST;

		rendering::TaskBlend::Handle task_blend(new rendering::TaskBlend());
		task_blend->amount = 1;
		task_blend->blend_method = Color::BLEND_ALPHA_OVER;
		task_blend->sub_task_a() = sub_task;
		task_blend->sub_task_b() = task_contour;
		
		sub_task = task_blend;
	}
	
	if (affine) {
		rendering::TaskTransformationAffine::Handle task_transformation(new rendering::TaskTransformationAffine());
		task_transformation->interpolation = interpolation;
		task_transformation->transformation->matrix = matrix;
		task_transformation->sub_task() = sub_task;
		return task_transformation;
	}
	
	TaskTransformationPerspective::Handle task_transformation(new TaskTransformationPerspective());
	task_transformation->interpolation = interpolation;
	task_transformation->transformation->matrix = matrix;
	task_transformation->sub_task() = sub_task;
	return task_transformation;
}
