/* === S I N F G =========================================================== */
/*!	\file target_multi.cpp
**	\brief Template File
**
**	$Id: target_multi.cpp,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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

#include "target_multi.h"
#include "string.h"
#include "surface.h"
#include "canvas.h"
#include "context.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Target_Multi::Target_Multi(Target_Scanline::Handle a,Target_Scanline::Handle b):
	a(a),
	b(b)
{
}

Target_Multi::~Target_Multi()
{
}

void
Target_Multi::set_canvas(etl::handle<Canvas> c)
{
	canvas=c;
	RendDesc desc=canvas->rend_desc();
	a->set_canvas(c);
	b->set_canvas(c);
	set_rend_desc(&desc);
}

bool
Target_Multi::set_rend_desc(RendDesc *d)
{
	desc=*d;
	return a->set_rend_desc(d) && b->set_rend_desc(d);
}

bool
Target_Multi::init()
{
	return a->init() && b->init();
}

bool
Target_Multi::add_frame(const sinfg::Surface *surface)
{
	return a->add_frame(surface) && b->add_frame(surface);
}

bool
Target_Multi::start_frame(ProgressCallback *cb)
{
	return a->start_frame(cb) && b->start_frame(cb);
}

void
Target_Multi::end_frame()
{
	a->end_frame();
	b->end_frame();
}

Color *
Target_Multi::start_scanline(int scanline)
{
	buffer_a=a->start_scanline(scanline);
	buffer_b=b->start_scanline(scanline);
	return buffer_a;
}

bool
Target_Multi::end_scanline()
{
	memcpy(buffer_b,buffer_a,sizeof(Color)*desc.get_w());
	return a->end_scanline() && b->end_scanline();
}
