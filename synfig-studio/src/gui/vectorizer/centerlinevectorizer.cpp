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

/*
** Insert headers here
*/
#include "centerlinepolygonizer.cpp"
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

//************************
//*    Vectorizer Main   *
//************************

// takes three arguments ( TimageP, config and palette )
void VectorizerCore::centerlineVectorize(etl::handle<synfig::Layer_Bitmap> &image, const CenterlineConfiguration &configuration)
 {
  std::cout<<"Inside CenterlineVectorize\n";
  VectorizerCoreGlobals globals;
  globals.currConfig = &configuration;

  // step 2 
  // Extracts a polygonal, minimal yet faithful representation of image contours
  Contours polygons;
  polygonize(image, polygons, globals);

  // step 3
  // The process of skeletonization reduces all objects in an image to lines, 
  //  without changing the essential structure of the image.

  // Most time-consuming part of vectorization, 'this' is passed to inform of
  // partial progresses
//   SkeletonList *skeletons = skeletonize(polygons, this, globals);

//   if (isCanceled()) {
//     // Clean and return 0 at cancel command
//     deleteSkeletonList(skeletons);

//     return TVectorImageP();
//   }

//   // step 4
//   // The raw skeleton data obtained from StraightSkeletonizer
//   // class need to be grouped in joints and sequences before proceding further
//   organizeGraphs(skeletons, globals);

//   // junctionRecovery(polygons);   //Da' problemi per maxThickness<inf...
//   // sarebbe da rendere compatibile

//   std::vector<TStroke *> sortibleResult;
//   TVectorImageP result;
  
//   // step 5
//   // Take samples of image colors to associate each sequence to its corresponding
//   // palette color
//   calculateSequenceColors(ras, globals);  // Extract stroke colors here

//   // step 6
//   // Converts each forward or single Sequence of the image in its corresponding Stroke.
//   conversionToStrokes(sortibleResult, globals);

//   // step 7
//   // Take samples of image colors to associate each stroke to its corresponding
//   // palette color.
//   applyStrokeColors(sortibleResult, ras, palette,
//                     globals);  // Strokes get sorted here
  
//   // step 8
//   // copy/apply the strokes to an empty vector image
//   // and return the vector image after further adjustments
//   result = copyStrokes(sortibleResult);

//   // Further misc adjustments
//   if (globals.currConfig->m_thicknessRatio < 100)
//     reduceThickness(result, configuration.m_thicknessRatio);
//   if (globals.currConfig->m_maxThickness == 0.0)
//     for (unsigned int i = 0; i < result->getStrokeCount(); ++i)
//       result->getStroke(i)->setSelfLoop(true);
//   if (globals.currConfig->m_makeFrame) addFrameStrokes(result, ras, palette);
//   // randomizeExtremities(result);   //Cuccio random - non serve...

//   deleteSkeletonList(skeletons);

//   return result;
}
