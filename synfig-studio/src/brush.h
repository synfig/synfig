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

		explicit SurfaceWrapper(surface_type *surface = NULL): surface(surface) { }

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

			float cs = cosf(angle/180.f*(float)PI);
			float sn = sinf(angle/180.f*(float)PI);

			// calculate bounds
			float maxr = fabsf(aspect_ratio) > 1.f ? fabsf(radius*aspect_ratio) : fabsf(radius);
			int x0 = (int)(x - maxr - 1.f);
			int x1 = (int)(x + maxr + 1.f);
			int y0 = (int)(y - maxr - 1.f);
			int y1 = (int)(y + maxr + 1.f);

			if (x0 < 0) x0 = 0;
			if (x1 > surface->get_w()-1) x1 = surface->get_w()-1;
			if (y0 < 0) y0 = 0;
			if (y1 > surface->get_h()-1) y1 = surface->get_h()-1;

			surface_type::alpha_pen apen(surface->get_pen(x0, y0));
			apen.set_blend_method(synfig::Color::BLEND_COMPOSITE);
			apen.set_value(synfig::Color(color_r, color_g, color_b));
			for(int py = y0; py <= y1; py++)
			{
				surface_type::alpha_pen ap(apen);
				for(int px = x0; px <= x1; px++)
				{
					float dx = (float)px - x;
					float dy = (float)py - y;
					float dyr = (dy*cs-dx*sn)*aspect_ratio;
					float dxr = (dy*sn+dx*cs);
					float dd = (dyr*dyr + dxr*dxr) / (radius*radius);
					if (dd > 1.f) continue;

					float opa = dd < hardness
							  ? dd + 1-(dd/hardness)
							  : hardness/(1-hardness)*(1-dd);
					ap.set_alpha(opa * opaque);
					ap.put_value();
					ap.inc_x();
				}
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
