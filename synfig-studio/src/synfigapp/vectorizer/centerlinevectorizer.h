/* === S Y N F I G ========================================================= */
/*!	\file centerlinevectorizer.h
**
**	$Id$
**
**	\legal
**	This file uses code from OpenToonz open-source animation software  (https://github.com/opentoonz/opentoonz/), which is developed from Toonz, a software originally created by Digital Video, S.p.A., Rome Italy Digital Video, S.p.A., Rome Italy.
**
**	Copyright (c) 2016 - 2019, DWANGO Co., Ltd.
**	Copyright (c) 2016 Toshihiro Shimizu - https://github.com/meso
**	Copyright (c) 2016 Shinya Kitaoka - https://github.com/skitaoka
**	Copyright (c) 2016 shun-iwasawa - https://github.com/shun-iwasawa
**	Copyright (c) 2016 Campbell Barton - https://github.com/ideasman42
**	Copyright (c) 2019 luzpaz - https://github.com/luzpaz
**
**	Copyright (c) 2019 - 2020, Ankit Kumar Dwivedi - https://github.com/ankit-kumar-dwivedi
**
**	LICENSE
** 
**	BSD 3-Clause "New" or "Revised" License
** 
**	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
**
**	1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
**
**	2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
**
**	3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
**
**	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
#include <synfigapp/uimanager.h>
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
  VectorizerCore() : m_currPartial(0), m_totalPartials(0), m_isCanceled(false) {}
  ~VectorizerCore() {}

  //! Returns true if vectorization was aborted at user's request
  bool isCanceled() { return m_isCanceled; }

  /*!Calls the appropriate technique to convert \b image to vectors depending on c.*/
 
  std::vector< etl::handle<synfig::Layer> > vectorize(const etl::handle<synfig::Layer_Bitmap> &image, const etl::handle<synfigapp::UIInterface> &ui_interface,const VectorizerConfiguration &c,const synfig::Gamma &gamma);

private:
  std::vector< etl::handle<synfig::Layer> > centerlineVectorize(Handle &image,const etl::handle<synfigapp::UIInterface> &ui_interface, const CenterlineConfiguration &configuration, const synfig::Gamma &gamma);

};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
