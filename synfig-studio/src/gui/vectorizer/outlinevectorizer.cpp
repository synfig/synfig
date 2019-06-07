/* === S Y N F I G ========================================================= */
/*!	\file template.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "centerlinevectorizer.h"
#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

//=================================================================

void studio::VectorizerCore::vectorize(const etl::handle<synfig::Layer_Bitmap> &img, const VectorizerConfiguration &c) 
{

  if (c.m_outline)
  {
    std::cout<<"newOutlineVectorize called/n";
    // vi = newOutlineVectorize(img, static_cast<const NewOutlineConfiguration &>(c), plt);
  }
  else 
  {
    Handle img2(img);
    centerlineVectorize(img2, static_cast<const CenterlineConfiguration &>(c));

    // if (vi) {
    //   for (int i = 0; i < (int)vi->getStrokeCount(); ++i) {
    //     TStroke *stroke = vi->getStroke(i);

    //     for (int j = 0; j < stroke->getControlPointCount(); ++j) {
    //       TThickPoint p = stroke->getControlPoint(j);
    //       p             = TThickPoint(c.m_affine * p, c.m_thickScale * p.thick);

    //       stroke->setControlPoint(j, p);
    //     }
    //   }

    //   applyFillColors(vi, img2, plt, c);
    // }
  }

}