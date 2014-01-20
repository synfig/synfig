/* === S Y N F I G ========================================================= */
/*!	\file brush.h
**	\brief Helper file to integrte brushlib into synfig
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_BRUSH_H
#define __SYNFIG_BRUSH_H

/* === H E A D E R S ======================================================= */

#include <synfig/surface.h>
#include <ETL/angle> // we need PI
#include "brushlib/brushlib.hpp"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace brush {
	class ActiveSurface: public Surface {
	public:
		virtual bool draw_dab(
			float /* x */, float /* y */,
			float /* radius */,
			float /* color_r */, float /* color_g */, float /* color_b */,
			float /* opaque */, float /* hardness */ = 0.5,
			float /* alpha_eraser */ = 1.0,
			float /* aspect_ratio */ = 1.0, float /* angle */ = 0.0,
			float /* lock_alpha */ = 0.0
		) { return false; };

		virtual void get_color(
			float /* x */, float /* y */,
			float /* radius */,
			float * color_r, float * color_g, float * color_b, float * color_a
		) { *color_r = 0.f; *color_g = 0.f; *color_b = 0.f; *color_a = 0.f; };
	};

	class SurfaceWrapper: public ActiveSurface {
	public:
		typedef synfig::Surface surface_type;
		surface_type *surface;
		int extra_left;
		int extra_right;
		int extra_top;
		int extra_bottom;
		int offset_x;
		int offset_y;

		explicit SurfaceWrapper(surface_type *surface = NULL):
			surface(surface),
			extra_left(0), extra_right(0),
			extra_top(0), extra_bottom(0),
			offset_x(0), offset_y(0) { }

		void reset() {
			extra_left = 0;
			extra_right = 0;
			extra_top = 0;
			extra_bottom = 0;
			offset_x = 0;
			offset_y = 0;
		}

		virtual bool draw_dab(
			float x, float y,
			float radius,
			float color_r, float color_g, float color_b,
			float opaque, float hardness = 0.5,
			float /* alpha_eraser */ = 1.0,
			float aspect_ratio = 1.0, float angle = 0.0,
			float /* lock_alpha */ = 0.0
		) {
			if (surface == NULL) return false;

			x += (float)offset_x;
			y += (float)offset_y;

			float cs = cosf(angle/180.f*(float)PI);
			float sn = sinf(angle/180.f*(float)PI);

			// calculate bounds
			if (aspect_ratio < 1.0) aspect_ratio = 1.0;
			if (hardness > 1.0) hardness = 1.0;
			if (hardness < 0.0) hardness = 0.0;
			float maxr = fabsf(radius);
			int x0 = (int)(x - maxr - 1.f);
			int x1 = (int)(x + maxr + 1.f);
			int y0 = (int)(y - maxr - 1.f);
			int y1 = (int)(y + maxr + 1.f);

			if (x0 < 0
			 || y0 < 0
			 || x1+1 > surface->get_w()
			 || y1+1 > surface->get_h() )
			{
				int l = x0 < 0 ? x0 : 0;
				int t = y0 < 0 ? y0 : 0;
				int r = x1+1 > surface->get_w() ? x1+1 : surface->get_w();
				int b = y1+1 > surface->get_h() ? y1+1 : surface->get_h();

				extra_left   -= l; // increase because l and t is negative
				extra_top    -= t;
				extra_right  += r - surface->get_w();
				extra_bottom += b - surface->get_h();

				synfig::Surface tmp(surface->get_w(), surface->get_h());
				tmp.copy(*surface);
				surface->set_wh(r-l, b-t);
				surface->clear();
				synfig::Surface::pen p(surface->get_pen(-l, -t));
				tmp.blit_to(p);

				offset_x -= l;
				offset_y -= t;
				x -= (float)l; y -= (float)t;
				x0 -= l; y0 -= t;
				x1 -= l; y1 -= t;
			}

			surface_type::alpha_pen apen(surface->get_pen(x0, y0));
			apen.set_blend_method(synfig::Color::BLEND_COMPOSITE);
			apen.set_value(synfig::Color(color_r, color_g, color_b));
			for(int py = y0; py <= y1; py++)
			{
				for(int px = x0; px <= x1; px++)
				{
					float dx = (float)px - x;
					float dy = (float)py - y;
					float dyr = (dy*cs-dx*sn)*aspect_ratio;
					float dxr = (dy*sn+dx*cs);
					float dd = (dyr*dyr + dxr*dxr) / (radius*radius);
					if (dd <= 1.f)
					{
						float opa = dd < hardness
								  ? dd + 1-(dd/hardness)
								  : hardness/(1-hardness)*(1-dd);
						apen.set_alpha(opa * opaque);
						apen.put_value();
					}
					apen.inc_x();
				}
				apen.dec_x(x1-x0+1);
				apen.inc_y();
			}

			return true;
		};

		virtual void get_color(
			float x, float y,
			float /* radius */,
			float * color_r, float * color_g, float * color_b, float * color_a
		) {
			if (surface == NULL) {
				*color_r = 0.f; *color_g = 0.f; *color_b = 0.f; *color_a = 0.f;
				return;
			}

			x += (float)offset_x;
			y += (float)offset_y;

			synfig::Color c = surface->cubic_sample(x, y);
			*color_r = c.get_r();
			*color_g = c.get_g();
			*color_b = c.get_b();
			*color_a = c.get_a();
		};
	};

}; // END of namespace brush

/* === E N D =============================================================== */

#endif
