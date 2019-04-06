/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/rendererpreviewsw.h
**	\brief RendererPreviewSW Header
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

#ifndef __SYNFIG_RENDERING_RENDERERPREVIEWSW_H
#define __SYNFIG_RENDERING_RENDERERPREVIEWSW_H

/* === H E A D E R S ======================================================= */

#include "../renderer.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class RendererPreviewSW: public Renderer
{
public:
	typedef etl::handle<RendererPreviewSW> Handle;

	RendererPreviewSW();
	virtual String get_name() const;
};

}; /* end namespace rendering */
}; /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
