/* === S Y N F I G ========================================================= */
/*!	\file centerlinevectorizer.h
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_STUDIO_CENTERLINEVECTORIZER_H
#define __SYNFIG_STUDIO_CENTERLINEVECTORIZER_H

/* === H E A D E R S ======================================================= */
#include "vectorizerparameters.h"
#include <ETL/handle>
#include <synfig/layers/layer_bitmap.h>
#include <synfig/vector.h>
/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */
typedef etl::handle<synfig::Layer_Bitmap> Handle;


/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

//==============================
//    Core vectorizer class
//==============================

//! Contains specific vectorization methods and deals with partial progress
//! notifications (using Qt signals).
/*!VectorizerCore class is the lowest layer of a vectorization process, it
provides vectorization of a
single input raster image by calling the \b vectorize method.

It can also deal notifications about its progress status, and is receptive to
user cancels.

\sa VectorizerPopup, Vectorizer, VectorizerConfiguration classes.*/
class VectorizerCore
{
  int m_currPartial;
  int m_totalPartials;

  bool m_isCanceled;

public:
  VectorizerCore() : m_currPartial(0), m_isCanceled(false) {}
  ~VectorizerCore() {}

  //! Returns true if vectorization was aborted at user's request
  bool isCanceled() { return m_isCanceled; }

  /*!Calls the appropriate technique to convert \b image to vectors depending on c.*/
 
  std::vector< etl::handle<synfig::Layer> > vectorize(const etl::handle<synfig::Layer_Bitmap> &image, const VectorizerConfiguration &c);

private:
  std::vector< etl::handle<synfig::Layer> > centerlineVectorize(Handle &image, const CenterlineConfiguration &configuration);

};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif