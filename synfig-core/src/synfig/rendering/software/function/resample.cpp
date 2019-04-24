/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/function/resample.cpp
**	\brief Resample
**
**	$Id$
**
**	\legal
**	......... ... 2015-2019 Ivan Mahonin
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/general.h>
#include <synfig/localization.h>

#include <synfig/debug/debugsurface.h>

#include "resample.h"
#include "../../primitive/transformationaffine.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

namespace {
	class Helper
	{
	public:
		struct MapPixelFull { int src; int dst; };
		struct MapPixelPart { int src; int dst; ColorReal k0; ColorReal k1; };

		template< Color reader(const void*,int,int),
				ColorAccumulator reader_cook(const void*,int,int) >
		class Generic {
		public:
			typedef synfig::Surface::sampler<Color, reader> Sampler;
			typedef synfig::Surface::sampler<ColorAccumulator, reader_cook> SamplerCook;
			typedef typename Sampler::coord_type Coord;
			typedef typename Sampler::func SamplerFunc;
			typedef typename SamplerCook::func SamplerCookFunc;

			struct Iterator {
				const void *surface;
				const RectInt &bounds;
				Vector pos, pos_dx, pos_dy;
				Vector aa0, aa0_dx, aa0_dy;
				Vector aa1, aa1_dx, aa1_dy;
				Iterator(const void *surface, const RectInt &bounds):
					surface(surface), bounds(bounds) { }
			};

			template<SamplerCookFunc sampler_func>
			static inline Color uncook(const void *surface, Coord x, Coord y)
				{ return ColorPrep::uncook_static( sampler_func(surface, x, y) ); }

			template<typename pen, SamplerFunc sampler_func>
			static inline void fill(pen &p, Iterator &i)
			{
				int idx = i.bounds.maxx - i.bounds.minx;
				int idy = i.bounds.maxy - i.bounds.miny;
				for(int y = idy; y; --y) {
					for(int x = idx; x; --x) {
						p.put_value( sampler_func(i.surface, i.pos[0], i.pos[1]) );
						p.inc_x();
						i.pos += i.pos_dx;
					}
					p.dec_x(idx); p.inc_y();
					i.pos += i.pos_dy;
				}
			}

			template<typename pen, SamplerFunc sampler_func>
			static inline void fill_cut(pen &p, Iterator &i)
			{
				const Real threshold = 0.5 - 1e-4;
				int idx = i.bounds.maxx - i.bounds.minx;
				int idy = i.bounds.maxy - i.bounds.miny;
				for(int y = idy; y; --y) {
					for(int x = idx; x; --x) {
						if ( i.aa0[0] > threshold && i.aa0[1] > threshold
						&& i.aa1[0] > threshold && i.aa1[1] > threshold )
							p.put_value( sampler_func(i.surface, i.pos[0], i.pos[1]) );
						i.pos += i.pos_dx;
						i.aa0 += i.aa0_dx;
						i.aa1 += i.aa1_dx;
						p.inc_x();
					}
					p.dec_x(idx); p.inc_y();
					i.pos += i.pos_dy;
					i.aa0 += i.aa0_dy;
					i.aa1 += i.aa1_dy;
				}
			}

			template<typename pen, SamplerFunc sampler_func>
			static inline void fill_aa(pen &p, Iterator &i)
			{
				int idx = i.bounds.maxx - i.bounds.minx;
				int idy = i.bounds.maxy - i.bounds.miny;
				for(int y = idy; y; --y) {
					for(int x = idx; x; --x) {
						if ( i.aa0[0] > 1 && i.aa0[1] > 1
						&& i.aa1[0] > 1 && i.aa1[1] > 1 )
						{
							p.put_value( sampler_func(i.surface, i.pos[0], i.pos[1]) );
						} else
						if ( i.aa0[0] > 0 && i.aa0[1] > 0
						&& i.aa1[0] > 0 && i.aa1[1] > 0 )
						{
							Color c = sampler_func(i.surface, i.pos[0], i.pos[1]);
							c.set_a( c.get_a()
								* std::min(i.aa0[0], 1.0)
								* std::min(i.aa0[1], 1.0)
								* std::min(i.aa1[0], 1.0)
								* std::min(i.aa1[1], 1.0) );
							p.put_value(c);
						}
						i.pos += i.pos_dx;
						i.aa0 += i.aa0_dx;
						i.aa1 += i.aa1_dx;
						p.inc_x();
					}
					p.dec_x(idx); p.inc_y();
					i.pos += i.pos_dy;
					i.aa0 += i.aa0_dy;
					i.aa1 += i.aa1_dy;
				}
			}

			template<typename pen, SamplerFunc sampler_func>
			static inline void fill(bool cut, bool antialiasing, pen &p, Iterator &i)
			{
				if (!cut) fill<pen, sampler_func>(p, i); else
					if (antialiasing) fill_aa<pen, sampler_func>(p, i); else
						fill_cut<pen, sampler_func>(p, i);
			}

			template<typename pen>
			static inline void fill(Color::Interpolation interpolation, bool cut, pen &p, Iterator &i)
			{
				bool no_transform =
					approximate_equal(fabs(i.pos_dx[0]), 0.0)
				&& approximate_equal(fabs(i.pos_dx[1]), 1.0)
				&& approximate_equal(fabs(i.pos_dy[0]), Real(i.bounds.minx - i.bounds.maxx))
				&& approximate_equal(fabs(i.pos_dy[1]), 1.0)
				&& approximate_equal(i.pos[0] - 0.5, round(i.pos[0] - 0.5))
				&& approximate_equal(i.pos[1] - 0.5, round(i.pos[1] - 0.5));

				if (no_transform)
					interpolation = Color::INTERPOLATION_NEAREST;

				switch(interpolation)
				{
				case Color::INTERPOLATION_LINEAR:
					fill< pen, uncook<SamplerCook::linear_sample> >(cut, true, p, i); break;
				case Color::INTERPOLATION_COSINE:
					fill< pen, uncook<SamplerCook::cosine_sample> >(cut, true, p, i); break;
				case Color::INTERPOLATION_CUBIC:
					fill< pen, uncook<SamplerCook::cubic_sample> >(cut, true, p, i); break;
				default:
					fill< pen, Sampler::nearest_sample >(cut, false, p, i); break;
				}
			}

			static void resample(
				synfig::Surface &dest,
				const RectInt &dest_bounds,
				const void *src,
				const RectInt &src_bounds,
				const Matrix &transformation,
				Color::Interpolation interpolation,
				bool blend,
				ColorReal blend_amount,
				Color::BlendMethod blend_method )
			{
				// bounds

				Vector corners[] = {
					transformation.get_transformed(Vector( Real(src_bounds.minx), Real(src_bounds.miny) )),
					transformation.get_transformed(Vector( Real(src_bounds.maxx), Real(src_bounds.miny) )),
					transformation.get_transformed(Vector( Real(src_bounds.minx), Real(src_bounds.maxy) )),
					transformation.get_transformed(Vector( Real(src_bounds.maxx), Real(src_bounds.maxy) )) };

				Rect boundsf(   corners[0] );
				boundsf.expand( corners[1] );
				boundsf.expand( corners[2] );
				boundsf.expand( corners[3] );

				RectInt bounds( (int)approximate_floor(boundsf.minx) - 1,
								(int)approximate_floor(boundsf.miny) - 1,
								(int)approximate_ceil (boundsf.maxx) + 1,
								(int)approximate_ceil (boundsf.maxy) + 1 );

				etl::set_intersect(bounds, bounds, dest_bounds);
				etl::set_intersect(bounds, bounds, RectInt(0, 0, dest.get_w(), dest.get_h()));

				// texture matrices

				if (bounds.valid()) {
					Matrix back_transformation = transformation;
					back_transformation.invert();

					Iterator i(src, bounds);

					Vector start((Real)bounds.minx, (Real)bounds.miny);
					Vector dx(1.0, 0.0);
					Vector dy((Real)(bounds.minx - bounds.maxx), 1.0);

					i.pos    = back_transformation.get_transformed( start ) - Vector(0.5, 0.5);
					i.pos_dx = back_transformation.get_transformed( dx, false );
					i.pos_dy = back_transformation.get_transformed( dy, false );

					bool cut = true;

					if (cut) {
						Vector sub_corners[] = {
							back_transformation.get_transformed(Vector( Real(bounds.minx), Real(bounds.miny) )),
							back_transformation.get_transformed(Vector( Real(bounds.maxx), Real(bounds.miny) )),
							back_transformation.get_transformed(Vector( Real(bounds.minx), Real(bounds.maxy) )),
							back_transformation.get_transformed(Vector( Real(bounds.maxx), Real(bounds.maxy) )) };

						Rect sub_boundsf(   sub_corners[0] );
						sub_boundsf.expand( sub_corners[1] );
						sub_boundsf.expand( sub_corners[2] );
						sub_boundsf.expand( sub_corners[3] );

						RectInt sub_bounds(
							(int)approximate_floor(sub_boundsf.minx) - 1,
							(int)approximate_floor(sub_boundsf.miny) - 1,
							(int)approximate_ceil (sub_boundsf.maxx) + 1,
							(int)approximate_ceil (sub_boundsf.maxy) + 1 );

						if (src_bounds.contains(sub_bounds))
							cut = false;
					}

					if (cut) {
						Vector axis_x = (corners[1] - corners[0]).norm();
						Vector axis_y = (corners[2] - corners[0]).norm();
						Matrix aa0_matrix = Matrix( axis_x,  axis_y, corners[0] - (axis_x + axis_y)*0.5).get_inverted();
						Matrix aa1_matrix = Matrix(-axis_x, -axis_y, corners[3] + (axis_x + axis_y)*0.5).get_inverted();

						i.aa0    = aa0_matrix.get_transformed( start );
						i.aa0_dx = aa0_matrix.get_transformed( dx, false );
						i.aa0_dy = aa0_matrix.get_transformed( dy, false );

						i.aa1    = aa1_matrix.get_transformed( start );
						i.aa1_dx = aa1_matrix.get_transformed( dx, false );
						i.aa1_dy = aa1_matrix.get_transformed( dy, false );
					}

					if (blend) {
						if (approximate_equal_lp(blend_amount, ColorReal(0))) return;
						synfig::Surface::alpha_pen p(dest.get_pen(bounds.minx, bounds.miny));
						p.set_blend_method(blend_method);
						p.set_alpha(blend_amount);
						fill(interpolation, cut, p, i);
					} else {
						synfig::Surface::pen p(dest.get_pen(bounds.minx, bounds.miny));
						fill(interpolation, cut, p, i);
					}
				}
			}

			static void build_downscale_pattern(
				MapPixelFull* &full_pixels,
				MapPixelPart* &part_pixels,
				int src0, int src1, int src_pitch,
				int dst0, int dst1, int dst_pitch )
			{
				Real pixel_size = (Real)(src1 - src0)/(Real)(dst1 - dst0);
				Real pixel_size0 = pixel_size - real_low_precision<Real>();
				Real pixel_size1 = pixel_size + real_low_precision<Real>();
				Real pos = 0;
				int j = dst0*dst_pitch, j1 = dst1*dst_pitch;
				for(int i = src0*src_pitch, i1 = src1*src_pitch; i < i1; i += src_pitch) {
					Real next = pos + Real(1.0);
					if (next > pixel_size1) {
						MapPixelPart &part = *(part_pixels++);
						part.src = i;
						part.dst = j;
						part.k0 = (ColorReal)(pixel_size - pos);
						pos = next - pixel_size;
						part.k1 = (ColorReal)pos;
						j += dst_pitch;
						if (j >= j1) break;
					} else
					if (next > pixel_size0) {
						MapPixelFull &full = *(full_pixels++);
						full.src = i;
						full.dst = j;
						pos = next - pixel_size;
						j += dst_pitch;
						if (j >= j1) break;
					} else {
						MapPixelFull &full = *(full_pixels++);
						full.src = i;
						full.dst = j;
						pos = next;
					}
				}
			}

			static void downscale(
				synfig::Surface &dest,
				const RectInt &dest_bounds,
				const void *src,
				const RectInt &src_bounds,
				bool keep_cooked )
			{
				int dw = dest_bounds.get_width();
				int dh = dest_bounds.get_height();
				int sw = src_bounds.get_width();
				int sh = src_bounds.get_height();

				assert(dw > 0 && dh > 0 && dw <= sw && dh <= sh);

				int dest_pitch = dest.get_pitch()/sizeof(Color);

				MapPixelFull full_cols[sw], *full_cols_begin = full_cols, *full_cols_end = full_cols_begin;
				MapPixelPart part_cols[sw], *part_cols_begin = part_cols, *part_cols_end = part_cols_begin;
				build_downscale_pattern( full_cols_end, part_cols_end,
										src_bounds.minx, src_bounds.maxx, 1,
										dest_bounds.minx, dest_bounds.maxx, 1 );

				MapPixelFull full_rows[sh], *full_rows_begin = full_rows, *full_rows_end = full_rows_begin;
				MapPixelPart part_rows[sh], *part_rows_begin = part_rows, *part_rows_end = part_rows_begin;
				build_downscale_pattern( full_rows_end, part_rows_end,
										src_bounds.miny, src_bounds.maxy, 1,
										dest_bounds.miny, dest_bounds.maxy, dest_pitch );

				Color *dest_ptr = dest[dest_bounds.miny] + dest_bounds.minx;

				// full rows
				for(MapPixelFull *fr = full_rows_begin; fr < full_rows_end; ++fr) {
					Color *dest_row = dest_ptr + fr->dst;

					// full cols
					for(MapPixelFull *fc = full_cols_begin; fc < full_cols_end; ++fc)
						*(dest_row + fc->dst) += reader_cook(src, fc->src, fr->src);

					// part cols
					for(MapPixelPart *pc = part_cols_begin; pc < part_cols_end; ++pc) {
						Color color = reader_cook(src, pc->src, fr->src);
						Color *dest_col = dest_row + pc->dst;
						*dest_col += color * pc->k0; ++dest_col;
						*dest_col += color * pc->k1;
					}
				}

				// part rows
				for(MapPixelPart *pr = part_rows_begin; pr < part_rows_end; ++pr) {
					Color *dest_row = dest_ptr + pr->dst;

					// full cols 0
					for(MapPixelFull *fc = full_cols_begin; fc < full_cols_end; ++fc)
						*(dest_row + fc->dst) += reader_cook(src, fc->src, pr->src) * pr->k0;

					// part cols 0
					for(MapPixelPart *pc = part_cols_begin; pc < part_cols_end; ++pc) {
						Color color = reader_cook(src, pc->src, pr->src) * pr->k0;
						Color *dest_col = dest_row + pc->dst;
						*dest_col += color * pc->k0; ++dest_col;
						*dest_col += color * pc->k1;
					}

					dest_row += dest_pitch;

					// full cols 1
					for(MapPixelFull *fc = full_cols_begin; fc < full_cols_end; ++fc)
						*(dest_row + fc->dst) += reader_cook(src, fc->src, pr->src) * pr->k1;

					// part cols 1
					for(MapPixelPart *pc = part_cols_begin; pc < part_cols_end; ++pc) {
						Color color = reader_cook(src, pc->src, pr->src) * pr->k1;
						Color *dest_col = dest_row + pc->dst;
						*dest_col += color * pc->k0; ++dest_col;
						*dest_col += color * pc->k1;
					}
				}

				// postprocess: divide to sub-pixels count and demult alpha
				ColorReal k = (ColorReal)(dw*dh)/(ColorReal)(sw*sh);
				if (keep_cooked)
					for(Color *row = dest_ptr, *row_end = row + dest_pitch*dh; row < row_end; row += dest_pitch)
						for(Color *col = row, *col_end = col + dw; col < col_end; ++col)
							*col *= k;
				else
					for(Color *row = dest_ptr, *row_end = row + dest_pitch*dh; row < row_end; row += dest_pitch)
						for(Color *col = row, *col_end = col + dw; col < col_end; ++col)
							*col = ColorPrep::uncook_static( (*col)*k );
			}

			static void resample_with_downscale(
				synfig::Surface &dest,
				const RectInt &dest_bounds,
				const void *src,
				const RectInt &src_bounds,
				const Matrix &transformation,
				Color::Interpolation interpolation,
				bool blend,
				ColorReal blend_amount,
				Color::BlendMethod blend_method )
			{
				if (interpolation != Color::INTERPOLATION_NEAREST) {
					const Real threshold = 1.2;

					Transformation::Bounds bounds =
						TransformationAffine( transformation.get_inverted() )
							.transform_bounds( Rect(0.0, 0.0, 1.0, 1.0), Vector(1.0, 1.0) );
					bounds.resolution *= threshold;

					int sw = src_bounds.get_width();
					int sh = src_bounds.get_height();
					int w = std::min( sw, std::max(1, (int)ceil((Real)sw * bounds.resolution[0])) );
					int h = std::min( sh, std::max(1, (int)ceil((Real)sh * bounds.resolution[1])) );

					if (w < sw || h < sh) {
						synfig::Surface new_src(w, h);
						downscale(new_src, RectInt(0, 0, w, h), src, src_bounds, true);

						Matrix new_transformation = transformation
												* Matrix().set_translate(src_bounds.minx, src_bounds.miny)
												* Matrix().set_scale((Real)sw/(Real)w, (Real)sh/(Real)h);
						Helper::Generic<synfig::Surface::reader, synfig::Surface::reader>::resample(
							dest,
							dest_bounds,
							&new_src,
							RectInt(0, 0, w, h),
							new_transformation,
							interpolation,
							blend,
							blend_amount,
							blend_method );
						return;
					}
				}

				resample(
					dest,
					dest_bounds,
					src,
					src_bounds,
					transformation,
					interpolation,
					blend,
					blend_amount,
					blend_method );
			}
		};
	};
}


void
software::Resample::downscale(
	synfig::Surface &dest,
	const RectInt &dest_bounds,
	const synfig::Surface &src,
	const RectInt &src_bounds,
	bool keep_cooked )
{
	typedef synfig::Surface Surface;
	Helper::Generic<Surface::reader, Surface::reader_cook>::downscale(
		dest, dest_bounds,
		&src, src_bounds,
		keep_cooked );
}


void
software::Resample::downscale(
	synfig::Surface &dest,
	const RectInt &dest_bounds,
	const software::PackedSurface &src,
	const RectInt &src_bounds,
	bool keep_cooked )
{
	typedef software::PackedSurface::Reader Reader;
	Helper::Generic<Reader::reader, Reader::reader_cook>::downscale(
		dest, dest_bounds,
		&src, src_bounds,
		keep_cooked );
}


void
software::Resample::resample(
	synfig::Surface &dest,
	const RectInt &dest_bounds,
	const synfig::Surface &src,
	const RectInt &src_bounds,
	const Matrix &transformation,
	Color::Interpolation interpolation,
	bool blend,
	ColorReal blend_amount,
	Color::BlendMethod blend_method )
{
	typedef synfig::Surface Surface;
	Helper::Generic<Surface::reader, Surface::reader_cook>::resample_with_downscale(
		dest,
		dest_bounds,
		&src,
		src_bounds,
		transformation,
		interpolation,
		blend,
		blend_amount,
		blend_method );
}

void
software::Resample::resample(
	synfig::Surface &dest,
	const RectInt &dest_bounds,
	const software::PackedSurface &src,
	const RectInt &src_bounds,
	const Matrix &transformation,
	Color::Interpolation interpolation,
	bool blend,
	ColorReal blend_amount,
	Color::BlendMethod blend_method )
{
	typedef software::PackedSurface::Reader Reader;
	software::PackedSurface::Reader src_reader(src);
	Helper::Generic<Reader::reader, Reader::reader_cook>::resample_with_downscale(
		dest,
		dest_bounds,
		&src_reader,
		src_bounds,
		transformation,
		interpolation,
		blend,
		blend_amount,
		blend_method );
}


/* === E N T R Y P O I N T ================================================= */
