/* === S Y N F I G ========================================================= */
/*!	\file centerlinevectorizer.cpp
**	\brief This is the entry point of centerline vectorization
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
#include "polygonizerclasses.h"
#include <synfig/layer.h>
#include <synfig/debug/log.h>
#endif

/* === U S I N G =========================================================== */

using namespace std;
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
