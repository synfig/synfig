/* === S Y N F I G ========================================================= */
/*!	\file cairo_renddesc.cpp
 **	\brief Implementation of Cairo helper functions with RendDesc
 **
 **	$Id$
 **
 **	\legal
 **	Copyright (c) 2013 Carlos López
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

#include "cairo_renddesc.h"
#include "general.h"
#include <synfig/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

bool
cairo_renddesc_untransform(cairo_t* cr, RendDesc &renddesc)
{
	const Real	pw = renddesc.get_pw(),	ph = renddesc.get_ph();
	
	const Point	tl(renddesc.get_tl());
	const Point br(renddesc.get_br());
	
	double tl_x, tl_y, tr_x, tr_y, bl_x, bl_y, br_x, br_y;
	double mtlx, mtly, mbrx, mbry;
	double pminx, pminy, pmaxx, pmaxy;
	
	const int flags=renddesc.get_flags();
	
	tl_x=tl[0];
	tl_y=tl[1];
	br_x=br[0];
	br_y=br[1];
	tr_x=br_x;
	tr_y=tl_y;
	bl_x=tl_x;
	bl_y=br_y;

	RendDesc	workdesc(renddesc);
	// In this block we are going to calculate the inversed transform of the
	// workdesc but not applying the transformation to convert the surface to
	// device space (See the cairo translate and scale on Target_Cairo::render and
	// Target_Cairo_Tile::render)
	
	// Extract the matrix from the current context
	cairo_matrix_t cr_matrix, cr_result;
	cairo_get_matrix(cr, &cr_matrix);
	
	// Now create three matrixes with the following values:
	// resulting matrix result=i_translate*i_scale
	// inverse translation i_translate = inverse translation from -renddesc_tl
	// inverse scale i_scale = inverse scale of 1/pw and 1/ph
	
	cairo_matrix_t i_scale, i_translate, result;
	cairo_matrix_init_translate(&i_translate, tl[0], tl[1]);
	cairo_matrix_init_scale(&i_scale, pw, ph);
	
	// Now multiply the two matrixes, the order is important!
	// first apply scale and then rotate, the inverse than done in Target_Cairo::render
	
	cairo_matrix_multiply(&result, &i_scale, &i_translate);
	
	// Now let's multiply the cr matrix retrieved and the result matrix
	
	cairo_matrix_multiply(&cr_result, &cr_matrix, &result);
	
	// Explanation:
	// Current cairo context matrix is this of this form:
	// [T][S][DRAW] where the [T][S] parts corresponds to convert the cairo operations
	// in DRAW part into the device space (usually the image surface of size w, h)
	// DRAW matrix is the result of the layer transformations stack (rotate, zoom, etc.)
	// But we want to transformm the render desc with the inverse of the DRAW part only,
	// not the inverse of the T and S part because we are transforming user coordinates
	// the renddesc and not pixels.
	// So we retrieve the cairo context matrix: [CR]=[T][S][DRAW] and remove the [T] and
	// [S] matrixes by applying its inverses: (the notation ' denotes inverse)
	// [S'][T'][CR]=[S'][T'][T][S][DRAW]=[S'][I][S][DRAW]=[I][DRAW]=[DRAW] as we wanted.
	// [M'][M]=[I] where I is the identity matrix.
	
	
	// Now let's invert the result matrix, that is calculate [DRAW']
	cairo_status_t status;
	status=cairo_matrix_invert(&cr_result);
	if(status) // doh! the matrix can't be inverted! I can't render the surface!
	{
		synfig::error("Can't invert current Cairo matrix!");
		return false;
	}
	
	// Now let's transform the renddesc corners with the calculated matrix
	cairo_matrix_transform_point(&cr_result, &tl_x, &tl_y);
	cairo_matrix_transform_point(&cr_result, &tr_x, &tr_y);
	cairo_matrix_transform_point(&cr_result, &bl_x, &bl_y);
	cairo_matrix_transform_point(&cr_result, &br_x, &br_y);
	
	// Now let's figure out the rounding box of the transformed renddesc
	pminx=min(min(min(tl_x, tr_x), bl_x), br_x);
	pminy=min(min(min(tl_y, tr_y), bl_y), br_y);
	pmaxx=max(max(max(tl_x, tr_x), bl_x), br_x);
	pmaxy=max(max(max(tl_y, tr_y), bl_y), br_y);
	// let's assign the right values to the meaningful variables :)
	mtlx=pminx;
	mtly=pmaxy;
	mbrx=pmaxx;
	mbry=pminy;
	
	// Now apply the new tl and br values to the workdesc
	// We don't want to render more pixels than the needed by the original
	// renddesc, so we are going to keep the number of pixels in the diagonal
	// to coincide with the original diagonal
	Vector m_diagonal(br_x-tl_x, br_y-tl_y);
	Vector m_diagonal_bbox(mbrx-mtlx, mbry-mtly);
	double diagonal=m_diagonal.mag();
	double diagonal_bbox=m_diagonal_bbox.mag();
	int w_new=diagonal_bbox/diagonal * renddesc.get_w();
	int h_new=diagonal_bbox/diagonal * renddesc.get_h();
	workdesc.clear_flags();
	// finally apply the new desc values!
	workdesc.set_tl_br(Point(mtlx, mtly), Point(mbrx, mbry));
	workdesc.set_w(w_new);
	workdesc.set_h(h_new);

	renddesc=workdesc;
	
	renddesc.set_flags(flags);
	
	return true;
}

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */
