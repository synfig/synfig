/* === S Y N F I G ========================================================= */
/*!	\file centerlinevectorizer.cpp
**	\brief This is the entry point of centerline vectorization
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "centerlinevectorizer.h"
#include "polygonizerclasses.h"
#include <synfig/layer.h>
#include <synfig/debug/log.h>
#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

inline void deleteSkeletonList(SkeletonList *skeleton) {
  unsigned int i;
  for (i = 0; i < skeleton->size(); ++i) delete (*skeleton)[i];

  delete skeleton;
}


/* === E N T R Y P O I N T ================================================= */

//************************
//*    Vectorizer Main   *
//************************

// takes two arguments ( image layer handle, config )

std::vector< etl::handle<synfig::Layer> > 
VectorizerCore::centerlineVectorize(etl::handle<synfig::Layer_Bitmap> &image,const etl::handle<synfigapp::UIInterface> &ui_interface, 
const CenterlineConfiguration &configuration,const Gamma &gamma)
 {
  synfig::debug::Log::info("","Inside CenterlineVectorize");
  VectorizerCoreGlobals globals;
  globals.currConfig = &configuration;

  // step 2 
  // Extracts a polygonal, minimal yet faithful representation of image contours
  Contours polygons;
  studio::polygonize(image, polygons, globals);
  ui_interface->amount_complete(3,10);
  
  // step 3
  // The process of skeletonization reduces all objects in an image to lines, 
  //  without changing the essential structure of the image.
  SkeletonList *skeletons = studio::skeletonize(polygons,ui_interface, globals);
  ui_interface->amount_complete(6,10);
  /* To be uncommented if isCanceled is needed in future
  if (isCanceled()) 
  {
    // Clean and return 0 at cancel command
    deleteSkeletonList(skeletons);
    std::cout<<"CenterlineVectorize cancelled\n";
  }
  */
  // step 4
  // The raw skeleton data obtained from StraightSkeletonizer
  // class need to be grouped in joints and sequences before proceeding further
  studio::organizeGraphs(skeletons, globals);
  ui_interface->amount_complete(8,10);


  std::vector< etl::handle<synfig::Layer> > sortibleResult;
  
  // step 5
  // Take samples of image colors to associate each sequence to its corresponding
  // palette color
  //studio::calculateSequenceColors(image, globals);  // Extract stroke colors here

  // step 6
  // Converts each forward or single Sequence of the image in its corresponding Stroke.
  studio::conversionToStrokes(sortibleResult, globals, image);
  ui_interface->amount_complete(9,10);

  deleteSkeletonList(skeletons);
  return sortibleResult;
}

std::vector< etl::handle<synfig::Layer> > 
VectorizerCore::vectorize(const etl::handle<synfig::Layer_Bitmap> &img,const etl::handle<synfigapp::UIInterface> &ui_interface, const VectorizerConfiguration &c, const Gamma &gamma) 
{
  std::vector< etl::handle<synfig::Layer> > result;

  if (c.m_outline)
  {
    return result;
  }
  else 
  {
    Handle img2(img);
    result = centerlineVectorize(img2, ui_interface,static_cast<const CenterlineConfiguration &>(c), gamma);
    ui_interface->amount_complete(10,10);

    return result;
    
  }

}
